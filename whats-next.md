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
    - Window: fullscreen
    - Window::Impl: have Wayland support
    - Window::Impl: DWM - complete all events (scroll/keys)

## Magma

- **bug**
    - Artifacts for opaque meshes with a lot of overlapping
        - Try having a RT as the GBufferHeader,
          so that depth resolution is managed as expected by the GC.
        - Have a new opaque epiphany to resolve the header into a RT.
          This can be the RT used in transparent epiphany later. 
        - Having that done, we can reset the GBufferHeader and counter,
          so that transparent objects have a full GBufferList to fill.
- **refacto**
    - Remove useless descriptor pools
    - Replace useless const vk::Whatever& by vk::Whatever
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
    - Light Linked List
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

- **feature**
    - Animation
    - Debounce system for window resizes
    - ColliderComponent: have a PhysicsComponent (holding if static)
    - ColliderComponent: allow multiple times the same component
    - GameEngine: remove the camera controls of `handleEvent`
    - InputManager: handle axes 
    - Materials introspection (or own shading language)
    - Mesh: compute tangents if not provided
    - Mesh: compute normals if not provided
    - Node: hierarchy of GameObjects
    - OrbitCameraController
    - RmMaterial: albedo color
    - SphereMesh: uv mapping

## Ashes

- **faroff**
    - A feature, an example
