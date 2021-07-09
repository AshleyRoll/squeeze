#ifndef SQUEEZE_CONCEPTS_H
#define SQUEEZE_CONCEPTS_H


namespace squeeze {

    template<typename T>
    concept CallableGivesIterableStringViews = requires(T t) {
        t();            // is callable
        t().begin();    // result has iterators
        t().end();
        // result iterates through string_views
        std::is_same_v<decltype(t().begin()), std::string_view>;
    };

}

#endif //SQUEEZE_CONCEPTS_H
