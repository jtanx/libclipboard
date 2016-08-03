libclipboard [![Build Status](https://travis-ci.org/jtanx/libclipboard.png)](https://travis-ci.org/jtanx/libclipboard)
=========

A cross-platform clipboard library.

## Currently supported actions
* Checking clipboard data ownership
* Clearing clipboard
* Retrieving/setting text (UTF-8)

## Supported platforms
* Windows
* Linux (X11)

## Platforms to be supported in the future
* OS X (Cocoa)
* Linux (Wayland) (maybe)

## Requirements
* cmake
* gtest & g++ >= 4.7 (for unit testing)
* libxcb-dev (for Linux/X11)
* pthreads (for Linux/X11)
