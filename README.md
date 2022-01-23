# PacFinder

**Repository & package explorer for Arch Linux.**

PacFinder is a GTK 3 desktop application for browsing packages installed on your [Arch Linux][archlinux] system as well as exploring packages in the Arch Linux official package repositories.

[archlinux]: https://archlinux.org/

## Building from source

### Dependencies

In addition to the [base-devel][base-devel] group of packages, you need to have the following packages installed on your system to compile and run this program:

 * glib2 >= 2.56
 * gtk3 >= 3.22
 * pacman >= 6.0

[base-devel]: https://archlinux.org/groups/x86_64/base-devel/

### Compiling

Run the following commands to build this project from the code in the git repository:

```shell
./autogen.sh
./configure
make
```
