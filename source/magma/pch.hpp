#include "../pch-glm.hpp"
#include "../pch-stl.hpp"

// @note We should be #if given technology used
#include <vulkan/vulkan.hpp>

// @note Compiler mingw's file windev.h defines near and far,
// and vulkan header does include it because of windows.h,
// which mean they cannot be used as identifiers, we fix that.
#undef near
#undef far

#include <lava/chamber.hpp>
#include <lava/core.hpp>
