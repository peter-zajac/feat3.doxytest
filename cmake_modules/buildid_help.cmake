# FEAST BUILD_ID mechanism: dump a nicely formatted help / error message
#
# \author Dominik Goeddeke
# \author Dirk Ribbrock
#
# TODO better ASCII art :)

message (STATUS "##############################################################")
message (STATUS "Using FEAST's BUILD_ID mechanism")
message (STATUS "##############################################################")
message (STATUS "The BUILD_ID mechanism is the only officially supported way to")
message (STATUS "configure FEAST. Assuming we are in the directory where FEAST ")
message (STATUS "should be configured and compiled in, these are valid options:")
message (STATUS "  \"cmake -D BUILD_ID=HELP /path/to/sources\"                 ")
message (STATUS "     Display this help message                                ")
message (STATUS "  \"cmake /path/to/sources\"                                  ")
message (STATUS "     This is the default and will enable FEAST's autodetection")
message (STATUS "     mechanism for the underlying hardware. FEAST will be con-")
message (STATUS "     figured using the GNU compiler suite, OpenMPI, and a rea-")
message (STATUS "     sonable set of compiler optimisation flags.              ")
message (STATUS "  \"cmake -D BUILD_ID=my-build-id /path/to/sources\"          ")
message (STATUS "     Uses the given build-ID, see below for details.          ")
message (STATUS "  \"cmake -D BUILD_ID=MANUAL /path/to/sources\"               ")
message (STATUS "     Disables the build-ID mechanism completely. This is un-  ")
message (STATUS "     supported and only meant for advanced developers who know")
message (STATUS "     what they are doing and in particular know the ins and   ")
message (STATUS "     outs of cmake and FEAST.                                 ")
message (STATUS "##############################################################")
message (STATUS "Some notes on the concept of build-IDs                        ")
message (STATUS "##############################################################")
message (STATUS "A build-ID in FEAST language is a list of tokens that uniquely")
message (STATUS "identifies a target environment to configure and compile FEAST")
message (STATUS "for. For instance, tokens identify the optimisation level,    ")
message (STATUS "the MPI implementation, the compiler, and the target hardware ")
message (STATUS "(including microarchitecture and hardware type, e.g. CPU/GPU),")
message (STATUS "all of which lead to different compiler and linker flags and  ")
message (STATUS "library dependencies that the build systems resolves automati-")
message (STATUS "cally under the hood. Tokens are separated by single dashes,  ")
message (STATUS "and the order of tokens is arbitrary. If there are conflicting")
message (STATUS "tokens (e.g., simultaneously requested support for different  ")
message (STATUS "compiler suites), then the first valid entry counts.          ")
message (STATUS "If the build-ID is set explicitly via \"BUILD_ID=my-build-id\"")
message (STATUS "then the following tokens supported, see below for valid      ")
message (STATUS "settings for each token:                                      ")
message (STATUS "  mode    : debug (pedantic error checking) or optimised      ")
message (STATUS "            code for the target architecture [mandatory token]")
message (STATUS "  mpi     : serial or MPI build, in the latter case this sets ")
message (STATUS "            the MPI environment to be used [mandatory token]  ")
message (STATUS "  compiler: the compiler suite to be used [mandatory token]   ")
message (STATUS "  arch:     the microarchitecture to be used for the CPUs and ")
message (STATUS "            other devices in the system, e.g.,                ")
message (STATUS "              nehalem                                         ")
message (STATUS "              westmere-opencl                                 ")
message (STATUS "              magnycours-cuda2.1                              ")
message (STATUS "            If this token is omitted, then the current        ")
message (STATUS "            architecture will be determined automatically, and")
message (STATUS "            optimised compiler settings will be set.          ")

# give information about each of these tokens, idea: Each individual cmake module
# dumps out its own information for less cluttered code
include ( ${FEAST_SOURCE_DIR}/cmake_modules/buildid_mode.cmake )
include ( ${FEAST_SOURCE_DIR}/cmake_modules/buildid_mpi.cmake )
include ( ${FEAST_SOURCE_DIR}/cmake_modules/buildid_compiler.cmake )
