#pragma once

#include <functional>
#include <lava/core/alignment.hpp>
#include <lava/core/anchor.hpp>
#include <lava/core/u8string.hpp>

namespace lava::sill {
    class FlatComponent;
    struct FlatNode;
}

namespace lava::sill::makers {
    struct TextFlatOptions {
        std::string fontHrid = "default";
        uint32_t fontSize = 32u;
        Anchor anchor = Anchor::Center;
        Alignment alignment = Alignment::Center;
        FloatExtent2d* extentPtr = nullptr;
    };

    std::function<FlatNode&(FlatComponent&)> textFlatMaker(const u8string& text,
                                                           const TextFlatOptions& options = TextFlatOptions());
}
