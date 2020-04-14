#pragma once

#include <lava/sill/components/i-ui-component.hpp>

#include <functional>

namespace lava::sill {
    class FlatComponent;
    class TransformComponent;
}

namespace lava::sill {
    class UiButtonComponent : public IUiComponent {
    public:
        using ClickedCallback = std::function<void()>;

    public:
        UiButtonComponent(GameEntity& entity);
        UiButtonComponent(GameEntity& entity, const std::wstring& text);

        // IComponent
        static std::string hrid() { return "ui.button"; }
        void update(float dt) final;

        // IUiComponent
        void hovered(bool hovered) final;
        void dragStart(const glm::ivec2& /* mousePosition */, bool& propagate) final { propagate = false; beingClicked(true); }
        void dragEnd(const glm::ivec2& mousePosition) final;

        // Configuration
        void text(const std::wstring& text);

        // Callbacks
        void onClicked(ClickedCallback callback) { m_clickedCallback = callback; }

    protected:
        void updateText();
        void updateHovered();
        void updateBeingClicked();

        void beingClicked(bool beingClicked);

    private:
        FlatComponent& m_flatComponent;

        // Configuration
        std::wstring m_text;
        bool m_textDirty = false;

        // User interaction
        bool m_beingClicked = false; // Left click down but not up yet.
        bool m_hovered = false;

        // Callbacks
        ClickedCallback m_clickedCallback;

    };
}
