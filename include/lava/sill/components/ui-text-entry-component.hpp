#pragma once

#include <lava/sill/components/i-ui-component.hpp>

#include <lava/core/u8string.hpp>

namespace lava::sill {
    class FlatComponent;
}

namespace lava::sill {
    class UiTextEntryComponent : public IUiComponent {
    public:
        using TextChangedCallback = std::function<void(const u8string&)>;

    public:
        UiTextEntryComponent(Entity& entity);
        UiTextEntryComponent(Entity& entity, const u8string& text);

        // IComponent
        static std::string hrid() { return "ui.text-entry"; }
        void update(float dt) final;

        // IUiComponent
        // @todo Have subtext selection!
        void hovered(bool hovered) final;
        void keyPressed(Key key, bool& propagate) final;
        void textEntered(uint32_t codepoint, bool& propagate) final;

        // Configuration
        const u8string& text() { return m_text; }

        // Callbacks
        void onTextChanged(TextChangedCallback callback) { m_textChangedCallbacks.emplace_back(callback); }

    protected:
        void updateText();
        void updateHovered();
        void updateCursor();

    private:
        FlatComponent& m_flatComponent;

        // Configuration
        u8string m_text;
        bool m_textDirty = false;

        // @todo :UiPaddingMerge
        const uint32_t m_fontSize = 30u;
        const uint32_t m_horizontalPadding = 5u;

        uint32_t m_cursorPosition = 0u; // Within text, expressed in bytes

        // Callbacks
        std::vector<TextChangedCallback> m_textChangedCallbacks;
    };
}
