#ifndef SQUEEZE_SQUEEZE_H
#define SQUEEZE_SQUEEZE_H

#include <string_view>
#include <array>
#include <numeric>

#include "concepts.h"
#include "nilencoder.h"
#include "huffmanencoder.h"

namespace squeeze
{
    namespace impl {
        template<typename TData>
        class StringTableDataImpl {
        public:
            constexpr StringTableDataImpl(TData data) : m_Data{data} {}

            // the number of strings
            constexpr std::size_t count() const { return TData::NumEntries; }

            // get the string at the given index. idx should be 0 to count()-1.
            // an index outside this bound will return an empty string representation
            constexpr auto operator[](std::size_t idx) const {
                return m_Data[idx];
            }

        private:
            TData m_Data;
        };


        template<typename TKey>
        struct KeyMap
        {
            TKey Key;
            std::size_t Index;
        };

        template<typename TKey, typename TData>
        class StringMapDataImpl {
        public:
            constexpr static std::size_t NumEntries = TData::NumEntries;

            using KeyType = TKey;
            using KeyMapType = KeyMap<TKey>;
            using LookupType = std::array<KeyMapType, NumEntries>;

            constexpr StringMapDataImpl(LookupType lookup, TData data) : m_Lookup{lookup}, m_Data{data} {}

            // the number of strings
            constexpr std::size_t count() const { return TData::NumEntries; }

            // Get the string for the given key. Note that if the string is not present in the
            // map, an empty result will be returned. Use contains() to determine if the string exists.
            constexpr auto get(KeyType key) const {
                // finds the first entry that is no less than the key. May be end(), or higher than the key
                auto entry = std::lower_bound(
                        m_Lookup.begin(), m_Lookup.end(), key,
                        [](auto const &e, auto const& v){return e.Key < v;}
                );

                if(entry == m_Lookup.end() || (*entry).Key != key) {  // could be larger key value
                    // use the bad_string() result. this is an "empty" string however that is
                    // represented by the encoded data.
                    return m_Data.bad_string();
                }

                return m_Data[(*entry).Index];
            }

            // Determine if the map contains the given key. If this returns false,
            // a call to get() for that key will return an empty result.
            constexpr bool contains(KeyType key) const {
                return m_Lookup.end() != std::find_if(
                    m_Lookup.begin(),
                    m_Lookup.end(),
                    [=](auto const &e){return e.Key == key;});
            }

        private:
            LookupType m_Lookup;
            TData m_Data;
        };


        template<typename TEncoder>
        static constexpr auto CompileTable(CallableGivesIterableStringViews auto f) {
            constexpr auto data = TEncoder::Compile(f);
            StringTableDataImpl<decltype(data)> result{data};
            return result;
        }

        template<typename TKey>
        static constexpr auto MapToStrings(CallableGivesIterableKeyedStringViews<TKey> auto f) -> CallableGivesIterableStringViews auto
        {
            return [=]() {
                constexpr auto stringmap = f();
                constexpr auto NumStrings = std::distance(stringmap.begin(), stringmap.end());

                std::array<std::string_view, NumStrings> result;
                std::size_t idx{0};
                for(auto const &v : stringmap) {
                    result.at(idx++) = v.Value;
                }
                return result;
            };
        }

        template<typename TKey, typename TEncoder>
        static constexpr auto CompileMap(CallableGivesIterableKeyedStringViews<TKey> auto f) {
            constexpr auto map = f();
            constexpr auto NumStrings = std::distance(map.begin(), map.end());

            // encode the string using the table encoder
            constexpr auto data = TEncoder::Compile(MapToStrings<TKey>(f));

            // build the lookup for key->index mapping
            std::array<KeyMap<TKey>, NumStrings> lookup;

            std::size_t idx{0};
            for(auto const &v : map) {
                lookup.at(idx).Key = v.Key;
                lookup.at(idx).Index = idx;
                ++idx;
            }
            // ensure it is sorted by key for searching
            std::sort(lookup.begin(), lookup.end(), [](auto &a, auto &b) { return a.Key < b.Key;});

            // build the final result with the lookup and data
            StringMapDataImpl<TKey, decltype(data)> result{lookup, data};
            return result;
        }
    }


    template<typename TEncoder = HuffmanEncoder>
    constexpr auto StringTable(CallableGivesIterableStringViews auto makeStringsLambda)
    {
        return impl::CompileTable<TEncoder>(makeStringsLambda);
    }

    template<typename TKey, typename TEncoder = HuffmanEncoder>
    constexpr auto StringMap(CallableGivesIterableKeyedStringViews<TKey> auto makeStringsLambda)
    {
        return impl::CompileMap<TKey, TEncoder>(makeStringsLambda);
    }

}

#endif //SQUEEZE_SQUEEZE_H
