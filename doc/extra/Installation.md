[TOC]

# Dependencies

In the following you will find a list of all software dependencies needed to compile pendule_pi. Most of them will be built from source, which generally requires three steps:

1. Download the code;
2. Build the sources;
3. Install headers and libraries.

While you are absolutely free to do as you please, we recommend to create a folder `programs` in your home and to move there whenever you need to download the sources of a dependency.


## CMake

This project requires [CMake](https://cmake.org/). To obtain the latest version of CMake, you can follow the [official instructions](https://cmake.org/install/). Alternatively, you can install CMake from `apt` using

```
sudo apt install cmake
```

Note that we built and tested the project using version 3.19.4 and thus we specified it as the minimum required version. However, it is very likely that earlier versions will work as well. Feel free to just change the required version passed to `cmake_minimum_required` in `CMakeLists.txt` and report the result to us.


## Python3

Python3 should be already installed on Raspberry Pi running Raspbian. Nonetheless, you can manually install it using:

```
sudo apt install python3 python3-dev python3-pip
```


## pigpio

To build the code of this repository, you need the [pigpio](https://github.com/joan2937/pigpio) library. Its documentation is available [here](http://abyz.me.uk/rpi/pigpio/index.html). The following is a summary of what you need to write in a console to build pigpio using CMake:

```
git clone https://github.com/joan2937/pigpio.git
mkdir pigpio/build
cd pigpio/build
cmake ..
make
sudo make install
```


## Digital Filters

The [digital_filters repository](https://github.com/francofusco/digital_filters) can be installed by following the instructions contained inside the README:

```
git clone https://github.com/francofusco/digital_filters.git
mkdir digital_filters/build
cd digital_filters/build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
echo "export digital_filters_DIR="`pwd`" >> ~/.bashrc
```


## ZeroMQ

ZeroMQ is the library used here for inter-process communications. You will need to install this library and several other components.


### libsodium

ZeroMQ is built on top of `libsodium`. To install it, download the latest version from the [releases page](https://download.libsodium.org/libsodium/releases) and then run the following instructions:

```
tar -zxvf LATEST.tar.gz
cd libsodium-stable
./configure
make
make check
sudo make install
sudo ldconfig
```

Note that the latest version at the time of writing is 1.0.18. If you encounter problems with the current latest release, you might want to revert to this version instead.



### ZeroMQ

After a successful installation of `libsodium`, you can install the latest version of ZeroMQ via:

```
sudo apt install libtool-bin
git clone git://github.com/zeromq/libzmq.git
cd libzmq
./autogen.sh
./configure --with-libsodium
make
sudo make install
sudo ldconfig
```

The version of ZeroMQ at the time of writing is 4.3.5, commit [78ea4ee](https://github.com/zeromq/libzmq/commit/78ea4ee7878779fb72d3551fb0d79b7b0a5ecd22).


### C++ bindings

To use ZeroMQ in C++, we use the [zmqpp](https://github.com/zeromq/zmqpp) bindings. They can be installed with:

```
git clone git://github.com/zeromq/zmqpp.git
cd zmqpp
make
sudo make install
sudo ldconfig
```

The version at the time of writing is 4.2.0, commit [85ae960](https://github.com/zeromq/zmqpp/commit/85ae96020f2376c53d2176e04e88e8e51021b748).


### Python bindings

The Python module `zmq` can be installed very easily via `pip`:

```
pip3 install pyzmq
```

The command above should install it for the current user only. To install it for all users, prepend `sudo` in front of the command.


## yaml-cpp

This library allows to parse YAML files in C++. In this way, parameters can be passed to executables in a more efficient way.

At the time of writing there was a [small issue](https://github.com/jbeder/yaml-cpp/issues/774) in the original yaml-cpp repository. We thus created a fork containing a workaround that (at least for us) solved the problem. The instructions below use our fork rather than the original repository.

```
git clone --branch fix-cmake https://github.com/francofusco/yaml-cpp.git
mkdir yaml-cpp/build
cd yaml-cpp/build
cmake ..
cmake --build .
sudo cmake --install .
```


## DoxyGen (optional)

[DoxyGen](https://www.doxygen.nl/index.html) can be used to generate a local copy of this documentation. If you do not have this tool installed yet, you can do it via:

```
sudo apt install doxygen graphviz
```

Additionally, you can consider installing the packages `doxygen-gui` and/or `doxygen-latex`.



# pendule_pi

The project can be build using CMake. First, create the build directory and move inside it. Assuming that you are in the root directory of pendule_pi, run:
```
mkdir build
cd build
```
From now on, all commands are to be run from there.

Note that the following is a detailed description of the steps required to configure, build and install pendule_pi's library and executables. For a quick summary of the required steps, refer to the **TL;DR** section below.

## Building the code

### Configuring the build

Let's start with the configuration step:
```
cmake ..
```
which should check for existence of some compilers and dependencies. Options can be passed to CMake during this step to configure the build. Options are to be passed as follows:
```
cmake .. -DOPTION=VALUE
```
Alternatively, you can build the CMake curses gui or the Qt-based one and change the options there. **TODO: put links to the GUIs!** The following options are available:

- `CMAKE_BUILD_TYPE`: either `Debug` or `Release`. In the first mode, the `O0` optimization flag is passed to the main library, more warnings are printed during code compilation and at runtime the generated programs will print some debug information about what is being executed. In the `Release` mode, the `O2` optimization flag is given instead, less warnings are produced during compilation and debug messages are deactivated.

- `BUILD_DOC`: either `ON` or `OFF`. If `OFF`, documentation will not be generated. If `ON`, then DoxyGen will be used to build a local copy of this documentation. By default, this option is set to `ON` if DoxyGen is installed (and found) on your Raspberry, and to `OFF` otherwise. Note that if you have installed DoxyGen, then you can change this option with no constraints. However, if DoxyGen is not found and you force the option to `ON`, an error will be sent by CMake.

- `QUIET_DOXYGEN`: either `YES` or `NO`. If `YES` (default), DoxyGen should print only warnings on the console when running. If set to `NO`, verbose output will be produced instead.

- `DOXYGEN_MAKE_LATEX`: either `YES` or `NO`. If `YES`, then the documentation will be generated also in the form of a "reference manual" (print-friendly pdf) using LaTeX (it requires the `doxygen-latex` package to be installed, see the DoxyGen installation section above). If the option is set to `NO` (default) only the HTML version of the documentation will be generated.  


### Building the targets

After the configuration step, the code can be built. To do so, run
```
cmake --build .
```
in the terminal. This should build all targets (libraries, executables, documentation). You can build selected targets using the form:
```
cmake --build . --target TARGET
```
As an example, the documentation target is called `doc`, and can be built via:
```
cmake --build . --target doc
```

### Installing the project

There is no need to install the built code since everything can be run from the `build` folder and a way to locate headers and other files is explained later. However, if you wish to do so, you can install all built products by executing:
```bash
cmake --install .
```

By default, the install location should be `/usr/local` and you might need
root privileges to properly copy the files. If you do not have the rights (or
if you prefer to install the project somewhere else for any reason) you can
change the install location during the configuration step. As an example:
```
cmake .. -DCMAKE_INSTALL_PREFIX=~/barbaz
```
would install the header and other compiled targets in the `barbaz` directory
in your home.

If at any point you wish to "uninstall" the project, you can do it thanks to a
provided CMake script. In particular, after installing the project, move into
the build location and run:

```bash
cmake -P uninstall.cmake
```

**Beware of the following limitations!** The `uninstall.cmake` script
is very simple and limited. It reads the `install_manifest.txt` that is
generated during the install step and attempts to remove, one by one, each
copied target. This implies that:

- Folders are not removed, but left empty. If you want to remove them, you have
  to do so manually.

- Since the script reads `install_manifest.txt`, if you remove the build folder
  and/or change the installed products, the script might not correctly and
  completely remove all installed targets.


## Configure pendule_pi for external usage

### Python

To use the Python interface to control the pendulum, you need to make the pendule_pi module available to the Python ecosystem. One way to do so is to extend the `PYTHONPATH` environment variable. One way to do so is:
```
echo "export PYTHONPATH=\"PYTHONPATH:`pwd`/../src/python/\"" >> ~/.bashrc
```
This will append one line to your `.bashrc`. The command assumes that you are in the build folder and that the build folder is located in the root directory of pendule_pi. If this is not the case, just insert manually the line
```
export PYTHONPATH="PYTHONPATH:/path/to/pendule_pi/src/python/"
```
to your `.bashrc`.


### C++

To use the C++ libraries provided by pendule_pi in other executables, you firstly need to make the project "discoverable" by CMake. This can be done in two ways: either install the project as discussed above or set an environment variable as follows:
```
echo "export pendule_pi_DIR=\"`pwd`\"" >> ~/.bashrc
```

The libraries can now be used in other CMake-based projects by adding one of the following
```cmake
find_package(pendule_pi)
find_package(pendule_pi QUIET)
find_package(pendule_pi REQUIRED)
```
in their `CMakeLists.txt`. Then, just link the `pendule_pi::pendule_pi` library to your targets, *e.g.*,
```cmake
add_executable(your_program your_source.cpp)
target_link_libraries(your_program pendule_pi::pendule_pi)
```
Note that headers are "automatically" made visible to your executable, and
there is thus no need for any of the following
```cmake
include_directories(${pendule_pi_INCLUDE_DIRS})
target_include_directories(your_program ${pendule_pi_INCLUDE_DIRS})
```
Indeed, when you call `find_package`, no `pendule_pi_INCLUDE_DIRS` is populated at all!


## TL;DR

From the root of pendule_pi run:
```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
echo "export PYTHONPATH=\"PYTHONPATH:`pwd`/../src/python/\"" >> ~/.bashrc
echo "export pendule_pi_DIR=\"`pwd`\"" >> ~/.bashrc
```
