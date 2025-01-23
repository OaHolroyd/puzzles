# TUI Puzzles
A collection of small puzzles to play on the command line. The TUIs are rendered with `ncursesw`. Since some implementations bundle `ncurses` and `ncursesw` together in one library, `pkg-config` is used to set the correct linker flags.

## Getting Started
Clone the repository and `cd` into the top-level directory.
```sh
make
make check
```
If `make` runs successfully, the puzzle executables will be in the `puzzles/` directory.