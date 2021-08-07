#ifndef SQUEEZE_HUFFMANENCODER_H
#define SQUEEZE_HUFFMANENCODER_H

#include <limits>
#include <algorithm>
#include <array>
#include <variant>

#include "concepts.h"
#include "lib/priority_queue.h"
#include "lib/list.h"

namespace squeeze {

    /*
    namespace {
        struct RawString {
        public:

            class Iterator {
                class ValueHolder {
                public:
                    ValueHolder(char value) : Value(value) {}

                    char operator*() { return Value; }

                private:
                    char Value;
                };

            public:
                using value_type = char const;
                using reference = char;
                using iterator_category = std::input_iterator_tag;
                using pointer = char const *;
                using difference_type = void;

                Iterator(RawString *owner) : Owner{owner} {}

                reference operator*() const {
                    return *Owner->Current;
                }

                pointer operator->() const {
                    return Owner->Current;
                }

                Iterator &operator++() {
                    if (!Owner)
                        throw std::runtime_error("Increment a past-the-end iterator");

                    ++Owner->Current;

                    // if we have reached the end, turn into the end iterator by setting the owner null
                    if (Owner->Current == Owner->Final)
                        Owner = nullptr;

                    return *this;
                }

                ValueHolder operator++(int) {
                    ValueHolder temp(**this);
                    ++*this;
                    return temp;
                }


                friend bool operator==(Iterator const &lhs, Iterator const &rhs) {
                    return lhs.Owner == rhs.Owner;
                }


            private:
                RawString *Owner;
            };


            constexpr RawString(char const *start, char const *final)
                    : Current{start}, Final{final} {}

            Iterator begin() { return Iterator{this}; }

            Iterator end() { return Iterator(nullptr); }

        private:
            char const *Current;
            char const *Final;
        };
    }
    */

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
            std::array<TreeNode*, 2> link;

            constexpr TreeNode(std::size_t p, char v)
                    : prob{p}, value{v}, index{0}, link{nullptr, nullptr}
            {}

            constexpr TreeNode(std::size_t p, TreeNode* n0, TreeNode *n1)
                    : prob{p}, value{0}, index{0}, link{n0, n1}
            {}

            constexpr TreeNode()
                    : prob{0}, value{0}, index{0}, link{nullptr, nullptr}
            {}

            [[nodiscard]] constexpr bool IsLeaf() const { return link.at(0) == nullptr || link.at(1) == nullptr; }
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
            using Links = std::array<IndexType, 2>;

            constexpr Node() = default; // needed to construct array before initialisation

            // Make a leaf node
            constexpr explicit Node(char c)
                : data{c}
            {}

            // Make an intermediate node
            constexpr Node(std::uint16_t zero, std::uint16_t one)
                : data{std::to_array({zero, one})}
            {}

            [[nodiscard]] constexpr bool IsLeaf() const { return std::holds_alternative<char>(data); }

            [[nodiscard]] constexpr CharType value() const { return std::get<char>(data); }

            [[nodiscard]] constexpr IndexType operator[](std::size_t idx) const
            {
                return std::get<Links>(data).at(idx);
            }

