# lava - layered api for vulkan awesomeness

## Project's composition

The **lava** project is in fact a bundle of multiple projects.

<table>
    <tr>
        <td colspan="4">
            <strong>chamber</strong><br/>
            Cross-projects core tools
        </td>
    </tr>
    <tr>
        <td colspan="2">
            <strong>crater</strong><br/>
            Window and events
        </td>
        <td rowspan="2">
            <strong>dike</strong><br/>
            Physics
        </td>
        <td rowspan="2">
            <strong>flow</strong><br/>
            Sound
        </td>
    </tr>
    <tr>
        <td colspan="2">
            <strong>magma</strong><br/>
            Renderer
        </td>
    </tr>
    <tr>
        <td colspan="4">
            <strong>sill</strong><br/>
            Behaviour and engine interface
        </td>
    </tr>
    <tr>
        <td colspan="2">
            <strong>ashes</strong><br/>
            Example applications
        </td>
        <td colspan="2">
            <strong>caldera</strong><br/>
            UI engine
        </td>
    </tr>
</table>

The core principles of **lava** are:
- Be *up-to-date* with C++ standard (currently set to C++14) to make your code-life easier ;
- Keep your *APIs clean* (each project has a public interface with no more information than needed) ;
- *Decouple* as much as you can, keeping different concepts encapsulated.

## Contributing guide

**lava** uses *Premake* as build configuration system.

- download [Premake v5](https://premake.github.io/download.html#v5) and install it from your platform ;
- run *Premake5* at project's root folder
	- `premake5 gmake` for *Linux* makefiles (or MinGW/CygWin ones) ;
	- `premake5 vs2017` for *Microsoft Visual Studio* solution (currently untested) ;
	- `premake5 xcode3` for *Apple* Xcode solution (currenly untested) ;
	- or check [this Premake5 wiki page](https://github.com/premake/premake-core/wiki/Using-Premake) for other solutions ;
- your solution files have been generated to project's root folder.

### Building

In order to select the configuration (debug or release):
- `make config=release` for gmake.

### Generating documentation

- Have [doxygen](www.doxygen.org/) installed on your system ;
- `cd doc && doxygen doxygen-config.xml` to generate documentation. 

## Dependencies

Everything **lava** needs is downloaded during *Premake* phase to `external/`. 

Current awesome dependencies are:
- [Nlohman's JSON](https://github.com/nlohmann/json)
- [Nothings.org STB Fonts](https://nothings.org/stb/font/)
- [OpenGL Mathematics](http://glm.g-truc.net/)
- [VulkanSDK](https://vulkan.lunarg.com/)

__NOTE__ The one guideline concerning dependencies is to not include within this repository any external source,
keeping the project light-weight and up-to-date. The one drawback is that compiling this repository in the future
could be impossible because of removed or changed projects. If so, a new repository should be created containing
the no-longer-available sources of concerned project.
