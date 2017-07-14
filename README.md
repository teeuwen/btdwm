# btdwm - Bastiaan Teeuwen's dynamic window manager
btdwm is a fork of dwm-xcb which again is a fork of dwm, an extremely fast, small, and dynamic window manager for X (see LICENSE file).
btdwm contains a number of (in my eyes) enhancements over dwm and dwm-xcb.

Changes/enhancements over dwm:

	* Per tag layouts
	* The status area has been removed
	* True transparency support
	* Probably more, I should've put this on version control earlier on...

Changes/enhancements over dwm-xcb:

	* (Almost all) drawing is done using Cairo instead of XCB
	* Proper mouse cursors
	* Text drawing is done using Pango instead of XCB
	* Xinerama support
	* Various bug-fixes
	* Probably more, have a look for yourself


## Requirements

In order to build btdwm you need the Xlib, xcb, cairo pango header files.


## Installation

Enter the following command to build and install btdwm (if necessary as root):

    make install

If you are going to use the default bluegray color scheme it is highly
recommended to also install the bluegray files shipped in the dextra package.


## Running btdwm

Add the following line to your .xinitrc to start btdwm using startx:

    exec /opt/btdwm/bin/btdwm

In order to connect btdwm to a specific display, make sure that
the DISPLAY environment variable is set correctly, e.g.:

    DISPLAY=host:1 exec /opt/btdwm/bin/btdwm


## Configuration

The configuration of btdwm is done by creating a custom config.h
and (re)compiling the source code.
