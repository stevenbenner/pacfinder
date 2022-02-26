# Translating PacFinder

If you would like to localize PacFinder to a new language or change existing
translations, then please follow the instructions below.

The PacFinder project uses [GNU gettext][gettext] for text localization. This is
the standard tool for text localization on Linux, so you may already be familiar
with the process. If not, then this document provides step-by-step guidance for
non-developers who may be new to this process.

[gettext]: https://www.gnu.org/software/gettext/


## Forking the project

Before you get started, you will want to get a fork of this project on your
local computer that is suitable for making changes and testing.

### Install git

First, you will need to have `git` installed on your computer.

```shell
sudo pacman -Syu git
```

Then you will need to configure git. At the bare minimum you will need to set
your name and email:

```shell
git config --global user.name 'Your Name'
git config --global user.email 'you@example.com'
```

This information is public.

### Set up your dev environment

You should then fork the project on GitHub. You will need a GitHub account, then
you can click the Fork button in the [PacFinder GitHub project][gh-project].
Once you have a fork you can clone that fork to your local machine with git:

```shell
git clone https://github.com/[YOUR_USERNAME]/pacfinder.git
cd pacfinder
```

I recommend that you make your changes in a new git branch. Create a new branch
with the following command:

```shell
git checkout -b update-loc
```

In that example, "update-loc" is the name of the new git branch. This branch
name can be basically anything you like, but you will need to remember it for
submitting your changes.

Then you should run the following commands to set up the build system and
perform the first build:

```shell
./autogen.sh
./configure
make
```

Also, testing the localizations require installing PacFinder from source. So, if
it is already installed on your system then you will need to uninstall it.

```shell
sudo pacman -R pacfinder
```

[gh-project]: https://github.com/stevenbenner/pacfinder


## Making your changes

### Using an editor

The most convenient way to make and edit po files is to use an editor built for
that purpose.

There are many po file editors to choose from, but if you need a recommendation
then the [poedit][poedit] program is a good choice. It is also available in the
Arch Linux official repos:

```shell
sudo pacman -S poedit
```

Please refer to the documentation for your preferred editor for instructions on
creating and editing translations.

[poedit]: https://poedit.net/

### Using gettext tools directly in CLI

If you prefer not to use a po file editor then you can use the gettext tools and
a basic text editor. For full documentation on the GNU gettext tools please
refer to the official [gettext manual][gettext-manual].

Note: Do not attempt to create a LINGUAS file. This is generated automatically
by the build system.

[gettext-manual]: https://www.gnu.org/software/gettext/manual/gettext.html


## Testing your localization

Once you have a po file with the translated text that you want, you should then
test the program to make sure that the new localization is working. This is done
by installing the program and running it.

Run the following commands from the project's root folder to update, build, and
install the program:

```shell
./configure
make
sudo make install
```

You can then run the program. If your system is not set to use the language that
you are adding translations for then you can run the program with the following
command to override the system-default language:

```shell
LANG=ja_JP.utf8 pacfinder
```

After you have tested your changes you should uninstall the program.

```shell
sudo make uninstall
```

Note that the language must be enabled in the operating system. If the language
you want to test doesn't appear in `locale -a` then you will need to enable the
language in your `/etc/locale.gen` file and run the `local-gen` command. For
full instructions, please see the [Locale][wikilocale] topic in the Arch Wiki.

[wikilocale]: https://wiki.archlinux.org/title/Locale


## Submitting your changes

### Commit to git

Once you have entered your translations and are satisfied that they are working,
you will then need to commit them to the git repository and push the changes to
your GitHub fork. Use the following commands:

```shell
git add po/ja_JP.po
git commit
```

The first command instructs `git` to stage the ja_JP.po file from the po folder
for commit, and the second command asks git to commit the changes to the
repository. You will be prompted for a commit message. I recommend that you keep
this message simple. Something like this:

> Create ja_JP localization

or

> Update ja_JP localization

Once you have committed your changes you should then push them to GitHub:

```shell
git push origin -u update-loc
```

### Open pull request

The final step is to create a pull request on the PacFinder GitHub project. Open
the [pull requests][gh-pr] page and create a new pull request based on your fork
for the git branch you created ("update-loc" if you followed the example above).

[gh-pr]: https://github.com/stevenbenner/pacfinder/pulls


## Questions

If you have any questions or run into any problems while translating, please
open a new discussion on GitHub. I will do my best to assist.
