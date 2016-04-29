# Tetris

A tetris clone implemented in C, using SDL2 libraries for the window and
rendering. Currently in working order although the graphics are a little off.

## Install/Running

Ensure you have sdl2 and sdl2\_ttf libraries installed, as well as clang. You
could go into the `build.sh` file and replace clang with gcc, as it should
compile with either.

Then, simply run `make` to build the library, `make run` to run the game
itself. Note that you have to launch the game from the data directory at the
moment, I'll be fixing that in a later update.

## Future

I plan on redoing the build system I'm using, because honestly I don't like the
current way I have Makefiles working. Also need to add the license for DejaVu
fonts. Currently working on redoing my board management in the `board` branch.
