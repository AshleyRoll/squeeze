#ifndef SQUEEZE_HUFFMANENCODER_H
#define SQUEEZE_HUFFMANENCODER_H

#include <limits>
#include <algorithm>
#include <array>
#include <utility>
#include <variant>
#include <span>

#include "concepts.h"
#include "lib/priority_queue.h"
#include "lib/list.h"
#include "lib/bit_stream.h"

namespace squeeze {

    namespace huffman {

        // Used to count character frequency in source strings
        struct CharFrequency {
            char c;
            std::size_t frequency;
        };

        // Used to construct the huffman tree in memory before outputting as an array of Nodes
        struct TreeNode
        {
            std::size_t prob;
            char value;
            std::uint16_t index;
            TreeNode *parent;
            std::array<TreeNode*, 2> child;


            constexpr TreeNode(std::size_t p, char v, TreeNode *up)
                    : prob{p}, value{v}, index{0}, parent{up}, child{nullptr, nullptr}
            {}

            constexpr TreeNode(std::size_t p, TreeNode* n0, TreeNode *n1, TreeNode *up)
                    : prob{p}, value{0}, index{0}, parent{up}, child{n0, n1}
            {}

            constexpr TreeNode()
                    : prob{0}, value{0}, index{0}, parent{nullptr}, child{nullptr, nullptr}
            {}

            [[nodiscard]] constexpr bool IsLeaf() const { return child.at(0) == nullptr || child.at(1) == nullptr; }
        };

        // Used to store the huffman tree in a flat array.
        struct Node
        {
        public:
            // We use an array of 2 words for indexing into the array of Nodes
            // to build the tree without pointer. 16 bits is more than enough to
            // handle the number of nodes that could be generated for a character set based on 8 bits.
            using IndexType = std::uint16_t;
            using CharType = char;
            using Links = std::array<IndexType, 2>; // [zeroLink, oneLink]

            constexpr Node() = default; // needed to construct array before initialisation

            // Make a leaf node
            constexpr explicit Node(char c)
                : m_Data{c}
            {}

            // Make an intermediate node
            constexpr Node(IndexType zero, IndexType one)
                : m_Data{std::to_array({zero, one})}
            {}

            [[nodiscard]] constexpr bool IsLeaf() const { return std::holds_alternative<char>(m_Data); }

            [[nodiscard]] constexpr CharType Value() const { return std::get<char>(m_Data); }

            [[nodiscard]] constexpr IndexType operator[](std::size_t idx) const
            {
                return std::get<Links>(m_Data).at(idx);
            }

        private:
            std::variant<char, Links> m_Data;
        };

        // we need to know the parent of a node to perform encoding efficiently
        struct EncodingNode : public Node
        {
        public:
            constexpr EncodingNode() = default; // needed to construct array before initialisation

            // Make a leaf node
            constexpr explicit EncodingNode(char c, IndexType parentIndex)
                : Node{c}, m_ParentIndex{parentIndex}
            {}

            // Make an intermediate node
            constexpr EncodingNode(IndexType zero, IndexType one, IndexType parentIndex)
                : Node{zero, one}, m_ParentIndex{parentIndex}
            {}

            [[nodiscard]] constexpr IndexType Parent() const
            {
                return m_ParentIndex;
            }

        private:
            IndexType m_ParentIndex;
        };

        // Encodes the start bit and original length of a compressed string
        struct Entry
        {
            std::size_t FirstBit;
            std::size_t OriginalStringLength;
        };


        // Represents a string that is being accessed. We have to do this via iteration,
        // so we don't have to build the entire string in memory before using it - this would
        // make compressing it pointless in a memory constrained environment.
        //
        // Note: we don't template this object and handle data as spans so that we have to have
        // multiple copies of this code if more than one string table is defined in an application
        struct IterableString
        {
        private:
            class ValueHolder
            {
            public:
                explicit ValueHolder(char value) : Value(value) {}

                char operator*() { return Value; }

            private:
                char Value;
            };

        public:
            using BitAccessorFunc = std::function<bool(std::size_t)>;   // index 0 = first bit in encoded string

            class Iterator
            {
            public:
                using value_type = char const;
                using reference = char;
                using iterator_category = std::input_iterator_tag;
                using pointer = char const *;
                using difference_type = void;

                struct EndPosition{IterableString const &str;};

                explicit Iterator(IterableString const &owner)
                    : m_Owner{owner}
                    , m_CharPosition{0}
                {
                    // handle empty string
                    if(m_Owner.m_StringLength == 0) {
                        // turn into end iterator
                        m_CharPosition = m_Owner.m_StringLength;
                    } else {
                        // load the first character
                        next();
                        m_CharPosition = 0;
                    }
                }

