#ifndef SQUEEZE_NILENCODER_H
#define SQUEEZE_NILENCODER_H

#include <string_view>
#include <array>

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

            constexpr std::string_view operator[](std::size_t idx) const
            {
                // bounds check without exceptions
                if(idx >= NumEntries)
                    return bad_string();

                // find the start of the next string to get its start. This might be the last entry
                // in which case the nextStart is the end of the storage
                auto const nextStart = (idx < NUM_ENTRIES-1) ? m_Entries.at(idx + 1) : STORE_LENGTH;
                auto const thisStart = m_Entries[idx];

                return std::string_view{&m_Storage[thisStart], nextStart - thisStart};
            }

            // provide a value that is an implementation defined value representing a
            // bad key or index was requested.
            constexpr std::string_view bad_string() const
            {
                return std::string_view{};
            }

            std::array<std::size_t, NUM_ENTRIES> m_Entries;
            std::array<char, STORE_LENGTH> m_Storage;
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
            auto loc = result.m_Storage.begin();
            std::size_t idx = 0;
            for (auto &sv : st) {
                auto const end = std::copy(sv.begin(), sv.end(), loc);
                result.m_Entries.at(idx) = static_cast<std::size_t>(std::distance(result.m_Storage.begin(), loc));

                ++idx;
                loc = end;
            }
            return result;
        }
    };
}

#endif //SQUEEZE_NILENCODER_H
