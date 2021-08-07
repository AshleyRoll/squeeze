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
            constexpr StringTableDataImpl(TData data) : Data{data} {}

            // the number of strings
            constexpr std::size_t count() const { return TData::NumEntries; }

            constexpr auto operator[](std::size_t idx) const {
                return Data[idx];
            }

        private:
            TData Data;
        };

        template<typename TData>
        class StringMapDataImpl {
        public:
            using KeyType = typename TData::KeyType;

            constexpr StringMapDataImpl(TData data) : Data{data} {}

            // the number of strings
            constexpr std::size_t count() const { return TData::NumEntries; }

            constexpr auto get(KeyType key) const {
                return Data.get(key);
            }

            constexpr bool contains(KeyType key) const {
                return Data.contains(key);
            }

        private:
            TData Data;
        };



        template<typename TEncoder>
        static constexpr auto CompileTable(CallableGivesIterableStringViews auto f) {
            constexpr auto data = TEncoder::Compile(f);
            StringTableDataImpl<decltype(data)> result{data};
            return result;
        }

        template<typename TKey, template<typename K> typename TEncoder>
        static constexpr auto CompileMap(CallableGivesIterableKeyedStringViews<TKey> auto f) {
            constexpr auto data = TEncoder<TKey>::Compile(f);
            StringMapDataImpl<decltype(data)> result{data};
            return result;
        }

    }


    template<typename TEncoder = NilTableEncoder>
    constexpr auto StringTable(CallableGivesIterableStringViews auto makeStringsLambda)
    {
        return impl::CompileTable<TEncoder>(makeStringsLambda);
    }

    template<typename TKey, template<typename K> typename TEncoder = NilMapEncoder>
    constexpr auto StringMap(CallableGivesIterableKeyedStringViews<TKey> auto makeStringsLambda)
    {
        return impl::CompileMap<TKey, TEncoder>(makeStringsLambda);
    }

}

#endif //SQUEEZE_SQUEEZE_H
