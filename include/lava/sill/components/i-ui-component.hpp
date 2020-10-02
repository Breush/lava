#pragma once

#include <lava/sill/components/i-component.hpp>

namespace lava::sill {
    class TransformComponent;
}

namespace lava::sill {
    class IUiComponent : public IComponent {
    public:
        using ClickedCallback = std::function<void()>;
        using ExtentChangedCallback = std::function<void(const glm::vec2&)>;

    public:
        IUiComponent(Entity& entity);
        ~IUiComponent();

        // External API
        const glm::vec2& extent() const { return m_extent; }
        void onExtentChanged(ExtentChangedCallback extentChangedCallback) {
            m_extentChangedCallbacks.emplace_back(extentChangedCallback);
        }

        // UI manager interaction
        // Transforms screen-space coordinates to object-space coordinates from top-left corner.
        glm::vec2 topLeftRelativePosition(const glm::ivec2& mousePosition) const;
        bool checkHovered(const glm::ivec2& mousePosition);

        virtual void hovered(bool /* hovered */) {}
        virtual void dragStart(const glm::ivec2& /* mousePosition */, bool& /* propagate */) {}
        virtual void dragEnd(const glm::ivec2& /* mousePosition */) {}
        virtual void textEntered(Key /* key */, wchar_t /* code */, bool& /* propagate */) {}

    protected:
        // Internal API
        void warnExtentChanged() const {
            for (const auto& callback : m_extentChangedCallbacks) {
                callback(m_extent);
            }
        }

    protected:
        TransformComponent& m_transformComponent;

        // Internal configuration
        glm::vec2 m_hoveringExtent = glm::vec2{1.f};
        glm::vec2 m_hoveringOffset = glm::vec2{0.f};

        // Configuration
        glm::vec2 m_extent = glm::vec2{1.f};

        // User interaction
        bool m_hovered = false;

    private:
        std::vector<ExtentChangedCallback> m_extentChangedCallbacks;
    };
}
