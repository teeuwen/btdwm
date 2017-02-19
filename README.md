# btdwm - Bastiaan Teeuwen's dynamic window manager
btdwm is a fork of dwm-xcb which again is a fork of dwm, an extremely fast, small, and dynamic window manager for X (see LICENSE file).
btdwm contains a number of (in my eyes) enhancements over dwm and dwm-xcb.

Changes/enhancements over dwm:

	* Per tag layout
	* The status area has been removed
	* libnotify notifications to display the active client
	* Probably more, I should've put this on version control earlier on...

Changes/enhancements over dwm-xcb:

	* (Almost all) drawing is done using Cairo instead of XCB
	* Proper mouse cursors
	* Text drawing is done using Pango instead of XCB
	* Xinerama support
	* Various bug-fixes
	* Probably more, have a look for yourself


## Requirements
In order to build dwm you need the Xlib header files.


## Installation
Edit config.mk to match your local setup (dwm is installed into
the /usr/local namespace by default).

Afterwards enter the following command to build and install dwm (if
necessary as root):

    make clean install

If you are going to use the default bluegray color scheme it is highly
recommended to also install the bluegray files shipped in the dextra package.


## Running dwm
Add the following line to your .xinitrc to start dwm using startx:

    exec dwm

In order to connect dwm to a specific display, make sure that
the DISPLAY environment variable is set correctly, e.g.:

    DISPLAY=foo.bar:1 exec dwm

(This will start dwm on display :1 of the host foo.bar.)

In order to display status info in the bar, you can do something
like this in your .xinitrc:

    while true
    do
        xsetroot -name "`date` `uptime | sed 's/.*,//'`"
        sleep 1
    done &
    exec dwm


## Configuration
The configuration of dwm is done by creating a custom config.h
and (re)compiling the source code.
