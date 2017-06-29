# What's next

- Change setter/getter to unique syntax

### Chamber

- Have user documentation

### Crater

- Everything's left!
- Have Win32 and Wayland support
- Rename files to lower case
- Remove comments \brief and such
- Window -> Remove everything not used
- Have a way to access keyboard/mouse state (isKeyPressed)
- Window fullscreen
- Window title
- Use pimpl conventions (no priv namespace)
- Clean warnings
- Have complex event handling (dragging, double click)

### Magma

- USE VULKAN.HPP 
- Material: Use define to know whether there is a texture (compile shaders within the engine)
- Mesh: have own secondary buffers
- Shader: have own wrapper around glslang... something like shaderc but way better
- Mesh: Compute tangents if not provided
- Material: Rename to RmMaterial
- PBR: Reflection cube maps
- PBR: [SgMaterial](https://github.com/KhronosGroup/glTF/tree/master/extensions/Khronos/KHR_materials_pbrSpecularGlossiness)
- Reflection probes
- Engine update main command buffer every frame
- OrbitCamera: controls in triangle.cpp should be a new lib (caldera?)
- Have interfaces.hpp

## Example app

- Everything's left!
