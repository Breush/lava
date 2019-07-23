#pragma once

/**
 * Specific config for vulkan Afts.
 */
namespace lava::magma {
    /**
     * Each frame will be renderer with a frame id being within [0 .. FRAME_IDS_COUNT],
     * this is independent from the swapchain and is incremented during each scene update.
     * Consider using it when you update some buffer that might be used during current render,
     * which is the case with Shadows's ubo.
     */
    constexpr static const uint8_t FRAME_IDS_COUNT = 3u;
}
