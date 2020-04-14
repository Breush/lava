#pragma once

#include <lava/sill/components/i-ui-component.hpp>

namespace lava::sill {
    class FlatComponent;
}

namespace lava::sill {
    class UiTextEntryComponent : public IUiComponent {
    public:
        using TextChangedCallback = std::function<void(const std::wstring&)>;

    public:
        UiTextEntryComponent(GameEntity& entity);
        UiTextEntryComponent(GameEntity& entity, const std::wstring& text);

        // IComponent
        static std::string hrid() { return "ui.text-entry"; }
        void update(float dt) final;

        // IUiComponent
        // @todo Have subtext selection!
        void hovered(bool hovered) final;
        void textEntered(Key key, wchar_t code, bool& propagate) final;

        // Configuration
        const std::wstring& text() { return m_text; }

        // Callbacks
        void onTextChanged(TextChangedCallback callback) { m_textChangedCallbacks.emplace_back(callback); }

    protected:
        void updateText();
        void updateHovered();
        void updateCursor();

    private:
        FlatComponent& m_flatComponent;

        // Configuration
        std::wstring m_text;
        bool m_textDirty = false;
        const uint32_t m_fontSize = 30u;
        const uint32_t m_horizontalPadding = 2u;

        uint32_t m_cursorPosition = 0u; // Within text

        // Callbacks
        std::vector<TextChangedCallback> m_textChangedCallbacks;
    };
}
