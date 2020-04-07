#pragma once

#include <lava/sill/components/i-component.hpp>

#include <functional>

namespace lava::sill {
    class FlatComponent;
    class TransformComponent;
}

namespace lava::sill {
    class UiButtonComponent : public IComponent {
    public:
        using ClickedCallback = std::function<void()>;

    public:
        UiButtonComponent(GameEntity& entity);
        UiButtonComponent(GameEntity& entity, const std::wstring& text);
        ~UiButtonComponent();

        // IComponent
        static std::string hrid() { return "ui.button"; }
        void update(float dt) final;

        // Configuration
        void text(const std::wstring& text);

        // UI manager interaction
        void hovered(bool hovered);
        bool checkHovered(const glm::ivec2& mousePosition);
        void dragStart(const glm::ivec2& /* mousePosition */) { beingClicked(true); }
        void dragEnd(const glm::ivec2& mousePosition);

        // Callbacks
        void onClicked(ClickedCallback callback) { m_clickedCallback = callback; }

    protected:
        void updateText();
        void updateHovered();
        void updateBeingClicked();

        void beingClicked(bool beingClicked);

    private:
        TransformComponent& m_transformComponent;
        FlatComponent& m_flatComponent;

        // Configuration
        glm::vec2 m_extent = glm::vec2{1.f};
        std::wstring m_text;
        bool m_textDirty = false;

        // user interaction
        bool m_beingClicked = false; // Left click down but not up yet.
        bool m_hovered = false;

        // Callbacks
        ClickedCallback m_clickedCallback;

    };
}
