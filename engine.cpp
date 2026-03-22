#include "engine.h"
#include <iostream>

int main()
{
    std::cout << "Engine starting..." << std::endl;
    // Use a symbol from engine_render to ensure linkage works if implemented
    // For header-only engine_render, this will compile fine as long as headers exist
    return 0;
}
