# What's next

- Change setter/getter to unique syntax

## Remove SFML

### Chamber

- Rename file to lower case
- Remove comments \brief and such
- Remove Export.hpp, lava/config.hpp
- Window -> Remove everything not used
- (?) Remove lava::String and lava::Utf class
- Replace lava::Thread with std::thread/std::mutex

### Crater

- Everything's left!
- Have Win32 and Wayland support
- Rename file to lower case
- Remove comments \brief and such
- Have a way to access keyboard/mouse state (isKeyPressed)
- Window fullscreen
- Window title

### Magma

- USE VULKAN.HPP
- Material: Use define to know whether there is a texture (compile shaders within the engine)
- Mesh: have own secondary buffers

- Engine update main command buffer every frame
- Reflection probe

## Example app

- Everything's left!
