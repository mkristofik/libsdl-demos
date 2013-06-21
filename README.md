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
- Multiple obstacle images per terrain type, chosen randomly at map generation time.  The obstacle images are offset slightly from the center of each hex for a more irregular look.
- No islands within each region.  Every open hex in a region is guaranteed to be reachable from every other open hex.
- Pathfinding using [A\*](http://en.wikipedia.org/wiki/A*) and Dijkstra's Algorithm.  It's fast enough to render paths in [real time](http://www.youtube.com/watch?v=2PPOoeHhWMw).
- Hierarchical pathfinding enables real-time path generation across multiple regions, or even the entire map.  I first compute a top-level set of hops between regions, then run the pathfinder again to compute the final path inside each region.  [See a demo](http://www.youtube.com/watch?v=r2fWScHL5DQ).

![screenshot](https://raw.github.com/mkristofik/libsdl-demos/master/random_screen.jpg)

## Pathfinder

[Pathfinder](https://github.com/mkristofik/libsdl-demos/blob/master/src/Pathfinder.h) is a class within the Random Map Generator project, but there are no dependencies that would prevent it from being compiled separately.  It aims to be a generic C++11 implementation of the A\* algorithm.  To avoid tying it to a particular graph or tile structure, all inputs are provided by lambda functions.  If the nodes of your map can be represented by integers, you can use this class.  Users must answer up to four key questions:

- What steps can you take from a given node?
- What is the cost for going from node A to node B?  *(Note: does not support negative edge weights)*
- Can you make a lower-bound estimate of a node's distance from the goal?
- What does the goal look like?

I think that last question is the most interesting.  Sometimes you don't know where the goal node is.  There might even be more than one.  A user might ask, "find me shortest path to the nearest water hex."  Any water hex will do.  A nice property of A\*/Dijkstra's is stopping once it reaches *any* goal node, knowing that it has taken the shortest path to get there.

## Jukebox

This little app does what you'd expect: it plays music.  Any game is probably going to want background music, so it would be useful to know how to play it.  To use it, create a `music` subfolder within the project and fill it with music files.

**Things I learned:**

- Playback of .mp3 and .ogg files is trivial with the SDL\_Mixer library.
- There's no API to get the current track position, so that's why you don't see a slider bar.  I presume this is because games don't typically need that feature.
- Handling pushbuttons in the user interface.
- Word wrapped text for long file names.

![screenshot, paused](https://raw.github.com/mkristofik/libsdl-demos/master/jukebox_screen_paused.jpg)
![screenshot, playing](https://raw.github.com/mkristofik/libsdl-demos/master/jukebox_screen_playing.jpg)

## Sprite Animation

This demo places a few sprites within a hex grid.  It aims to experiment with the unit animations used in Battle for Wesnoth.  So far we have an archer firing an arrow and a knight swinging a sword.  [See a video](http://youtu.be/Up7vQKw3Nvg).

**Technical details:**

- Up to this point I've always been drawing full images.  This demo adds the ability to draw each animation frame from a sprite sheet.
- All of the sprites face to the right.  I flip the enemy graphics in code at runtime.
- After doing the jukebox app, it was simple to add a background music track and some sound effects.
- I've implemented the Battle for Wesnoth [team coloring](http://wiki.wesnoth.org/Team_Color_Shifting) algorithm to adjust the colors of each sprite at load time.

![screenshot](https://raw.github.com/mkristofik/libsdl-demos/master/animate_screen.jpg)

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
[SDL\_mixer 1.2.12](http://www.libsdl.org/projects/SDL_mixer/)  
[DejaVuSans](http://dejavu-fonts.org/wiki/Main_Page) font (drop the .ttf file
in the top-level directory)

### Special Thanks To
[Battle for Wesnoth](www.wesnoth.org) for all of the art assets.  They can
draw, and I can't.

### License

Copyright 2012-2013 by Michael Kristofik  
The license for code and art assets is described in the file COPYING.txt.
