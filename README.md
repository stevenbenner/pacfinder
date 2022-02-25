# PacFinder

**Repository & package explorer for Arch Linux.**

PacFinder is a GTK 3 desktop application for browsing packages installed on your
[Arch Linux][archlinux] system as well as exploring packages in the Arch Linux
official package repositories. Should be compatible with any Linux distribution
that uses pacman/libalpm for package management (e.g. [Manjaro][manjaro]).

This software is licensed under the [Apache License, Version 2.0][license].

[archlinux]: https://archlinux.org/
[manjaro]: https://manjaro.org/
[license]: COPYING

![PacFinder Screenshot](https://stevenbenner.com/misc/pacfinder-screenshot.png)

## Features

 * Comprehensive list of all packages known to local pacman
 * View & filter packages based on installation status
    * Installed
    * Not installed
    * Explicitly installed
    * Installed as a dependency
    * Installed as an optional dependency
    * Orphan packages, not required by anything
 * Explore packages contained in Arch package repositories and groups
 * Show installed "foreign" packages (i.e. manually installed or AUR)
 * Search packages by name
 * Display and navigate package dependency relationships
 * GTK desktop application, designed to obey user themes

## Installation

### Requirements

In addition to the [base-devel][base-devel] group of packages, you need to have
the following packages installed on your system to compile and run this program:

 * glib2 >= 2.56
 * gtk3 >= 3.22

[base-devel]: https://archlinux.org/groups/x86_64/base-devel/

### Install from tarball

Download the latest distribution tarball from the [releases page][releases] for
this project on GitHub. Extract the contents of that file to some temporary
directory, and then run the following commands in that directory to install the
software:

```shell
./configure
make
sudo make install
```

You will now find the PacFinder application in your software list, or you can
start it from the command line by running `pacfinder`.

[releases]: https://github.com/stevenbenner/pacfinder/releases

## Building from source

If you intend to do development work or make other changes to the project then
you can follow these instructions. If you just want to use the program then you
should instead follow the installation instructions above.

### Clone git repo

Clone the git repository to your local system:

```shell
git clone https://github.com/stevenbenner/pacfinder.git
cd pacfinder
```

### Compiling

Run the following commands to build this project from the source code in the git
repository and produce an executable suitable for development and debugging:

```shell
./autogen.sh
./configure CFLAGS="-ggdb3 -O0"
make
```

You can then run the program with the following command:

```shell
GSETTINGS_SCHEMA_DIR=data/gsettings ./src/pacfinder
```