                explicit Iterator(EndPosition pos)
                : m_Owner{pos.str}
                , m_CharPosition{m_Owner.m_StringLength}
                {}

                reference operator*() const {
                    return m_Current;
                }

                pointer operator->() const {
                    return &m_Current;
                }

                Iterator &operator++() {
                    if (is_done())
                        throw std::runtime_error("Increment past-the-end iterator");

                    next();

                    return *this;
                }

                ValueHolder operator++(int) {
                    ValueHolder temp(**this);
                    ++*this;
                    return temp;
                }

                friend bool operator==(Iterator const &lhs, Iterator const &rhs) {
                    return lhs.m_CharPosition == rhs.m_CharPosition;
                    //(lhs.m_Owner == rhs.m_Owner && lhs.m_CharPosition == rhs.m_CharPosition);
                }

            private:
                [[nodiscard]] bool is_done() const
                {
                    return m_CharPosition >= m_Owner.m_StringLength;
                }

                void next()
                {
                    // we can only fetch up to the last character, but need to increment past end
                    // for end iterator comparison
                    if(m_CharPosition + 1 < m_Owner.m_StringLength) {
                        std::size_t i{0};    // start at root node

                        // walk the node tree using the bit stream until we get to a leaf node.
                        // Then return the character encoded by that node
                        while(!m_Owner.m_Nodes[i].IsLeaf()) {
                            auto bit = m_Owner.m_GetBit(m_NextBit++);
                            i = m_Owner.m_Nodes[i][static_cast<std::size_t>(bit)];
                        }

                        m_Current = m_Owner.m_Nodes[i].Value();
                    }

                    ++m_CharPosition;
                }

                IterableString const &m_Owner;

                // iteration state
                char m_Current{0};
                std::size_t m_NextBit{0};
                std::size_t m_CharPosition{0};

            };




            IterableString(
                    std::size_t stringLength,
                    BitAccessorFunc getBit,
                    std::span<Node const> nodes
            )
                : m_StringLength{stringLength}
                , m_GetBit{std::move(getBit)}
                , m_Nodes{nodes}
            {}

            [[nodiscard]] std::size_t size() const { return m_StringLength; }

            Iterator begin() { return Iterator{*this}; }
            Iterator end() { return Iterator{Iterator::EndPosition{*this}}; }

        private:
            std::size_t const m_StringLength;
            BitAccessorFunc const m_GetBit;
            std::span<Node const> const m_Nodes;
        };



        // Contains the entries and the bitstream they are based on
        // to store all the compressed strings
        template<std::size_t NUM_ENTRIES, std::size_t NUM_ENCODED_BITS, std::size_t NUM_TREE_NODES>
        struct Encoding
        {
            static constexpr std::size_t NumEntries = NUM_ENTRIES;
            static constexpr std::size_t NumEncodedBits = NUM_ENCODED_BITS;
            static constexpr std::size_t NumTreeNodes = NUM_TREE_NODES;

            IterableString operator[](std::size_t idx) const
            {
                auto const thisEntry = Entries.at(idx);

                return IterableString{
                    thisEntry.OriginalStringLength,
                    [&, thisEntry](std::size_t i) { return CompressedStream.at(i + thisEntry.FirstBit); },
                    std::span{HuffmanTable}
                };
            }

            std::array<Entry, NUM_ENTRIES> Entries;
            lib::bit_stream<NUM_ENCODED_BITS> CompressedStream;
            std::array<Node, NUM_TREE_NODES> HuffmanTable;
        };





