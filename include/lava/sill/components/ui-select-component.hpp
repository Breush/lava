#pragma once

#include <lava/sill/components/i-ui-component.hpp>

namespace lava::sill {
    class FlatComponent;
}

namespace lava::sill {
    class UiSelectComponent : public IUiComponent {
    public:
        using IndexChangedCallback = std::function<void(uint8_t, const std::wstring&)>;

    public:
        UiSelectComponent(Entity& entity);
        UiSelectComponent(Entity& entity, std::vector<std::wstring> options, uint8_t index);

        // IComponent
        static std::string hrid() { return "ui.select"; }
        void update(float dt) final;

        // IUiComponent
        void hovered(bool hovered) final;
        void dragStart(const glm::ivec2& /* mousePosition */, bool& propagate) final { propagate = false; m_beingClicked = true; }
        void dragEnd(const glm::ivec2& mousePosition) final;

        // Configuration
        uint8_t index() { return m_index; }

        // Callbacks
        void onIndexChanged(IndexChangedCallback callback) { m_indexChangedCallbacks.emplace_back(callback); }

    protected:
        void updateText();
        void updateHovered();

    private:
        FlatComponent& m_flatComponent;

        // Configuration
        std::vector<std::wstring> m_options;
        uint8_t m_index = 0xFF;
        bool m_textDirty = false;

        // User interaction
        bool m_beingClicked = false; // Left click down but not up yet.

        // @todo :UiPaddingMerge
        const uint32_t m_fontSize = 30u;
        const uint32_t m_horizontalPadding = 5u;

        // Callbacks
        std::vector<IndexChangedCallback> m_indexChangedCallbacks;
    };
}
