# Documentation will be optionally built using Doxygen.
find_package(Doxygen QUIET)

# Provide the option to build the documentation.
# Defaults to true if Doxygen is installed.
option(
  BUILD_DOC
  "If enabled, Doxygen will be used to build the documentation."
  ${Doxygen_FOUND}
)

# Provide the option to run Doxygen in quet mode.
set(
  QUIET_DOXYGEN
  YES
  CACHE
  STRING
  "If 'YES', Dxygen will run in quiet mode, thus not producing messages on the standard output (warning and errors are still reported)."
)
set_property(CACHE QUIET_DOXYGEN PROPERTY STRINGS YES NO)

# Provide the option to run Doxygen in quet mode.
set(
  DOXYGEN_MAKE_LATEX
  NO
  CACHE
  STRING
  "If 'YES', Dxygen will create a latex-compilable documentation (in addition to the html output)."
)
set_property(CACHE DOXYGEN_MAKE_LATEX PROPERTY STRINGS YES NO)



# Process the documentation if needed.
if(${BUILD_DOC})
  if(${Doxygen_FOUND})
    # Perform substitution of variables in the doxy-file.
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    # Create the "doc" target which will call Doxygen.
    add_custom_target(
      doc ALL
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM
    )
    if(${DOXYGEN_MAKE_LATEX})
      set(DOXYGEN_LATEX_LOG_FILE ${CMAKE_CURRENT_BINARY_DIR}/doc-latex.log)
      add_custom_target(
        doc-latex ALL
        COMMAND make > ${DOXYGEN_LATEX_LOG_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/latex
        COMMENT "Generating API manual using LaTeX"
        VERBATIM
      )
      add_dependencies(doc-latex doc)
    endif(${DOXYGEN_MAKE_LATEX})
  else(${Doxygen_FOUND})
    message(FATAL_ERROR "Doxygen not found, documentation cannot be generated")
  endif(${Doxygen_FOUND})
else(${BUILD_DOC})
  message(STATUS "Doxygen documentation is DISABLED, you can enable it using -DBUILD_DOC=ON (provided that you have Doxygen installed)")
endif(${BUILD_DOC})
