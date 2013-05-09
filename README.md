# LibSDL Tech Demos
**Michael Kristofik** ([kristo605@gmail.com](mailto:kristo605@gmail.com))

This project charts my progress as I learn SDL and become familiar with C++11
techniques.  I'm sharing my work because code like this wants to be free.  It's
also my first foray into cmake without a Build Guy to do all the setup for me.

## Random Map Generator

This was inspired by Amit Patel's [blog
post](http://www-cs-students.stanford.edu/~amitp/game-programming/polygon-map-generation/)
about Voronoi Diagrams being used for random map generation.  I'm surprised I
never learned about them in school because they're quite useful.  I wanted to
see if this idea could be scaled down to a hexagonal tile map.  You can see the
results in the screenshot below.

**Technical details:**

- Random regions generated using a [Voronoi Diagram](http://en.wikipedia.org/wiki/Voronoi_diagrams) and [Lloyd's Algorithm](http://en.wikipedia.org/wiki/Lloyd%27s_algorithm).
- Six possible terrain types, assigned by a [greedy graph coloring](http://en.wikipedia.org/wiki/Greedy_coloring) algorithm.
- Edge transitions between tiles of different terrain.
- Map scrolling by hovering the mouse near a map edge, or click-and-drag within the minimap ([video](http://youtu.be/foWstanCoUw)).
- Obstacles (trees, mountains, etc.) assigned by a simple [value noise](http://en.wikipedia.org/wiki/Value_noise) algorithm.  Any hex above a threshold gets an obstacle.
- Multiple obstacle images per terrain type, chosen randomly at map generation time.
- No islands within each region.  Every open hex in a region is guaranteed to be reachable from every other open hex.
- Pathfinding using [A\*](http://en.wikipedia.org/wiki/A*) and Dijkstra's Algorithm.

![screenshot](https://raw.github.com/mkristofik/libsdl-demos/master/random_screen.jpg)

## Pathfinder

[Pathfinder](https://github.com/mkristofik/libsdl-demos/blob/master/src/Pathfinder.h) is a class within the Random Map Generator project, but there are no dependencies that would prevent it from being compiled separately.  It aims to be a generic C++11 implementation of the A\* algorithm.  To avoid tying it to a particular graph or tile structure, all inputs are provided by lambda functions.  If the nodes of your map can be represented by integers, you can use this class.  Users must answer up to four key questions:

- What steps can you take from a given node?
- What is the cost for going from node A to node B?  *(Note: does not support negative edge weights)*
- Can you make a lower-bound estimate of a node's distance from the goal?
- What does the goal look like?

I think that last question is the most interesting.  Sometimes you don't know where the goal node is.  There might even be more than one.  A user might ask, "find me shortest path to the nearest water hex."  Any water hex will do.  A nice property of A\*/Dijkstra's is stopping once it reaches *any* goal node, knowing that it has taken the shortest path to get there.

## Hello World

No learning experience is complete without a Hello World program, so here it
is.  I create a window, load an image, and print some text with various levels
of anti-aliasing enabled.

![screenshot](https://raw.github.com/mkristofik/libsdl-demos/master/hello_screen.jpg)

----

### Requirements

[SDL 1.2.15](http://www.libsdl.org/)  
[SDL\_image 1.2.12](http://www.libsdl.org/projects/SDL_image/)  
[SDL\_ttf 2.0.11](http://www.libsdl.org/projects/SDL_ttf/)  
[DejaVuSans](http://dejavu-fonts.org/wiki/Main_Page) font (drop the .ttf file
in the top-level directory)

### Special Thanks To
[Battle for Wesnoth](www.wesnoth.org) for all of the art assets.  They can
draw, and I can't.

### License

Copyright 2012-2013 by Michael Kristofik  
The license for code and art assets is described in the file COPYING.txt.
