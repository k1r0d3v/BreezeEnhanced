# SolidBreeze

## Overview

Solid is a fork of BreezeEnhanced decoration with the following changes:
 - Code cleaning and refactor
 - Title bar background color deduction of client window
 - Text color adaptable to title bar background color
 - Improved button feel and colorize, with easy theme addiction
 - Easy creation of new button types and solid themes
 - Hide title bar on maximize 
 - Support for latte-dock(modified) colorize in base title bar color for active and maximized windows

Please note that Solid is not related to the Breeze widget style. In fact, it is made to match personal preferences.

## Installation

The version number in the file [NEWS](NEWS) shows the main version of KWin that is required for the compilation. *Compilation should not be done against other versions of KWin!*

Open a terminal inside the source directory and do:
```sh
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DKDE_INSTALL_LIBDIR=lib -DBUILD_TESTING=OFF -DKDE_INSTALL_USE_QT_SYS_PATHS=ON
make
sudo make install
```
After the installation, restart KWin by logging out and in. Then, Solid will appear in *System Settings &rarr; Application Style &rarr; Window Decorations*.

## TODO
 - Improve background color detection
   - With shade support
 - Adapt left, right and bottom borders to expand the client window
 - Better animated icons
 - Menu drawing over title bar
 - Configurable options with UI, latte-dock colorize mode, hide title bar, etc...

## Notes
 - Some features has been removed, like the grip

## Credits:
Solid was started from [BreezeEnhanced](https://github.com/tsujan/BreezeEnhanced).
BreezeEnhanced was started from [BreezeBlurred](https://github.com/alex47/BreezeBlurred), a former fork of Breeze with title-bar translucency and blurring.
Needless to say, the main work behind BreezeEnhanced is the Breeze KWin decoration itself, which can be downloaded from <https://download.kde.org/stable/plasma/>.
