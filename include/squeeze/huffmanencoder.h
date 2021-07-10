#ifndef SQUEEZE_HUFFMANENCODER_H
#define SQUEEZE_HUFFMANENCODER_H

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
                using reference = char const;
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




}


#endif //SQUEEZE_HUFFMANENCODER_H