        private:
            std::variant<char, Links> data;
        };

        //
        // Count the frequency of all the characters in tall the strings to be compressed
        //
        static constexpr auto CountFrequency(CallableGivesIterableStringViews auto makeStringsLambda)
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
        }

        //
        // Build an array of CharFrequency structs for each used character from the original strings
        // capturing its frequency as character value
        //
        static constexpr auto BuildFrequencyTable(CallableGivesIterableStringViews auto makeStringsLambda)
        {
            constexpr auto counts = CountFrequency(makeStringsLambda);
            constexpr auto NumEntries = std::count_if(counts.begin(), counts.end(), [](auto i){return i != 0;});

            std::array<CharFrequency, NumEntries> ft;
            std::size_t e = 0;
            for(std::size_t i = 0; i < counts.size(); ++i)
            {
                if(counts.at(i) == 0) {
                    continue;
                }

                ft.at(e++) = CharFrequency{static_cast<char>(i), counts.at(i)};
            }

            return ft;
        }

        //
        // In order to allocate the correct number of nodes to store the Huffman tree, we have to be
        // able to count the needed nodes and return a constant compile time value. This means we have to
        // do the work of making a tree twice.
        //
        // We optimise the counting but only doing the minimal work, and not making the tree itself.
        // This works because the Huffman tree is built from the bottom up.
        //
        static constexpr auto CalculateTreeNodeCount(CallableGivesIterableStringViews auto makeStringsLambda)
        {
            // compute the frequency table
            constexpr auto ft = BuildFrequencyTable(makeStringsLambda);
            constexpr auto NumEntries = std::distance(ft.begin(), ft.end());

            // make a queue of probabilities and iterate through as if building the tree
            // tracking the total number of nodes needed. Min-heap priority queue.
            lib::priority_queue<std::size_t, NumEntries, std::greater<std::size_t>> queue;

            // initialise the min-priority queue
            for(auto const &f: ft)
            {
                queue.push(f.frequency);
            }

            // track the number of nodes used. Starting with the leaf nodes we added to the queue.
            // as we "build" the tree bottom up
            std::size_t numNodes{NumEntries};

            while(queue.size() > 1)
            {
                auto n1 = queue.top();
                queue.pop();
                auto n2 = queue.top();
                queue.pop();

                numNodes++; // we would allocate a new node here

                queue.push(n1+n2);
            }

            return numNodes;
        }

        //
        // Build an array of Nodes which link together using indexes to represent the Huffman tree.
        //
        // This flattened tree is then used for generating the encoded strings at compile time, and
        // decoding the strings character by character at run time.
        //
        static constexpr auto BuildHuffmanTree(CallableGivesIterableStringViews auto makeStringsLambda)
        {
            // compute the frequency table and calculate the number of tree nodes needed
            constexpr auto ft = BuildFrequencyTable(makeStringsLambda);
            constexpr auto NumEntries = std::distance(ft.begin(), ft.end());
            constexpr auto NumNodes = CalculateTreeNodeCount(makeStringsLambda);

            // compare "greater" to build a min-heap priority queue
            auto cmpTreeNode = [](TreeNode const* left, TreeNode const* right){ return left->prob > right->prob; };

            // Build the initial TreeNode min-heap. We allocate storage for all the tree nodes
            // in the array, then fill them with the initial leaf nodes, and push them into the min-heap
            std::array<TreeNode, NumNodes> nodes;
            lib::priority_queue<TreeNode*, NumEntries, decltype(cmpTreeNode)> queue;

            for(std::size_t i = 0; i < NumEntries; ++i)
            {
                auto const& f = ft.at(i);
                nodes.at(i) = TreeNode{f.frequency, f.c};
                queue.push(&nodes.at(i));
            }

            // Build the tree by removing 2 TreeNodes from the priority queue and making
            // a new TreeNode with them as children, and add this back to the queue.
            //
            // The top of the min-heap priority queue will be the 2 least used items which
            // could be leaf nodes or previously combined intermediate nodes.
            //
            // When there is only 1 item left, this is the root of the huffman tree
            std::size_t nextNode{NumEntries};   // next available TreeNode slot to allocate

            while(queue.size() > 1)
            {
                auto n1 = queue.top();
                queue.pop();
                auto n2 = queue.top();
                queue.pop();

                // create a new tree node & add to queue
                nodes.at(nextNode) = TreeNode{ n1->prob + n2->prob, n1, n2 };
                queue.push(&nodes.at(nextNode));

                // increment next available node
                nextNode++;
            }

            // queue has 1 element which is the top of the tree
            auto rootNode = queue.top();

            // Do a breadth first traversal of the node tree to allocate indexes in final output
            // array to the nodes. This places the root node at index 0, so we know where to start
            // any traversals.
            lib::list<TreeNode*> remaining;
            std::uint16_t i = 0;
            remaining.push_back(rootNode);
            while(!remaining.empty())
            {
                auto *n = remaining.front();
                remaining.pop_front();

                n->index = i++;

                if(n->link.at(0) != nullptr) {
                    remaining.push_back(n->link.at(0));
                }

                if(n->link.at(1) != nullptr) {
                    remaining.push_back(n->link.at(1));
                }
            }

            // Now that we have numbered the nodes, we iterator over the node data
            // and generate the output ordered set of nodes, using the index from the child nodes
            // for the link indexes.
            //
            // Note that intermediate nodes will always have 2 children, and leaf nodes will have none
            std::array<Node, NumNodes> result;

            for(auto const& n : nodes)
            {
                auto idx = n.index;
                if(n.IsLeaf()) {
                    result.at(idx) = Node{ n.value };
                } else {
                    result.at(idx) = Node{ n.link.at(0)->index, n.link.at(1)->index };
                }
            }

            return result;
        }

    }

    class HuffmanTableEncoder
    {
    public:

        template<std::size_t NUM_TREE_ENTRIES, std::size_t NUM_ENTRIES>
        struct TableData
        {
            static constexpr std::size_t NumEntries = NUM_ENTRIES;
            static constexpr std::size_t NumTreeNodes = NUM_TREE_ENTRIES;

            std::array<huffman::Node, NUM_TREE_ENTRIES> HuffmanTree;
        };

        static constexpr auto Compile(CallableGivesIterableStringViews auto makeStringsLambda)
        {
            // get the string table to work with
            constexpr auto st = makeStringsLambda();

            constexpr auto tree = huffman::BuildHuffmanTree(makeStringsLambda);

            constexpr auto NumStrings = std::distance(st.begin(), st.end());
            constexpr auto NumTreeNodes = std::distance(tree.begin(), tree.end());

            TableData<NumTreeNodes, NumStrings> result;

            std::copy(tree.begin(), tree.end(), result.HuffmanTree.begin());

            return result;
        }

    private:



    };


}


#endif //SQUEEZE_HUFFMANENCODER_H
