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
    - Add semaphores (fences?) to wait between each step
- **refacto**
    - Have interfaces.hpp
    - Have user documentation
    - Forward used in API symbols `namespace magma { using Event = crater::Event; }` (or completing wrap them to have no external reference in the API?)
    - Make some ImageDescriptor/BufferDescriptor?
    - Remove useless descriptor pools
    - RenderStage: A good design shouldn't need virtual functions for create infos
    - RenderEngine: Make rendering dimensions independent to window ones
    - Clean up capsule.hpp
    - Move utils functions to own folder or inside Holders's cpp file
- **improvement** 
    - Engine: rework main loop - environment -> view -> shader -> material -> mesh
    - Engine: update main command buffer every frame
    - Engine: allow to free descriptor set (and do it in mesh, etc) - see descriptor pool flags
    - Engine: remove mesh animation from the engine - have timeElapsed passed (thanks to caldera)?
    - GLB: pass factors and colors (with textures)
    - Mesh: compute tangents if not provided
    - Mesh: have own secondary buffers
    - Mesh: allow not to have to specify a material (defaults to RmMaterial?)
    - Node: Hierarchy and update transforms
    - OrbitCamera: FOV and up-vector configurable
    - PBR: albedo color factors and such
    - PBR: Use define to know whether there is a texture (compile shaders within the engine)
    - PBR: Remove Vertex Color attribute
    - PBR: what is the meaning of the 0.5 * ambientColor?
    - RenderWindow: fix window resizing
    - RmMaterial: Clarify albedo/normalMap usage
    - SphereMesh: fix poles tangents
    - SphereMesh: uv mapping
- **feature**
    - DirectionalLight
    - Light: Allow multiple lights (how many? how to optimize stationary ones?)
    - Light: Shadows
    - PBR: [SgMaterial](https://github.com/KhronosGroup/glTF/tree/master/extensions/Khronos/KHR_materials_pbrSpecularGlossiness)
    - PBR: Reflection cube maps
    - Reflection probes
    - Scene: A scene holds some render stages an is binded to x render-targets
    - Shader: Recompile on demand, removing the need to bind empty textures
    - Shader: ShaderManager to handle recompilation on the fly
    - Texture: Have the notion and a manager
- **faroff**
    - Shader: have own wrapper around glslang...
    - GLB: should that loader really be in magma? That's not rendering *per se*. (Same goes for MeshMakers) - It should be another layer
    - Think about inlining in code data (shaders mainly, but debug texture too)

## Sill

- Everything's left!
- OrbitCameraController
- Render loop controled by this lib?

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
