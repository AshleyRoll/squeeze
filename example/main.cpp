#include <cstdio>

#include "../include/squeeze/squeeze.h"


enum class Key {
    String_1,
    String_2,
    String_3
};

static constexpr auto buildMapStrings = [] {
    return std::to_array<squeeze::KeyedStringView<Key>>({
      // out of order and missing a value
      { Key::String_3, "There is little point to using short strings in a compressed string table." },
      { Key::String_1, "We will include some long strings in the table to test it." },
    });
};

// defaults to huffman encoding
static constexpr auto map = squeeze::StringMap<Key>(buildMapStrings);

int main()
{
    // grab the first compressed string and dump it to stdout.
    static constexpr auto str1 = map.get(Key::String_1);
    for (const auto &c : str1) {
        putc(c, stdout);
    }

    putc('\n', stdout);
    return 0;
}
