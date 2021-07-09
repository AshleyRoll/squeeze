#include <iostream>
#include "squeeze.h"


constinit auto table = squeeze::StringTable<decltype([]{
    return std::to_array<std::string_view>({
        "There is little point to using short strings in a compressed string table.",
        "We will include some long strings in the table to test it."
    });
})>::Compile();

int main()
{
    for(int i = 0; i < table.count(); ++i)
        std::cout << table[i] << "\n";

    return 0;
}
