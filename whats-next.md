# What's next

## Cross-projects

- Split into different git projects
- Convert to C++17 (string_view, UTF8, filesystem, ...)
- Removed useless folder when no namespace (and clean useless namespaces)

## Chamber

- **refacto**
    - Have user documentation
- **feature**
    - CallStack: allow to compile with clang, make a dummy implementation

## Crater

- **refacto**
    - See if that crater::input::* namespace is a bit too much (remove input?)
    - Change pollEvent() to return a reference_wrapper instead 
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
    - Engine: allow to free descriptor set (and do it in mesh, etc) - see descriptor pool flags
    - GLB: pass factors and colors (with textures)
    - Mesh: have own secondary buffers
    - OrbitCamera: FOV and up-vector configurable
    - Viewport: specify region to be shown
- **feature**
    - AmbientLight
    - DirectionalLight
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
    - PBR: Convert to specular-glossiness only [SgMaterial](https://github.com/KhronosGroup/glTF/tree/master/extensions/Khronos/KHR_materials_pbrSpecularGlossiness)
    - PBR: Reflection cube maps
    - Reflection probes
    - Shader: Recompile on demand, removing the need to bind empty textures
    - Texture: Have the notion and a manager
- **faroff**
    - Shader: have own wrapper around glslang...
    - Think about inlining in code data (shaders mainly, but debug texture too)

## Sill

- **refacto**
    - magma should be internal use and not be seen in public API includes
- **feature**
    - Animation
    - Main loop (elapsed time)
    - Materials introspection (-- or own shading language)
    - Mesh: compute tangents if not provided
    - Mesh: compute normals if not provided
    - Node: hierarchy of GameObjects
    - OrbitCameraController
    - RenderWindow: Take from magma - so that it does not depend on crater anymore
    - RmMaterial: albedo color
    - SphereMesh: uv mapping

## Dike

- Everything's left!
- Wrap BulletPL into a nice API

## Ashes

- **faroff**
    - A feature, an example