        //
        // Build an array of Nodes which link together using indexes to represent the Huffman tree.
        //
        // This flattened tree is then used for generating the encoded strings at compile time, and
        // decoding the strings, character by character at run time.
        //
        static constexpr auto BuildHuffmanTree(CallableGivesIterableStringViews auto makeStringsLambda)
        {
            //
            // Count the frequency of all the characters in tall the strings to be compressed
            //
            constexpr auto CountFrequency = [=]()
            {
                // get the string table to work with
                constexpr auto st = makeStringsLambda();

                // one element per possible character value
                std::array<std::size_t, std::numeric_limits<std::string_view::value_type>::max()> counts{};
                counts.fill(0);

                for(auto &s : st) {
                    for (auto c : s) {
                        counts.at(static_cast<std::size_t>(c)) += 1;
                    }
                }

                return counts;
            };


            //
            // Build an array of CharFrequency structs for each used character from the original strings
            // capturing its frequency as character value
            //
            constexpr auto BuildFrequencyTable = [=]()
            {
                constexpr auto counts = CountFrequency();
                constexpr auto NumEntries = std::count_if(counts.begin(), counts.end(), [](auto i){return i != 0;});

                std::array<CharFrequency, NumEntries> ft;
                std::size_t e{0};
                for(std::size_t i{0}; i < counts.size(); ++i) {
                    if(counts.at(i) == 0) {
                        continue;
                    }

                    ft.at(e++) = CharFrequency{static_cast<char>(i), counts.at(i)};
                }

                return ft;
            };

            // compute the frequency table and calculate the number of tree nodes needed
            constexpr auto ft = BuildFrequencyTable();
            constexpr auto NumFtEntries = std::distance(ft.begin(), ft.end());

            //
            // In order to allocate the correct number of nodes to store the Huffman tree, we have to be
            // able to count the needed nodes and return a constant compile time value. This means we have to
            // do the work of making a tree twice.
            //
            // We optimise the counting but only doing the minimal work, and not making the tree itself.
            // This works because the Huffman tree is built from the bottom up.
            //
            constexpr auto CalculateTreeNodeCount = [=]()
            {
                // make a queue of probabilities and iterate through as if building the tree
                // tracking the total number of nodes needed. Min-heap priority queue.
                lib::priority_queue<std::size_t, NumFtEntries, std::greater<std::size_t>> queue;

                // initialise the min-priority queue
                for(auto const &f: ft) {
                    queue.push(f.frequency);
                }

                // track the number of nodes used. Starting with the leaf nodes we added to the queue.
                // as we "build" the tree bottom up
                std::size_t numNodes{NumFtEntries};

                while(queue.size() > 1) {
                    auto n1 = queue.top();
                    queue.pop();
                    auto n2 = queue.top();
                    queue.pop();

                    numNodes++; // we would allocate a new node here

                    queue.push(n1+n2);
                }

                return numNodes;
            };

            constexpr auto NumNodes = CalculateTreeNodeCount();

            // compare "greater" to build a min-heap priority queue
            auto cmpTreeNode = [](TreeNode const* left, TreeNode const* right){ return left->prob > right->prob; };

            // Build the initial TreeNode min-heap. We allocate storage for all the tree nodes
            // in the array, then fill them with the initial leaf nodes, and push them into the min-heap
            std::array<TreeNode, NumNodes> nodes;
            lib::priority_queue<TreeNode*, NumFtEntries, decltype(cmpTreeNode)> queue;

            for(std::size_t i{0}; i < NumFtEntries; ++i) {
                auto const& f = ft.at(i);
                nodes.at(i) = TreeNode{f.frequency, f.c, nullptr};
                queue.push(&nodes.at(i));
            }

            // Build the tree by removing 2 TreeNodes from the priority queue and making
            // a new TreeNode with them as children, and add this back to the queue.
            //
            // The top of the min-heap priority queue will be the 2 least used items which
            // could be leaf nodes or previously combined intermediate nodes.
            //
            // When there is only 1 item left, this is the root of the huffman tree
            std::size_t nextNode{NumFtEntries};   // next available TreeNode slot to allocate

            while(queue.size() > 1) {
                auto n1 = queue.top();
                queue.pop();
                auto n2 = queue.top();
                queue.pop();

                // create a new tree node & add to queue
                nodes.at(nextNode) = TreeNode{ n1->prob + n2->prob, n1, n2, nullptr};

                // set the parent links
                auto *newNode = &nodes.at(nextNode);
                n1->parent = newNode;
                n2->parent = newNode;

                queue.push(newNode);

                // increment next available node
                nextNode++;
            }

            // queue has 1 element which is the top of the tree
            auto *rootNode = queue.top();

            // Do a breadth first traversal of the node tree to allocate indexes in final output
            // array to the nodes. This places the root node at index 0, so we know where to start
            // any traversals.
            lib::list<TreeNode*> remaining;
            std::uint16_t i = 0;
            remaining.push_back(rootNode);
            while(!remaining.empty()) {
                auto *n = remaining.front();
                remaining.pop_front();

                n->index = i++;

                if(n->child.at(0) != nullptr) {
                    remaining.push_back(n->child.at(0));
                }

                if(n->child.at(1) != nullptr) {
                    remaining.push_back(n->child.at(1));
                }
            }

            // Now that we have numbered the nodes, we iterator over the node data
            // and generate the output ordered set of nodes, using the index from the child nodes
            // for the link indexes.
            //
            // Note that intermediate nodes will always have 2 children, and leaf nodes will have none
            std::array<EncodingNode, NumNodes> result;

            for(auto const& n : nodes) {
                auto idx = n.index;
                EncodingNode::IndexType parentIdx = n.parent != nullptr ? n.parent->index : 0;

                if(n.IsLeaf()) {
                    result.at(idx) = EncodingNode{ n.value, parentIdx };
                } else {
                    result.at(idx) = EncodingNode{ n.child.at(0)->index, n.child.at(1)->index, parentIdx };
                }
            }

            return result;
        }


