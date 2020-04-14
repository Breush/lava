#pragma once

#include <lava/sill/components/i-component.hpp>

namespace lava::sill {
    class TransformComponent;
}

namespace lava::sill {
    class IUiComponent : public IComponent {
    public:
        using ClickedCallback = std::function<void()>;

    public:
        IUiComponent(GameEntity& entity);
        ~IUiComponent();

        const glm::vec2& extent() const { return m_extent; }

        // UI manager interaction
        bool checkHovered(const glm::ivec2& mousePosition);

        virtual void hovered(bool /* hovered */) {}
        virtual void dragStart(const glm::ivec2& /* mousePosition */, bool& /* propagate */) {}
        virtual void dragEnd(const glm::ivec2& /* mousePosition */) {}
        virtual void textEntered(Key /* key */, wchar_t /* code */, bool& /* propagate */) {}

    protected:
        TransformComponent& m_transformComponent;

        // Configuration
        glm::vec2 m_extent = glm::vec2{1.f};

        // User interaction
        bool m_hovered = false;

    };
}
