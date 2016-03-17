# Psi Engine [WIP]
### Goals:
- Build a multithreaded game engine conforming in its general architecture to https://software.intel.com/en-us/articles/designing-the-framework-of-a-parallel-game-engine/

### Components:
###### Psi Engine
The engine.
TODO describe engine here

###### Generic Shooter Game
A sample usage of the engine.

### Dependencies:
- [CMake](https://cmake.org/) >= 3.3.0
- [Boost](http://www.boost.org/) >= 1.60.0
- [GLFW](http://www.glfw.org/) >= 3.1.2
- [Intel TBB](https://www.threadingbuildingblocks.org/) >= 4.4
- [glLoadGen](https://bitbucket.org/alfonse/glloadgen/wiki/Home/) Included

### Building:
###### Linux:
	mkdir build
	cd build
	cmake ..
	make
Only tested on Clang 3.7.1. Binary will be stored in build/generic_shooter_game.

###### Mac OS X:
Not yet tested.

###### Windows:
Not yet tested.
