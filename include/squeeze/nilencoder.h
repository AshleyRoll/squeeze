#ifndef SQUEEZE_NILENCODER_H
#define SQUEEZE_NILENCODER_H

#include <string_view>
#include <array>
#include <stdexcept>

#include "concepts.h"

namespace squeeze
{
    class NilTableEncoder
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

    template<typename TKey>
    class NilMapEncoder
    {
    public:
        struct Entry
        {
            TKey Key;
            std::size_t Index;
        };

        template<std::size_t STORE_LENGTH, std::size_t NUM_ENTRIES>
        struct TableData
        {
            using KeyType = TKey;

            static constexpr std::size_t NumEntries = NUM_ENTRIES;

            constexpr std::string_view get(KeyType key) const
            {
                // finds the first entry that is no less than the key. May be end(), or higher than the key
                auto entry = std::lower_bound(Entries.begin(), Entries.end(), key,
                                              [](auto const &e, auto const& v){return e.Key < v;});

                if(entry == Entries.end() || (*entry).Key != key)   // could be larger
                    throw std::out_of_range{"key not found"};

                // find the start of the next string to get its start. This might be the last entry
                // in which case the nextStart is the end of the storage
                auto const next = std::next(entry);
                auto const nextStart = (next != Entries.end()) ? (*next).Index : STORE_LENGTH;
                auto const thisStart = (*entry).Index;

                return std::string_view{&Storage.at(thisStart), nextStart - thisStart};
            }

            constexpr bool contains(KeyType key) const
            {
                return Entries.end() != std::find_if(Entries.begin(), Entries.end(), [=](auto const &e){return e.Key == key;});
            }

            std::array<Entry, NUM_ENTRIES> Entries; // stored sorted by Key
            std::array<char, STORE_LENGTH> Storage;
        };

        static constexpr auto Compile(CallableGivesIterableKeyedStringViews<TKey> auto makeStrings)
        {
            // get the string table to work with
            constexpr auto st = makeStrings();

            // work out the size of the result.
            constexpr auto NumStrings = std::distance(st.begin(), st.end());
            constexpr auto TotalStringLength = std::accumulate(
                    st.begin(), st.end(), std::size_t{0},
                    [](auto total, auto const &ksv){ return total + ksv.Value.size(); });

            TableData<TotalStringLength, NumStrings> result;

            // sort the entries before we encode them otherwise our indices won't be right
            // for lookup as that assumes (for efficiency) that index increases to calculate length.
            // we could store a length, but that is additional storage that is not required
            //
            // The string table is const, so unfortunately we need a copy. Sorry compiler
            std::remove_const_t<decltype(st)> working;
            std::copy(st.begin(), st.end(), working.begin());
            std::sort(working.begin(), working.end(), [](auto &a, auto &b) { return a.Key < b.Key;});

            // copy the strings into the result, and build the list of string start locations
            auto loc = result.Storage.begin();
            std::size_t idx = 0;
            for (auto const &ksv : working) {
                auto const end = std::copy(ksv.Value.begin(), ksv.Value.end(), loc);
                result.Entries.at(idx) = Entry{ksv.Key, static_cast<std::size_t>(std::distance(result.Storage.begin(), loc))};

                ++idx;
                loc = end;
            }

            return result;
        }
    };
}

#endif //SQUEEZE_NILENCODER_H
