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

    /**
     * Each shadow map cascade image size expressed in pixels.
     */
    // @todo Currently fixed extent for shadow maps, might need dynamic ones
    constexpr const uint32_t SHADOW_MAP_SIZE = 1024u;
}
