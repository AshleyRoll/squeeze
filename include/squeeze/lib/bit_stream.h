#ifndef SQUEEZE_BIT_STREAM_H
#define SQUEEZE_BIT_STREAM_H

#include <climits>
#include <cstdint>
#include <array>
#include <stdexcept>

namespace squeeze::lib
{
   //
   // A simple class to store a fixed number of bits and be able to access them
   // by index
   //
   template<std::size_t NUM_BITS>
   class bit_stream
   {
   public:
       constexpr std::size_t size() const { return NumBits; }

       constexpr bit_stream()
       {
           // ensure we initialise all storage locations to satisfy constexpr context constraints
           m_Storage.fill(0);
       }

       constexpr void set(std::size_t idx)
       {
           if(idx >= size())
               throw std::out_of_range{"bad index"};

           auto offset = idx / BitsPerStorageElement;
           auto bit = idx % BitsPerStorageElement;

           storage_type mask = static_cast<storage_type>(1 << bit);
           m_Storage[offset] |= mask;
       }

       constexpr void clear(std::size_t idx)
       {
           if(idx >= size())
               throw std::out_of_range{"bad index"};

           auto offset = idx / BitsPerStorageElement;
           auto bit = idx % BitsPerStorageElement;

           storage_type mask = static_cast<storage_type>(~(1 << bit));
           m_Storage[offset] &= mask;
       }

       constexpr bool at(std::size_t idx) const
       {
           if(idx >= size())
               throw std::out_of_range{"bad index"};

           auto offset = idx / BitsPerStorageElement;
           auto bit = idx % BitsPerStorageElement;

           storage_type mask = static_cast<storage_type>(1 << bit);
           return (m_Storage[offset] & mask) != 0;
       }

   private:
       using storage_type = std::uint8_t;

       constexpr static std::size_t BitsPerStorageElement = sizeof(storage_type) * CHAR_BIT;
       constexpr static std::size_t NumStorageElements = (NUM_BITS/BitsPerStorageElement) + (NUM_BITS%BitsPerStorageElement>0?1:0);
       constexpr static std::size_t NumBits = NUM_BITS;

       std::array<storage_type, NumStorageElements> m_Storage;
   };



}


#endif //SQUEEZE_BIT_STREAM_H
