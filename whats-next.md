# What's next

- Change setter/getter to unique syntax

## Remove SFML

### Chamber

- Rename file to lower case
- Remove comments \brief and such
- Replace `err()` with `Logger` class (with categories)
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

- Mesh have own secondary buffers
- Engine update main command buffer every frame

## Example app

- Everything's left!
