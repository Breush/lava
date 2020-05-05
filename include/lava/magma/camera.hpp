#pragma once

#include <glm/glm.hpp>
#include <lava/core/extent.hpp>
#include <lava/core/macros/aft.hpp>
#include <lava/core/transform.hpp>
#include <lava/magma/frustum.hpp>
#include <lava/magma/polygon-mode.hpp>
#include <lava/magma/render-image.hpp>
#include <lava/magma/ubos.hpp>

namespace lava::magma {
    class CameraAft;
    class Scene;
}

namespace lava::magma {
    /**
     * A generic camera.
     *
     * To get user-friendly controls,
     * create an `OrbitCameraController` or such.
     *
     * ```c++
     * auto& camera = scene.make<Camera>({600, 400});
     * OrbitCameraController cameraController(camera);
     * ```
     */
    class Camera {
    public:
        Camera(Scene& scene, Extent2d extent);
        ~Camera();

        $aft_class(Camera);

        Scene& scene() { return m_scene; }
        const Scene& scene() const { return m_scene; }

        /**
         * @name Rendering
         */
        /// @{
        /// Render images of the camera. Bindable to a view.
        RenderImage renderImage() const;
        RenderImage depthRenderImage() const;

        /// How meshes should be renderered within this camera.
        PolygonMode polygonMode() const { return m_polygonMode; }
        void polygonMode(PolygonMode polygonMode);

        /// Its frustum, automatically updated.
        const Frustum& frustum() const { return m_frustum; }

        /// A frustum, based on a view rectangle from [-1, -1] (top left) to [1, 1] (bottom right).
        Frustum frustum(const glm::vec2& topLeft, const glm::vec2& bottomRight) const;

        /// Whether frustum culling is enabled.
        bool frustumCullingEnabled() const { return m_frustumCullingEnabled; }
        void frustumCullingEnabled(bool frustumCullingEnabled) { m_frustumCullingEnabled = frustumCullingEnabled; }

        /**
         * Whether the camera is used to render in a VR render target.
         * When true, meshes tagged as non-vrRenderable will not be displayed.
         *
         * Binding a CameraController might update this value.
         */
        bool vrAimed() const { return m_vrAimed; }
        void vrAimed(bool vrAimed) { m_vrAimed = vrAimed; }
        /// @}

        /**
         * @name Init-time configuration
         *
         * Changing any of these values required to call updateCamera
         * on the CameraController that has been binded to this camera.
         *
         * These values might or might not be used to generate the projection transform
         * within the CameraController. For instance, VrEyeCameraController
         * does not use the FOV because it is constrained by the VR engine.
         */
        /// @{
        /// Dimensions of the render area.
        Extent2d extent() const { return m_extent; }
        void extent(Extent2d extent);

        /// Vertical field of view (FOV) expressed in radians.
        float fovY() const { return m_fovY; }
        void fovY(float fovY) { m_fovY = fovY; }

        /// Minimum distance of rendering.
        float nearClip() const { return m_nearClip; }
        void nearClip(float nearClip) { m_nearClip = nearClip; }

        /// Maximum distance of rendering.
        float farClip() const { return m_farClip; }
        void farClip(float farClip) { m_farClip = farClip; }
        /// @}

        /**
         * @name Transforms
         */
        /// @{
        /// Translation, rotation of the camera within world-space.
        const lava::Transform& viewTransform() const { return m_viewTransform; }
        void viewTransform(const lava::Transform& viewTransform);

        // @todo Would be nice to have these deprecated!
        const glm::mat4& viewMatrix() const { return m_viewMatrix; }
        const glm::mat4& viewMatrixInverse() const { return m_viewMatrixInverse; }

        /// Projection matrix to get Vulkan's Normalized Device Coordinates.
        const glm::mat4& projectionMatrix() const { return m_projectionMatrix; }
        const glm::mat4& projectionMatrixInverse() const { return m_projectionMatrixInverse; }
        void projectionMatrix(const glm::mat4& projectionMatrix);

        /// Pre-computed inverse(projectionMatrix * viewTransform).
        const glm::mat4& viewProjectionMatrixInverse() const { return m_viewProjectionMatrixInverse; }
        /// @}

        /**
         * @name Shader data
         */
        /// @{
        const CameraUbo& ubo() const { return m_ubo; }
        /// @}

    protected:
        void updateFrustum();

    private:
        // ----- References
        Scene& m_scene;

        // ----- Shader data
        CameraUbo m_ubo;

        // ----- Rendering
        PolygonMode m_polygonMode = PolygonMode::Fill;
        Frustum m_frustum;
        bool m_frustumCullingEnabled = true;
        bool m_vrAimed = false;

        // ----- Init-time configuration
        Extent2d m_extent = {800u, 600u};
        float m_fovY = glm::radians(45.f);
        float m_nearClip = 0.1f;
        float m_farClip = 1000.f;

        // ----- Transforms
        lava::Transform m_viewTransform;
        glm::mat4 m_viewMatrix = glm::mat4(1.f);
        glm::mat4 m_projectionMatrix = glm::mat4(1.f);
        glm::mat4 m_viewMatrixInverse = glm::mat4(1.f);
        glm::mat4 m_projectionMatrixInverse = glm::mat4(1.f);
        glm::mat4 m_viewProjectionMatrixInverse = glm::mat4(1.f);
    };
}
