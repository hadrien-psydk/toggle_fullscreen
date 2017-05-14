# toggle_fullscreen

This programs allows toggling a X11 window to fullscreen (or more exactly, to a border-less maximized window).

### Usage
Run the program in a terminal, with the window name (the window title text) as the argument. The program waits for the specified window to open, make it fullscreen and leave.

### How to build on Ubuntu

Install build packages:
`sudo apt install build-essential`
`sudo apt install libx11-dev`

Get the sources, open a terminal, go into the main directory and run `make`.

Then run the command `./toggle_fullscreen WINDOW_NAME` where WINDOW_NAME is the title text of the target window.

### Install/Uninstall

To add the program to your system, you can call:
`sudo make install`
To uninstall it:
`sudo make uninstall`

### Origin
This utility was originally coded as a workaround to play Trine2 in fullscreen. So here are the instructions:

* in a terminal, run `toggle_fullscreen Trine2`
* start Trine 2 in windowed mode, with the same resolution than your desktop
