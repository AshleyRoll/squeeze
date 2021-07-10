#ifndef SQUEEZE_CONCEPTS_H
#define SQUEEZE_CONCEPTS_H

#include <string_view>

namespace squeeze
{
    template<typename TKey>
    struct KeyedStringView
    {
        TKey Key;
        std::string_view Value;
    };


    template<typename T>
    concept CallableGivesIterableStringViews = requires(T t) {
        t();            // is callable
        std::input_iterator<decltype(t().begin())>;    // result has iterators
        // result iterates through string_views
        std::is_same_v<decltype(t().begin()), std::string_view>;
    };

    template<typename T, typename K>
    concept CallableGivesIterableKeyedStringViews = requires(T t, K k)
    {
        t();            // is callable
        std::input_iterator<decltype(t().begin())>;    // result has iterators
        // result iterates through string_views
        std::is_same_v<decltype(t().begin()), KeyedStringView<K>>;
    };


}

#endif //SQUEEZE_CONCEPTS_H
