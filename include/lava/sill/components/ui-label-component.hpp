#pragma once

#include <lava/sill/components/i-ui-component.hpp>

namespace lava::sill {
    class FlatComponent;
}

namespace lava::sill {
    class UiLabelComponent : public IUiComponent {
    public:
        using ClickedCallback = std::function<void()>;

    public:
        UiLabelComponent(Entity& entity);
        UiLabelComponent(Entity& entity, const std::wstring& text);

        // IComponent
        static std::string hrid() { return "ui.label"; }
        void update(float dt) final;

        // IUiComponent
        void hovered(bool hovered) final;
        void dragStart(const glm::ivec2& /* mousePosition */, bool& propagate) final { propagate = false; m_beingClicked = true; }
        void dragEnd(const glm::ivec2& mousePosition) final;

        // Configuration
        const std::wstring& text() { return m_text; }

        // Callbacks
        void onClicked(ClickedCallback callback) { m_clickedCallback = callback; }

    protected:
        void updateText();
        void updateHovered();

    private:
        FlatComponent& m_flatComponent;

        bool m_beingClicked = false;

        // Configuration
        std::wstring m_text;
        bool m_textDirty = false;

        // @todo :UiPaddingMerge
        const uint32_t m_fontSize = 23u;
        const uint32_t m_horizontalPadding = 5u;

        // Callbacks
        ClickedCallback m_clickedCallback;
    };
}
