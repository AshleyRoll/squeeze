#include <iostream>
#include "squeeze.h"




constinit
auto table = squeeze::StringTable([]{
    return std::to_array<std::string_view>({
        "There is little point to using short strings in a compressed string table.",
        "We will include some long strings in the table to test it."
    });
});

enum class StringName
{
    String_1,
    String_2,
    String_3,
    String_4,
};

constinit
auto map = squeeze::StringMap<StringName>([]{
    return std::to_array<squeeze::KeyedStringView<StringName>>(
    {
        // out of order, and not all keys provided
        {StringName::String_4, "This is string 4"},
        {StringName::String_1, "This is string 1"},
        {StringName::String_2, "This is string 2"},
    });
});


void DumpMapEntry(StringName key)
{
    bool contained = map.contains(key);

    std::cout << "Key: " << (int)key << ((contained) ? " Found: " : " Not Found\n");

    if(contained)
        std::cout << map.get(key) << "\n";
}

int main()
{
    std::cout << "String Table:\n";
    for(int i = 0; i < table.count(); ++i)
        std::cout << table[i] << "\n";

    std::cout << "\nStringMap:\n";

    DumpMapEntry(StringName::String_1);
    DumpMapEntry(StringName::String_2);
    DumpMapEntry(StringName::String_3);
    DumpMapEntry(StringName::String_4);

    return 0;
}
