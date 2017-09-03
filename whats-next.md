# What's next

## Cross-projects

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

- **up**
    - Light Linked List Renderer
        - Fill linked list
            - Software depth test
            - Min/max depths
            - Allocate LLL fragment
        - Lighting G-buffer (Epiphany)
            - Access LLL
            - PBR Epiphany (keep for last step)
        - ---Custom Materials---
        - Alpha
- **bug**
    - Fix top-view camera
- **refacto**
    - Remove references to crater from the public API
    - Remove useless descriptor pools
    - RenderStage: A good design shouldn't need virtual functions for create infos
    - Replace useless const vk::Whatever& by vk::Whatever
    - Split magma's RenderWindow so that magma does not depend on crater anymore (make the other a part of sill)
- **improvement** 
    - Have user documentation
    - Engine: rework main loop - environment -> view -> shader -> material -> mesh
    - Engine: allow to free descriptor set (and do it in mesh, etc) - see descriptor pool flags
    - GLB: pass factors and colors (with textures)
    - Mesh: have own secondary buffers
    - Node: Hierarchy and update transforms
    - OrbitCamera: FOV and up-vector configurable
    - PBR: albedo color factors and such
    - PBR: Use define to know whether there is a texture (compile shaders within the engine)
    - PBR: what is the meaning of the 0.5 * ambientColor?
    - RmMaterial: Clarify albedo/normalMap usage
    - Viewport: specify region to be shown
- **feature**
    - DirectionalLight
    - Light: Allow multiple lights (how many? how to optimize stationary ones?)
    - Light: Shadows
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
    - crater should be internal use and not be seen in public API includes
- **feature**
    - Animation
    - GLB: take loader from magma as that's not rendering *per se*. (Same goes for MeshMakers)
    - Main loop (elapsed time)
    - Mesh: compute tangents if not provided
    - Mesh: compute normals if not provided
    - OrbitCameraController
    - RenderWindow: Take from magma - so that it does not depend on crater anymore
    - SphereMesh: Take from magma 
    - SphereMesh: uv mapping

## Dike

- Everything's left!
- Wrap BulletPL into a nice API

## Ashes

- **bug**
    - Track artefact in corset-back (magma-related)
- **feature**
    - Light: improve controls
- **faroff**
    - A feature, an example
