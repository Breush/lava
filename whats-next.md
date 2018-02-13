# What's next

## Cross-projects

- Split into different git projects
- Convert to C++17 (string_view, UTF8, ...)
- Removed useless folder when no namespace (and clean useless namespaces)

## Chamber

- **refacto**
    - Have user documentation

## Crater

- **feature**
    - Event: gave a way to access keyboard/mouse state (isKeyPressed)
    - Event: complex event handling (dragging, double click)
    - Window: fullscreen
    - Window::Impl: have Wayland support
    - Window::Impl: DWM - complete all events

## Magma

- **bug**
    - Fix top-view camera
    - Artifacts for opaque meshes with a lot of overlapping
- **refacto**
    - Remove references to crater from the public API
    - Remove useless descriptor pools
    - Replace useless const vk::Whatever& by vk::Whatever
    - Split magma's RenderWindow so that magma does not depend on crater anymore (make the other a part of sill)
- **improvement** 
    - Have user documentation
    - Engine: rework main loop - environment -> view -> shader -> material -> mesh
    - GLB: pass factors and colors (with textures)
    - Mesh: have own secondary buffers
    - OrbitCamera: FOV and up-vector configurable
    - Viewport: specify region to be shown
- **feature**
    - AmbientLight
    - DirectionalLight
    - ForwardRenderer
    - VR integration
    - Light: Shadows
    - Light Linked List?
        - Fill linked list
            - Software depth test
            - Min/max depths
            - Allocate LLL fragment
        - Lighting G-buffer (Epiphany)
            - Access LLL
            - PBR Epiphany (keep for last step)
        - Alpha
    - PBR: Reflection cube maps
    - Reflection probes
    - Texture: Have a manager?
    - Wireframe
- **faroff**
    - Think about inlining in code data (shaders mainly, but debug texture too)

## Dike

- **feature**
    - Check deletion of everything in Colliders
    - Have custom MotionState
    - Other collision shapes

## Sill

- **refacto**
    - magma should be internal use and not be seen in public API includes
- **feature**
    - Animation
    - InputManager: handle axes 
    - Materials introspection (-- or own shading language)
    - Mesh: compute tangents if not provided
    - Mesh: compute normals if not provided
    - Node: hierarchy of GameObjects
    - OrbitCameraController
    - RenderWindow: Take from magma - so that it does not depend on crater anymore
    - RmMaterial: albedo color
    - SphereMesh: uv mapping

## Ashes

- **faroff**
    - A feature, an example
