@page troubleshooting Troubleshooting

[TOC]

# General Warnings

## DoxyGen configuration file

:warning: Please, do *not* use the `doxywizard` (aka `doxygen-gui`) application to edit the Doxygen configuration file `doc/doxyfile.in`. In fact, it contains some CMake-configurable variables (such as `@GENERATE_LATEX@`) that would be overridden by `doxywizard` - it expects them to contain either `YES` or `NO` and freaks out if anything else is detected. The result is that it would break some of the functionalities of the CMake configuration.
