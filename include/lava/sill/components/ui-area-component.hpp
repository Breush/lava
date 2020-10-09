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

        void maxOffset(const glm::vec2& maxOffset) { m_maxOffset = maxOffset; }

        void scrollSensibility(float scrollSensibility) { m_scrollSensibility = scrollSensibility; }

    protected:
        void updateFromAnchor();

    private:
        Anchor m_anchor = Anchor::Center; // @fixme Centralize in IUiComponent?
        glm::vec2 m_accumulatedOffset{0.f, 0.f};
        glm::vec2 m_minOffset{0.f, 0.f};
        glm::vec2 m_maxOffset{INFINITY, INFINITY};
        float m_scrollSensibility = 15.f;
    };
}
