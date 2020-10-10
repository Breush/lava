#pragma once

#include <lava/core/anchor.hpp>
#include <lava/sill/components/i-ui-component.hpp>

namespace lava::sill {
    class TransformComponent;
}

namespace lava::sill {
    class UiAreaComponent : public IUiComponent {
    public:
        UiAreaComponent(Entity& entity);
        UiAreaComponent(Entity& entity, const glm::vec2& extent);

        // IComponent
        static std::string hrid() { return "ui.area"; }

        // IUiComponent
        void verticallyScrolled(float delta, bool& propagate) final;

        // Configuration
        Anchor anchor() const { return m_anchor; }
        void anchor(Anchor anchor) { m_anchor = anchor; updateFromAnchor(); }

        void scrollOffset(const glm::vec2& scrollOffset);
        void scrollMaxOffset(const glm::vec2& scrollMaxOffset) { m_scrollMaxOffset = scrollMaxOffset; }
        void scrollSensibility(float scrollSensibility) { m_scrollSensibility = scrollSensibility; }

    protected:
        void updateFromAnchor();

    private:
        Anchor m_anchor = Anchor::Center; // @fixme Centralize in IUiComponent?
        glm::vec2 m_scrollOffset{0.f, 0.f};
        glm::vec2 m_scrollMinOffset{0.f, 0.f};
        glm::vec2 m_scrollMaxOffset{INFINITY, INFINITY};
        float m_scrollSensibility = 15.f;
    };
}
