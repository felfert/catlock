# catlock
This is a simple program to protect my computer from my cat walking over the keyboard
and playing with the mouse :-)

It was inspired by xtrlock, but instead requiring a password to unlock, a simple three-key combination (**Ctrl+Alt+KP_Enter**) is sufficient to terminate. Furthermore, pressing **Meta+L** switches off the screen backlight (like xset dpms force off does). These shortcuts are configurable. Just run ```catlock --help``` to get a short description.

This software is distributed under a **BSD-2-Clause** license. The following parts have been taken from external projects:

* The cat icon and cat bitmap for the cursor is taken from [oneko](https://directory.fsf.org/wiki/Oneko) which is public domain.
* The cmake FindXCB script and it's dependency (BSD-3-Clause license) from the [KDE project](https://github.com/KDE/extra-cmake-modules).
* The cmake FindPOPT script (BSD-2-Clause license) from Lars Baehren's [CMakeModules project](https://github.com/lbaehren/CMakeModules).
* The cmake FindXmlTo script (BSD-3-Clause license) from the [rabbitmq-c project](https://github.com/alanxz/rabbitmq-c)

## How to build

### Prerequisites
catlock requires XCB including it's extensions KEYSYMS IMAGE and DPMS. It uses cmake as a build system and is written in **C++**. So:

On a **Ubuntu-18.04** system in order to get all required tools and dependencies, you would run:
```
sudo apt-get install git cmake g++ pkg-config \
xmlto libxcb-keysyms1-dev libxcb-image0-dev \
libxcb-dpms0-dev libxext-dev libpopt-dev
```

On a **Fedora-29** system, you would run:
```
sudo dnf install git cmake gcc-c++ pkg-config xmlto \
make libxcb-devel xcb-util-keysyms-devel \
xcb-util-image-devel libX11-devel popt-devel
```
### Fetch the repo
```
git clone https://github.com/felfert/catlock.git
```
### The actual build and install
```
mkdir catlock/build
cd catlock/build
cmake ..
make
sudo make install
```
