#include <lava/chamber/stb/vorbis.hpp>

// @note Because of how stb_vorbis is done,
// we are forced to reinclude it manually here...
#undef STB_VORBIS_HEADER_ONLY
#include <stb/stb_vorbis.h>
