# PacFinder

**Repository & package explorer for Arch Linux.**

PacFinder is a GTK 3 desktop application for browsing packages installed on your [Arch Linux][archlinux] system as well as exploring packages in the Arch Linux official package repositories.

This software is licensed under the [Apache License, Version 2.0][license].

[archlinux]: https://archlinux.org/
[license]: COPYING

## Building from source

### Dependencies

In addition to the [base-devel][base-devel] group of packages, you need to have the following packages installed on your system to compile and run this program:

 * glib2 >= 2.56
 * gtk3 >= 3.22

[base-devel]: https://archlinux.org/groups/x86_64/base-devel/

### Compiling

Run the following commands to build this project from the source code in the git repository and produce an executable suitable for development and debugging:

```shell
./autogen.sh
./configure CFLAGS="-ggdb3 -O0"
make
```

You can then run the program with the following command:

```shell
GSETTINGS_SCHEMA_DIR=data/gsettings ./src/pacfinder
```