        static constexpr auto MakeEncodedBitStream(CallableGivesIterableStringViews auto makeStringsLambda)
        {
            // get the string table to work with
            constexpr auto st = makeStringsLambda();
            constexpr auto NumStrings = std::distance(st.begin(), st.end());

            // build the huffman tree
            constexpr auto tree = BuildHuffmanTree(makeStringsLambda);

            constexpr auto MakeCharacterLookupTable = [=]() {
                struct CharData
                {
                    std::size_t TreeIndex{0};
                    std::size_t BitLength{0};
                };

                // Build a fast lookup "table" for all the characters
                std::array<CharData, 256> charLookup;

                for(std::size_t nodeIdx{0}; nodeIdx < tree.size(); ++nodeIdx) {
                    auto const &node = tree.at(nodeIdx);
                    if(node.IsLeaf())  {
                        // this is a leaf, find the bit length for this character
                        std::size_t len{0};
                        std::size_t idx{nodeIdx};

                        // walk up the tree until we get to the root node at index 0
                        while(idx != 0) {
                            auto const &n = tree.at(idx);
                            ++len;
                            idx = n.Parent();
                        }

                        // store the character data for this node's character
                        charLookup.at(static_cast<std::size_t>(node.Value())) = CharData{nodeIdx, len};
                    }
                }

                return charLookup;
            };

            constexpr auto charLookup = MakeCharacterLookupTable();

            constexpr auto CalculateStringLength = [=](std::string_view s) -> std::size_t
            {
                std::size_t len{0};

                for(char const c : s) {
                    len += charLookup.at(static_cast<std::size_t>(c)).BitLength;
                }

                return len;
            };

            constexpr auto CalculateEncodedStringBitLengths = [=]()
            {
                // we will return an array of lengths in bits
                std::array<std::size_t, NumStrings> result;

                std::size_t i{0};
                for(auto const &s : st) {
                    result.at(i) = CalculateStringLength(s);
                    ++i;
                }

                return result;
            };

            constexpr auto EncodeString = [=]<std::size_t NUM_BITS>(std::string_view str, std::size_t const firstBit, lib::bit_stream<NUM_BITS> &stream)
            {
                std::size_t i{0};

                for(char const c : str) {
                    // Get the character data
                    auto cd = charLookup.at(static_cast<std::size_t>(c));

                    auto cLen = cd.BitLength;

                    // starting at the character leaf node, walk up the tree
                    // capturing bits as we go. Note that we are traversing bottom up
                    // so we have to write the bits into the stream reversed.
                    std::size_t nidx{cd.TreeIndex};
                    while(nidx != 0) {
                        auto const &n = tree.at(nidx);
                        auto const &p = tree.at(n.Parent());

                        // determine if this is the one or zero node of the parent
                        // if the "one" link is our node, bit will be true (1)
                        // stream is initialised to zero, so we don't need to do clears
                        auto const bitNum = firstBit + i + cLen - 1;
                        if(p[1] == nidx) {
                            stream.set(bitNum);
                        } else {
                            stream.clear(bitNum);
                        }

                        // move to next bit
                        --cLen;
                        nidx = n.Parent();
                    }

                    // move to the next start position
                    i += cd.BitLength;
                }

                return i;
            };

            // ------------------------------------------------------------


            // get the encoded string lengths
            constexpr auto stringLengths = CalculateEncodedStringBitLengths();

            constexpr auto totalEncodedLength = std::accumulate(stringLengths.begin(), stringLengths.end(), 0);

            // create a suitable bit stream to hold the data
            Encoding<NumStrings, totalEncodedLength, tree.size()> result;

            // Build the entries into the result and write the compressed bit stream
            std::size_t entry{0};
            std::size_t bit{0};
            for(auto &sv : st) {
                // save the original length and the start bit for this string
                result.Entries.at(entry) = Entry{ bit, sv.size() };

                auto numBits = EncodeString(sv, bit, result.CompressedStream);
                ++entry;
                bit += numBits;
            }

            // copy the huffman tree into the result
            std::copy(tree.begin(), tree.end(), result.HuffmanTable.begin());

            return result;
        }


    }

    class HuffmanTableEncoder
    {
    public:

        static constexpr auto Compile(CallableGivesIterableStringViews auto makeStringsLambda)
        {
            constexpr auto const encoding = huffman::MakeEncodedBitStream(makeStringsLambda);

            return encoding;
        }

    private:



    };


}


#endif //SQUEEZE_HUFFMANENCODER_H
