# Psi Engine
### Goals:
- Build a multithreaded game engine conforming in its general architecture to https://software.intel.com/en-us/articles/designing-the-framework-of-a-parallel-game-engine/

### Components:
###### Sol Library
A collection of (mostly abstract) classes and methods that can be reused in various games.

###### Generic Shooter Game
A generic shooter game. Doesn't render anything yet. [WIP]

### Dependencies:
- CMake >= 3.3.0
- Boost >= 1.60.0
- GLFW >= 3.1.2
- Intel TBB >= 4.4

### Building:
###### UNIX:
	mkdir build
	cd build
	cmake ..
	make
Only tested on Clang 3.7.1. Binary GSG will be stored in build/generic_shooter_game.

###### Mac OS X:
Not yet tested.

###### Windows:
Not yet tested.

### Libraries used:
- [Boost](http://www.boost.org/)

- [GLFW](http://www.glfw.org/)

- [Intel TBB](https://www.threadingbuildingblocks.org/)

- [glLoadGen](https://bitbucket.org/alfonse/glloadgen/wiki/Home/)
