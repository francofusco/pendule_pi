# pendule_pi

C++ utilities to control a pendulum from a Raspberry Pi4

## Dependencies

To build the library, the tests, the examples, *etc.*, you need the
[pigpio](https://github.com/joan2937/pigpio) library. Its documentation is
available [here](http://abyz.me.uk/rpi/pigpio/index.html).

To build the documentation, you need Doxygen and GraphViz. On Ubuntu/Debian,
you should be able to install them via:
```
apt install doxygen doxygen-gui graphviz
```
(you might need privileges for this!)

## How to compile

This project requires CMake 3.15 at least (it might work with lower versions,
but this is not tested at all). Follow the "usual" procedure:

```
mkdir build && cd build
cmake ..
cmake --build .
```

To view the documentation, open in your browser the file `doc/html/index.html`.


### Enable debug mode

```
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```


### Disable documentation

```
cmake .. -DBUILD_DOC=OFF
cmake --build .
```


### Documentation only

```
cmake ..
cmake --build . --target doc
```
