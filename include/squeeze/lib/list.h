#ifndef SQUEEZE_LIST_H
#define SQUEEZE_LIST_H

#include <cstddef>

namespace squeeze::lib
{
    //
    // A simple double-linked list that can be used at compile time in constexpr contexts.
    //
    template<typename T>
    class list
    {
    public:
        using value_type = T;

        constexpr void push_back(const value_type& v)
        {
            Node *n = new Node{v, nullptr, tail};
            if(tail != nullptr) {
                tail->next = n;
            }
            tail = n;
            if(head == nullptr) {
                head = n;
            }
        }

        constexpr void pop_back()
        {
            Node *n = tail;
            if(n != nullptr) {
                tail = n->prev;
                if(tail != nullptr) {
                    tail->next = nullptr;
                }
                if(head == n) {
                    head = tail;
                }

                n->value.~T();
                delete n;
            }
        }

        constexpr void push_front(const value_type& v)
        {
            Node *n = new Node{v, head, nullptr};
            if(head != nullptr) {
                head->prev = n;
            }
            head = n;
            if(tail == nullptr) {
                tail = n;
            }
        }

        constexpr void pop_front()
        {
            Node *n = head;
            if(n != nullptr) {
               head = n->next;
               if(head != nullptr) {
                   head->prev = nullptr;
               }
               if(tail == n) {
                   tail = head;
               }

               n->value.~T();
               delete n;
            }
        }

        [[nodiscard]] constexpr bool empty() const
        {
            return head == nullptr;
        }

        [[nodiscard]] constexpr value_type front() const
        {
           return head->value;
        }

        [[nodiscard]] constexpr value_type back() const
        {
            return tail->value;
        }

        constexpr void clear()
        {
            while(!empty())
                pop_front();
        }

        constexpr list() = default;

        constexpr ~list()
        {
            // ensure all memory is deleted on destruction
            clear();
        }

    private:
        struct Node
        {
            value_type value;

            Node* next;
            Node* prev;

            constexpr Node(const value_type& v, Node* n, Node* p)
                : value{v}, next{n}, prev{p}
            {}
        };

        Node *head{nullptr};
        Node *tail{nullptr};
    };


}

#endif //SQUEEZE_LIST_H
