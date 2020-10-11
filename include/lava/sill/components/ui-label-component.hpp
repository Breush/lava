#pragma once

#include <lava/core/u8string.hpp>
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
        UiLabelComponent(Entity& entity, const u8string& text);

        // IComponent
        static std::string hrid() { return "ui.label"; }
        void update(float dt) final;

        // IUiComponent
        void hovered(bool hovered) final;
        void dragStart(const glm::ivec2& /* mousePosition */, bool& propagate) final { propagate = false; m_beingClicked = true; }
        void dragEnd(const glm::ivec2& mousePosition) final;

        // Configuration
        const u8string& text() { return m_text; }
        void backgroundColor(const glm::vec4& backgroundColor) { m_backgroundColor = backgroundColor; updateHovered(); }

        // Callbacks
        void onClicked(ClickedCallback callback) { m_clickedCallback = callback; }

    protected:
        void updateText();
        void updateHovered();

    private:
        FlatComponent& m_flatComponent;

        bool m_beingClicked = false;

        // Configuration
        u8string m_text;
        bool m_textDirty = false;

        // @todo :UiPaddingMerge
        const uint32_t m_fontSize = 23u;
        const uint32_t m_horizontalPadding = 5u;
        glm::vec4 m_backgroundColor = glm::vec4{1};

        // Callbacks
        ClickedCallback m_clickedCallback;
    };
}
