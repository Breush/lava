## Project's composition

| Layer | Project | Description                                     |
| :---: | ---     | ---                                             |
| 0     | chamber | Cross-projects core tools like String.          |
| 1     | crater  | Cross-platforms windows and event interface.    |
| 2     | magma   | Vulkan-based rendering engine.                  |

... and there're more to come!

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
- `doxygen doc/doxygen-config.xml` to generate documentation. 

## Dependencies

Everthing **lava** needs is downloaded during *Premake* phase to `external/`. 

Current awesome dependencies are:
- [VulkanSDK](https://vulkan.lunarg.com/)
- [OpenGL Mathematics](http://glm.g-truc.net/)
- [Nothings.org STB Fonts](https://nothings.org/stb/font/)

__NOTE__ The one guideline concerning dependencies is to not include within this repository any external source, keeping the project light-weight and up-to-date. The one drawback is that compiling this repository in the future could be impossible because of removed or changed projects. If so, a new repository should be created containing the no-longer-available sources of concerned project.
