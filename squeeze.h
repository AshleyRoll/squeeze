#ifndef SQUEEZE_SQUEEZE_H
#define SQUEEZE_SQUEEZE_H

#include <string_view>
#include <array>
#include <stdexcept>
#include <numeric>

#include "concepts.h"
#include "nilencoder.h"


namespace squeeze
{

    template<CallableGivesIterableStringViews GET_STRINGS, template<typename T> typename TEncoder = NilEncoder>
    struct StringTable {
        using Encoder = TEncoder<GET_STRINGS>;

        template<typename TData>
        class Impl {
        public:
            constexpr Impl(TData data) : Data{data} {}

            // the number of strings
            constexpr std::size_t count() const { return TData::NumEntries; }

            constexpr auto operator[](std::size_t idx) const
            {
                if (idx >= TData::NumEntries)
                    throw std::out_of_range("index beyond last string");

                return Data[idx];
            }

        private:
            TData Data;
        };

        static constexpr auto Compile()
        {
            constexpr auto data = Encoder::Compile();
            Impl<decltype(data)> result{data};
            return  result;
        }
    };
}

#endif //SQUEEZE_SQUEEZE_H
