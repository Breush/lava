# What's next

- Change setter/getter to unique syntax
- Each lib has its own namespace
- Convert to C++17 (string_view, UTF8, filesystem, ...)

### Chamber

- **refacto**
    - Have user documentation
- **feature**
    - Macros: `$pimpl_property` and `$pimpl_attribute`

### Crater

- **feature**
    - Event: gave a way to access keyboard/mouse state (isKeyPressed)
    - Event: complex event handling (dragging, double click)
    - Window: fullscreen
    - Window::Impl: have Win32 and Wayland support

### Magma

- **refacto**
    - Use vulkan.hpp 
    - Have interfaces.hpp
    - Have user documentation
- **improvement** 
    - Engine: update main command buffer every frame
    - Engine: remove mesh animation from the engine - have timeElapsed passed (thanks to caldera)?
    - GLB: pass factors and colors (with textures)
    - Mesh: have own secondary buffers
    - Mesh: allow not to have to specify a material (defaults to RmMaterial?)
    - OrbitCamera: FOV and up-vector configurable
    - PBR: Rename MrrMaterial to RmMaterial
    - PBR: Use define to know whether there is a texture (compile shaders within the engine)
    - PBR: Remove Vertex Color attribute
- **feature**
    - Light: In engine, and forwarded to the shaders
    - Mesh: Compute tangents if not provided
    - Mesh: Transform
    - PBR: [SgMaterial](https://github.com/KhronosGroup/glTF/tree/master/extensions/Khronos/KHR_materials_pbrSpecularGlossiness)
    - PBR: Reflection cube maps
    - Reflection probes
    - Shader: Recompile on demand, removing the need to bind empty textures
    - Shader: ShaderManager to handle recompilation on the fly
    - Texture: Have the notion and a manager
- **faroff**
    - Shader: have own wrapper around glslang...
    - GLB: should that loader really be in magma? That's not rendering *per se*. (Same goes for MeshMakers) - It should be another layer
    - Think about inlining in code data (shaders mainly, but debug texture too).

### Caldera

- Everything's left!
- OrbitCameraController
- Render loop controled by this lib?

## Ashes

- **faroff**
    - A feature, an example
