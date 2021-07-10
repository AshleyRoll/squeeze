#ifndef SQUEEZE_SQUEEZE_H
#define SQUEEZE_SQUEEZE_H

#include <string_view>
#include <array>
#include <numeric>

#include "concepts.h"
#include "nilencoder.h"


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



        template<CallableGivesIterableStringViews GET_STRINGS, template<typename T> typename TEncoder>
        static constexpr auto CompileTable() {
            constexpr auto data = TEncoder<GET_STRINGS>::Compile();
            StringTableDataImpl<decltype(data)> result{data};
            return result;
        }

        template<typename TKey, CallableGivesIterableKeyedStringViews<TKey> GET_STRINGS, template<typename K, typename T> typename TEncoder>
        static constexpr auto CompileMap() {
            constexpr auto data = TEncoder<TKey, GET_STRINGS>::Compile();
            StringMapDataImpl<decltype(data)> result{data};
            return result;
        }

    }


    template<template<typename T> typename TEncoder = NilTableEncoder>
    constexpr auto StringTable(auto makeStringsLambda)
    {
        return impl::CompileTable<decltype(makeStringsLambda), TEncoder>();
    }

    template<typename TKey, template<typename K, typename T> typename TEncoder = NilMapEncoder>
    constexpr auto StringMap(auto makeStringsLambda)
    {
        return impl::CompileMap<TKey, decltype(makeStringsLambda), TEncoder>();
    }

}

#endif //SQUEEZE_SQUEEZE_H
