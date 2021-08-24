#ifndef SQUEEZE_NILENCODER_H
#define SQUEEZE_NILENCODER_H

#include <string_view>
#include <array>
#include <stdexcept>

#include "concepts.h"

namespace squeeze
{
    class NilEncoder
    {
    public:
        template<std::size_t STORE_LENGTH, std::size_t NUM_ENTRIES>
        struct TableData
        {
            static constexpr std::size_t NumEntries = NUM_ENTRIES;

            constexpr std::string_view operator[](std::size_t index) const
            {
                // find the start of the next string to get its start. This might be the last entry
                // in which case the nextStart is the end of the storage
                auto const nextStart = (index < NUM_ENTRIES-1) ? Entries.at(index+1) : STORE_LENGTH;
                auto const thisStart = Entries.at(index);

                return std::string_view{&Storage.at(thisStart), nextStart - thisStart};
            }

            std::array<std::size_t, NUM_ENTRIES> Entries;
            std::array<char, STORE_LENGTH> Storage;
        };

        static constexpr auto Compile(CallableGivesIterableStringViews auto makeStringsLambda)
        {
            // get the string table to work with
            constexpr auto st = makeStringsLambda();

            // work out the size of the result.
            constexpr auto NumStrings = std::distance(st.begin(), st.end());
            constexpr auto TotalStringLength = std::accumulate(
                    st.begin(), st.end(), std::size_t{0},
                    [](auto total, auto const &sv){ return total + sv.size(); });

            TableData<TotalStringLength, NumStrings> result;

            // copy the strings into the result, and build the list of string start locations
            auto loc = result.Storage.begin();
            std::size_t idx = 0;
            for (auto &sv : st) {
                auto const end = std::copy(sv.begin(), sv.end(), loc);
                result.Entries.at(idx) = static_cast<std::size_t>(std::distance(result.Storage.begin(), loc));

                ++idx;
                loc = end;
            }
            return result;
        }
    };
}

#endif //SQUEEZE_NILENCODER_H
