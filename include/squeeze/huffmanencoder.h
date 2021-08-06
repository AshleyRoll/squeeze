#ifndef SQUEEZE_HUFFMANENCODER_H
#define SQUEEZE_HUFFMANENCODER_H

#include <limits>
#include <algorithm>
#include <array>

#include "concepts.h"
#include "lib/priority_queue.h"
#include "lib/list.h"

namespace squeeze {

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

    namespace huffman {
        struct CharFrequency {
            char c;
            std::size_t f;
        };

        struct TreeNode
        {
            std::size_t prob;
            char value;
            std::size_t index;

            TreeNode *zero;
            TreeNode *one;

            constexpr TreeNode(std::size_t p, char v)
                    : prob{p}, value{v}, index{0}, zero{nullptr}, one{nullptr}
            {}

            constexpr TreeNode(std::size_t p, TreeNode* c0, TreeNode *c1)
                    : prob{p}, value{0}, index{0}, zero{c0}, one{c1}
            {}

            constexpr TreeNode()
                    : prob{0}, value{0}, index{0}, zero{nullptr}, one{nullptr}
            {}

            constexpr bool IsLeaf() const { return zero == nullptr || one == nullptr; }
        };

        struct Node
        {
            char value;         // character or '\0' if intermediate node
            std::size_t zero;   // index child node for "zero" bit. if 0 (root node), there is no child
            std::size_t one;    // index child node for "one" bit. if 0 (root node), there is no child
        };


        static constexpr auto CountFrequency(CallableGivesIterableStringViews auto makeStringsLambda)
        {
            // get the string table to work with
            constexpr auto st = makeStringsLambda();

            std::array<std::size_t, std::numeric_limits<std::string_view::value_type>::max()> counts;
            counts.fill(0);

            for(auto &s : st)
                for(auto c : s)
                    counts[static_cast<std::size_t>(c)] += 1;

            return counts;
        }

        static constexpr auto BuildFrequencyTable(CallableGivesIterableStringViews auto makeStringsLambda)
        {
            constexpr auto counts = CountFrequency(makeStringsLambda);

            constexpr auto entries = std::count_if(counts.begin(), counts.end(), [](auto i){return i != 0;});

            std::array<CharFrequency, entries> ft;
            std::size_t e = 0;
            for(std::size_t i = 0; i < counts.size(); ++i)
            {
                if(counts[i] == 0) continue;
                ft[e++] = {static_cast<char>(i), counts[i]};
            }

            // sort least common to most common
            std::sort(ft.begin(), ft.end(), [](auto &a, auto &b) { return a.f < b.f; });

            return ft;
        }

        static constexpr auto CalculateTreeNodeCount(CallableGivesIterableStringViews auto makeStringsLambda)
        {
            // compute the frequency table
            constexpr auto ft = BuildFrequencyTable(makeStringsLambda);
            constexpr auto NumEntries = std::distance(ft.begin(), ft.end());

            // make a queue of probabilities and iterate through as if building the tree
            // tracking the total number of nodes needed
            lib::priority_queue<std::size_t, NumEntries, std::greater<std::size_t>> queue;

            for(auto const &f: ft)
            {
                queue.push(f.f);
            }

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

        static constexpr auto BuildHuffmanTree(CallableGivesIterableStringViews auto makeStringsLambda)
        {
            // compute the frequency table
            constexpr auto ft = BuildFrequencyTable(makeStringsLambda);
            constexpr auto NumEntries = std::distance(ft.begin(), ft.end());
            constexpr auto NumNodes = CalculateTreeNodeCount(makeStringsLambda);

            // compare "greater" to build a min-heap
            auto cmpTreeNode = [](TreeNode const* left, TreeNode const* right){ return left->prob > right->prob; };

            // Build the initial TreeNode min-heap
            std::array<TreeNode, NumNodes> nodes;
            lib::priority_queue<TreeNode*, NumEntries, decltype(cmpTreeNode)> queue;

            for(std::size_t i = 0; i < NumEntries; ++i)
            {
                auto const& f = ft[i];
                nodes[i] = TreeNode{f.f, f.c};
                queue.push(&nodes[i]);
            }

            // Build the tree by removing 2 TreeNodes from the priority queue and making
            // a new TreeNode with them as children, and add this back to the queue.
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
                nodes[nextNode] = TreeNode{ n1->prob + n2->prob, n1, n2 };
                queue.push(&nodes[nextNode]);

                // increment next available node
                nextNode++;
            }

            // queue has 1 element which is the top of the tree
            auto rootNode = queue.top();

            // Do a breadth first traversal of the node tree to allocate indexes in final output
            // structure to the nodes.
            lib::list<TreeNode*> remaining;
            std::size_t i = 0;
            remaining.push_back(rootNode);
            while(!remaining.empty())
            {
                auto n = remaining.front();
                remaining.pop_front();

                n->index = i++;

                if(n->zero != nullptr)
                    remaining.push_back(n->zero);

                if(n->one != nullptr)
                    remaining.push_back(n->one);
            }

            remaining.clear();

            // Now that we have numbered the nodes, we iterator over the node data
            // and generate the output ordered set of nodes, using the index from the child nodes
            // for references.
            //
            // Note that intermediate nodes will always have 2 children, and leaf nodes will have none
            std::array<Node, NumNodes> result;

            for(auto const& n : nodes)
            {
                auto idx = n.index;
                if(n.IsLeaf()) {
                    result[idx] = Node{ n.value, 0, 0 };
                } else {
                    result[idx] = Node{ 0, n.zero->index, n.one->index };
                }
            }

            return result;
        }

    }

    class HuffmanTableEncoder
    {
    public:

        template<std::size_t TREE_SIZE, std::size_t NUM_ENTRIES>
        struct TableData
        {
            static constexpr std::size_t NumEntries = NUM_ENTRIES;
            static constexpr std::size_t NumTreeNodes = TREE_SIZE;

            std::array<huffman::Node, TREE_SIZE> HuffmanTree;

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
