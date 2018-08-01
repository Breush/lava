#pragma once

#include <lava/sill/components/i-component.hpp>

#include <functional>
#include <glm/glm.hpp>
#include <string>

namespace lava::sill {
    class GameEntity;
}

namespace lava::sill {
    class TransformComponent final : public IComponent {
    public:
        enum ChangeReasonFlag : uint16_t {
            User = 0x0001,
            Physics = 0x0002,
            All = 0xFFFF,
        };

        using ChangeReasonFlags = uint16_t;

    public:
        TransformComponent(GameEntity& entity);
        ~TransformComponent();

        /// IComponent
        static std::string hrid() { return "transform"; }
        void update() override final;

        void translate(const glm::vec3& delta, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        glm::vec3 translation() const;
        void translation(const glm::vec3& translation, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        const glm::mat4& worldTransform() const;

        // Callbacks
        void onTranslationChanged(std::function<void(const glm::vec3&)> positionChangedCallback,
                                  ChangeReasonFlags changeReasonFlags = ChangeReasonFlag::All);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
