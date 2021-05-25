[TOC]

# CMake

This project requires [CMake](https://cmake.org/). To obtain the latest version of CMake, you can follow the [official instructions](https://cmake.org/install/). Alternatively, you can install CMake from `apt` using

```
sudo apt install cmake
```

Note that we built and tested the project using version 3.19.4 and thus we specified it as the minimum required version. However, it is very likely that earlier versions will work as well. Feel free to just change the required version passed to `cmake_minimum_required` in `CMakeLists.txt` and report the result to us.


# Python3

Python3 should be already installed on Raspberry Pi running Raspbian. Nonetheless, you can manually install it using:

```
sudo apt install python3 python3-dev python3-pip
```


# pigpio

To build the code of this repository, you need the [pigpio](https://github.com/joan2937/pigpio) library. Its documentation is available [here](http://abyz.me.uk/rpi/pigpio/index.html). The following is a summary of what you need to write in a console to build pigpio using CMake:

```
git clone https://github.com/joan2937/pigpio.git
mkdir pigpio/build
cd pigpio/build
cmake ..
make
sudo make install
```


# Digital Filters

The [digital_filters repository](https://github.com/francofusco/digital_filters) can be installed by following the instructions contained inside the README:

```
git clone https://github.com/francofusco/digital_filters.git
mkdir digital_filters/build
cd digital_filters/build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
echo "export digital_filters_DIR="`pwd`" >> ~/.bashrc
```


# ZeroMQ

ZeroMQ is the library used here for inter-process communications. You will need to install this library and several other components.


## libsodium

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



## ZeroMQ

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


# YamlCpp

TODO!


# DoxyGen

DoxyGen can be used to generate a copy of this documentation. If you do not hqve this tool instqlled yet, you can do it via:

```
sudo apt install doxygen graphviz
```

Additionally, you can consider installing the packages `doxygen-gui` and/or `doxygen-latex`.
