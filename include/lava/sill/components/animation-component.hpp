#pragma once

#include <lava/sill/components/i-component.hpp>

#include <lava/core/transform.hpp>
#include <lava/sill/animation-flags.hpp>
#include <lava/magma/uniform.hpp>
#include <unordered_map>
#include <variant>

namespace lava::magma {
    class Material;
}

namespace lava::sill {
    /**
     * Allows interpolation between two states.
     */
    class AnimationComponent final : public IComponent {
    public:
        AnimationComponent(Entity& entity);

        // IComponent
        static std::string hrid() { return "animation"; }
        void update(float dt) final;

        /**
         * Controls animation over the specified flags.
         */
        void start(AnimationFlags flags, float time, bool autoStop = true);
        void start(AnimationFlags flags, magma::Material& material, const std::string& uniformName, float time, bool autoStop = true);
        void stop(AnimationFlags flags);
        void target(AnimationFlag flag, const lava::Transform& target);
        void target(AnimationFlag flag, magma::Material& material, const std::string& uniformName, float target);
        void target(AnimationFlag flag, magma::Material& material, const std::string& uniformName, const glm::vec4& target);

    protected:
        struct AnimationInfo {
            bool autoStop = false;
            bool needUpdate = true; // When timeSpent < totalTime or if target or time has changed since last update.
            float timeSpent = 0.f;
            float totalTime = 0.f;
            std::variant<lava::Transform, lava::magma::UniformFallback> startValue;
            std::variant<lava::Transform, lava::magma::UniformFallback> targetValue;

            // For AnimationFlag::MaterialUniform
            magma::Material* material;
            std::string uniformName;
            lava::magma::UniformType uniformType;
        };

    protected:
        AnimationInfo* findAnimationInfo(AnimationFlag flag);
        AnimationInfo* findAnimationInfo(AnimationFlag flag, magma::Material& material, const std::string& uniformName);
        AnimationInfo& findOrCreateAnimationInfo(AnimationFlag flag);
        AnimationInfo& findOrCreateAnimationInfo(AnimationFlag flag, magma::Material& material, const std::string& uniformName);
        void updateInterpolation(AnimationFlag flag, AnimationInfo& animationInfo);

    private:
        std::unordered_map<AnimationFlag, std::vector<AnimationInfo>> m_animationsInfos;
    };
}
