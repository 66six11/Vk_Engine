// This file ensures engine_render is not treated as a header-only library
// This helps IDE (Visual Studio, CLion, etc.) to properly show header files as part of the target

#include "RenderEngine.h"

namespace render {
    // Empty namespace to prevent unused warnings
    void dummy_function_to_prevent_empty_translation_unit() {}
} // namespace render
