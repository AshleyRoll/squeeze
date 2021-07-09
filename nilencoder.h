#ifndef SQUEEZE_NILENCODER_H
#define SQUEEZE_NILENCODER_H

#include <string_view>
#include <array>
#include "concepts.h"

namespace squeeze
{
    template<CallableGivesIterableStringViews GET_STRINGS>
    class NilEncoder
    {
    public:
        template<std::size_t STORE_LENGTH, std::size_t NUM_ENTRIES>
        struct TableData {
            static constexpr std::size_t NumEntries = NUM_ENTRIES;

            constexpr std::string_view operator[](std::size_t index) const
            {
                // find the start of the next string
                auto const nextStart = (index < NUM_ENTRIES -1) ? Entries[index+1] : STORE_LENGTH;
                auto const thisStart = Entries[index];

                return std::string_view{&Storage[thisStart], nextStart - thisStart};
            }


            std::array<std::size_t, NUM_ENTRIES> Entries;
            std::array<char, STORE_LENGTH> Storage;
        };

        static constexpr auto Compile()
        {
            // get the string table to work with
            constexpr auto st = GET_STRINGS{}();

            // work out the size of the result.
            constexpr auto NumStrings = std::distance(st.begin(), st.end());
            constexpr auto TotalStringLength = std::accumulate(
                    st.begin(), st.end(), 0,
                    [](auto total, std::string_view const &sv){ return total + sv.size(); });

            TableData<TotalStringLength, NumStrings> result;

            // copy the strings into the result, removing null terminations
            // and build the list of string start locations
            auto loc = result.Storage.begin();
            std::size_t idx = 0;
            for (const auto &sv : st) {
                const auto end = std::copy(sv.begin(), sv.end(), loc);
                result.Entries[idx] = static_cast<std::size_t>(std::distance(result.Storage.begin(), loc));

                ++idx;
                loc = end;
            }
            return result;
        }

    };

}

#endif //SQUEEZE_NILENCODER_H
