#ifndef SQUEEZE_PRIORITY_QUEUE_H
#define SQUEEZE_PRIORITY_QUEUE_H

#include <array>
#include <functional>
#include <stdexcept>

namespace squeeze::lib
{

    //
    // A simple priority queue with a fixed capacity that can be used at compile time in constexpr contexts
    //
    template<typename T, std::size_t N, typename Compare = std::less<T>>
    class priority_queue
    {
    public:
        using container_type = std::array<T, N>;
        using value_compare = Compare;
        using value_type = typename container_type::value_type;
        using size_type = typename container_type::size_type;
        using reference = typename container_type::reference;
        using const_reference = typename container_type::const_reference;

        [[nodiscard]] constexpr size_type max_size() const { return N; }
        [[nodiscard]] constexpr size_type size() const { return m_Count; }
        [[nodiscard]] constexpr bool empty() const { return m_Count == 0; }

        [[nodiscard]] constexpr const_reference top() const { return m_Data.front(); }

        constexpr void push(const value_type& v)
        {
            if(size() == max_size()) {
                throw std::out_of_range{"priority_queue full"};
            }

            m_Data[m_Count++] = v;

            if(m_Count > 1) {
                std::push_heap(&m_Data[0], &m_Data[m_Count], value_compare{});
            }
        }

        constexpr void pop()
        {
            if(empty()) {
                throw std::out_of_range{"priority_queue empty"};
            }

            std::pop_heap(&m_Data[0], &m_Data[m_Count], value_compare{});

            m_Data[m_Count-1].~T();
            m_Count--;
        }

    private:
        size_type m_Count{0};
        container_type m_Data;
    };


}

#endif //SQUEEZE_PRIORITY_QUEUE_H
