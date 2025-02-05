# enable compiler output
set (CMAKE_VERBOSE_MAKEFILE ON)

# add custom 'thirdparty' target
ADD_CUSTOM_TARGET(thirdparty)

########################################################################################################################
########################################################################################################################
########################################################################################################################

if (FEAT_HAVE_ALGLIB)
  file(GLOB alglib-list "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ALGLIB}/cpp/src/*.cpp")

  ADD_LIBRARY(thirdparty-alglib STATIC ${alglib-list})
  ADD_DEPENDENCIES(thirdparty thirdparty-alglib)
  if (FEAT_COTIRE)
    cotire(thirdparty-alglib)
  endif (FEAT_COTIRE)
  SET_PROPERTY(TARGET thirdparty-alglib APPEND PROPERTY COMPILE_FLAGS " -w")
  SET_PROPERTY(TARGET thirdparty-alglib APPEND PROPERTY LINK_FLAGS " -w")
  if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    SET_PROPERTY(TARGET thirdparty-alglib APPEND_STRING PROPERTY COMPILE_FLAGS " -fno-lto")
    SET_PROPERTY(TARGET thirdparty-alglib APPEND_STRING PROPERTY LINK_FLAGS " -fno-lto")
  endif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
endif (FEAT_HAVE_ALGLIB)

########################################################################################################################
########################################################################################################################
########################################################################################################################

if (FEAT_HAVE_FPARSER)
  file(GLOB fparser-list "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_FPARSER}/*.cc")

  # The 'fpoptimizer.cc' can not be compiled with clang++, so we have to disable
  # it by adding the corresponding pre-processor define
  if (${FEAT_COMPILER_ID} STREQUAL "clang")
    ADD_DEFINITIONS(-DFP_NO_SUPPORT_OPTIMIZER)
  endif (${FEAT_COMPILER_ID} STREQUAL "clang")

  ADD_LIBRARY(thirdparty-fparser STATIC ${fparser-list})
  ADD_DEPENDENCIES(thirdparty thirdparty-fparser)
  SET_PROPERTY(TARGET thirdparty-fparser PROPERTY COMPILE_FLAGS " -w")
  SET_PROPERTY(TARGET thirdparty-fparser APPEND PROPERTY LINK_FLAGS " -w")
endif (FEAT_HAVE_FPARSER)

########################################################################################################################
########################################################################################################################
########################################################################################################################

if (FEAT_HAVE_UMFPACK)
  file(GLOB suite_sparse-list "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_SUITESPARSE}/SuiteSparse_config/*.c")
    ##nice solution: swapp out the .c files to incorporate all variable assembly flags
  #always define our Source biuld through these wrappers
  # if(WIN32)
    file(GLOB amd-list "${FEAT_SOURCE_DIR}/thirdparty/SuiteSparseWrapperFiles/AMD/SourceWrappers/*.c")
    file(GLOB umfpack-list "${FEAT_SOURCE_DIR}/thirdparty/SuiteSparseWrapperFiles/UMFPACK/SourceWrappers/*.c")


  # else(WIN32)
  #   file(GLOB amd-list "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_SUITESPARSE}/AMD/Source/*.c")
  #   file(GLOB umfpack-list "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_SUITESPARSE}/UMFPACK/Source/*.c")
  # endif(WIN32)
  ADD_LIBRARY(thirdparty-umfpack STATIC ${suite_sparse-list} ${amd-list} ${umfpack-list})
  ADD_DEPENDENCIES(thirdparty thirdparty-umfpack)
  if (FEAT_COTIRE)
    cotire(thirdparty-umfpack)
  endif (FEAT_COTIRE)


  if (FEAT_HAVE_MKL AND FEAT_HAVE_OMP)
    SET_PROPERTY(TARGET thirdparty-umfpack APPEND PROPERTY COMPILE_FLAGS " -w -DNCHOLMOD -DNTIMER -DLONGBLAS='long long'")
    if ((${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang") OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU"))
      TARGET_LINK_LIBRARIES(thirdparty-umfpack -Wl,--start-group libmkl_intel_ilp64.a libmkl_core.a libmkl_gnu_thread.a -Wl,--end-group -lpthread -lm -ldl)
    elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
      TARGET_LINK_LIBRARIES(thirdparty-umfpack -Wl,--start-group libmkl_intel_ilp64.a libmkl_core.a libmkl_intel_thread.a -Wl,--end-group -lpthread -lm -ldl)
    else()
      message(WARNING "compiler ${CMAKE_CXX_COMPILER_ID} not know in mkl library selection, using serial mkl library!")
      TARGET_LINK_LIBRARIES(thirdparty-ufmpack -Wl,--start-group libmkl_intel_ilp64.a libmkl_core.a libmkl_sequential.a -Wl,--end-group -lpthread -lm -ldl)

    endif ((${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang") OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU"))
  elseif (FEAT_HAVE_MKL AND NOT FEAT_HAVE_OMP)
    SET_PROPERTY(TARGET thirdparty-umfpack APPEND PROPERTY COMPILE_FLAGS " -w -DNCHOLMOD -DNTIMER -DLONGBLAS='long long'")
    TARGET_LINK_LIBRARIES(thirdparty-umfpack
       -Wl,--start-group libmkl_intel_ilp64.a libmkl_core.a libmkl_sequential.a -Wl,--end-group -lm -ldl)
    if (NOT FEAT_HAVE_MPI OR NOT "${MPI_CXX_LIBRARIES}" MATCHES ".*libpthread.*")
       TARGET_LINK_LIBRARIES(thirdparty-umfpack -lpthread)
    endif (NOT FEAT_HAVE_MPI OR NOT "${MPI_CXX_LIBRARIES}" MATCHES ".*libpthread.*")
  else (FEAT_HAVE_MKL AND FEAT_HAVE_OMP)
    SET_PROPERTY(TARGET thirdparty-umfpack APPEND PROPERTY COMPILE_FLAGS " -w -DNCHOLMOD -DNTIMER -DNBLAS")
  endif (FEAT_HAVE_MKL AND FEAT_HAVE_OMP)

  SET_PROPERTY(TARGET thirdparty-umfpack APPEND PROPERTY LINK_FLAGS " -w")
  SET_PROPERTY(TARGET thirdparty-umfpack APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_SUITESPARSE}/SuiteSparse_config/")
  SET_PROPERTY(TARGET thirdparty-umfpack APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_SUITESPARSE}/AMD/Include/")
  SET_PROPERTY(TARGET thirdparty-umfpack APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_SUITESPARSE}/UMFPACK/Include/")
endif (FEAT_HAVE_UMFPACK)

########################################################################################################################
########################################################################################################################
########################################################################################################################

if (FEAT_HAVE_PARMETIS)
  file(GLOB gklib-list "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_PARMETIS}/metis/GKlib/*.c")
  file(GLOB metis-list "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_PARMETIS}/metis/libmetis/*.c")
  ADD_LIBRARY(thirdparty-metis STATIC ${gklib-list} ${metis-list})
  ADD_DEPENDENCIES(thirdparty thirdparty-metis)
  if (FEAT_COTIRE)
    cotire(thirdparty-metis)
  endif (FEAT_COTIRE)
  SET_PROPERTY(TARGET thirdparty-metis APPEND PROPERTY COMPILE_FLAGS " -w")
  SET_PROPERTY(TARGET thirdparty-metis APPEND PROPERTY LINK_FLAGS " -w")
  SET_PROPERTY(TARGET thirdparty-metis APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_PARMETIS}/metis/GKlib/")
  SET_PROPERTY(TARGET thirdparty-metis APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_PARMETIS}/metis/include")
  SET_PROPERTY(TARGET thirdparty-metis APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_PARMETIS}/metis/libmetis/")

  file(GLOB parmetis-list "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_PARMETIS}/libparmetis/*.c")
  ADD_LIBRARY(thirdparty-parmetis STATIC ${parmetis-list})
  ADD_DEPENDENCIES(thirdparty thirdparty-parmetis)
  if (FEAT_COTIRE)
    cotire(thirdparty-parmetis)
  endif (FEAT_COTIRE)
  SET_PROPERTY(TARGET thirdparty-parmetis PROPERTY COMPILE_FLAGS " -w")
  SET_PROPERTY(TARGET thirdparty-parmetis PROPERTY LINK_FLAGS " -w")
  SET_PROPERTY(TARGET thirdparty-parmetis APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_PARMETIS}/metis/GKlib/")
  SET_PROPERTY(TARGET thirdparty-parmetis APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_PARMETIS}/metis/include/")
  SET_PROPERTY(TARGET thirdparty-parmetis APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_PARMETIS}/libparmetis")
  SET_PROPERTY(TARGET thirdparty-parmetis APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_PARMETIS}/include")
  TARGET_LINK_LIBRARIES(thirdparty-parmetis thirdparty-metis)
endif (FEAT_HAVE_PARMETIS)

########################################################################################################################
########################################################################################################################
########################################################################################################################

if (FEAT_HAVE_ZLIB)
  file(GLOB zlib-list "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZLIB}/*.c")
  ADD_LIBRARY(thirdparty-zlib STATIC ${zlib-list})
  ADD_DEPENDENCIES(thirdparty thirdparty-zlib)
  if (FEAT_COTIRE)
    cotire(thirdparty-zlib)
  endif (FEAT_COTIRE)
  SET_PROPERTY(TARGET thirdparty-zlib APPEND PROPERTY COMPILE_FLAGS " -w")
  SET_PROPERTY(TARGET thirdparty-zlib APPEND PROPERTY LINK_FLAGS " -w")
  #if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  #  SET_PROPERTY(TARGET thirdparty-zlib APPEND_STRING PROPERTY COMPILE_FLAGS " -fno-lto")
  #  SET_PROPERTY(TARGET thirdparty-zlib APPEND_STRING PROPERTY LINK_FLAGS " -fno-lto")
  #endif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
endif (FEAT_HAVE_ZLIB)

########################################################################################################################
########################################################################################################################
########################################################################################################################

if (FEAT_HAVE_TRIANGLE)
  # only this one source file is required
  file(GLOB triangle-list "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_TRIANGLE}/triangle.c")

  ADD_LIBRARY(thirdparty-triangle STATIC ${triangle-list})
  ADD_DEPENDENCIES(thirdparty thirdparty-triangle)
  if (FEAT_COTIRE)
    cotire(thirdparty-triangle)
  endif (FEAT_COTIRE)
  SET_PROPERTY(TARGET thirdparty-triangle APPEND PROPERTY COMPILE_FLAGS " -w -DANSI_DECLARATORS -DTRILIBRARY")
  SET_PROPERTY(TARGET thirdparty-triangle APPEND PROPERTY LINK_FLAGS " -w")
endif (FEAT_HAVE_TRIANGLE)

########################################################################################################################
########################################################################################################################
########################################################################################################################

if (FEAT_HAVE_HYPRE)
  file(READ filelist_hypre hypre-list-in)
  STRING(REGEX REPLACE "\n" ";" hypre-list-in "${hypre-list-in}")
  foreach(hypre-list-item ${hypre-list-in})
    SET(hypre-list "${hypre-list}" "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/${hypre-list-item}")
  endforeach()
  ADD_LIBRARY (thirdparty-hypre STATIC ${hypre-list})
  ADD_DEPENDENCIES(thirdparty thirdparty-hypre)
  configure_file(${FEAT_SOURCE_DIR}/thirdparty/config_hypre.h.in ${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src/HYPRE_config.h @ONLY)
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src")
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src/parcsr_ls")
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src/multivector")
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src/krylov")
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src/distributed_matrix")
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src/matrix_matrix")
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src/IJ_mv")
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src/parcsr_block_mv")
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src/parcsr_mv")
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src/seq_mv")
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src/sstruct_ls")
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src/sstruct_mv")
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src/struct_ls")
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src/struct_mv")
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src/utilities")
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src/blas")
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_HYPRE}/src/lapack")
  if (FEAT_HAVE_MPI)
    if(WIN32 AND (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
      SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY COMPILE_FLAGS " -w -DHYPRE_HAVE_MPI -DWIN32")
    else(WIN32 AND (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
      SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY COMPILE_FLAGS " -w -DHYPRE_HAVE_MPI")
    endif(WIN32 AND (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
  else (FEAT_HAVE_MPI)
    if(WIN32 AND (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
      SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY COMPILE_FLAGS " -w -DHYPRE_SEQUENTIAL -DWIN32")
    else(WIN32 AND (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
      SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY COMPILE_FLAGS " -w -DHYPRE_SEQUENTIAL")
    endif(WIN32 AND (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
  endif (FEAT_HAVE_MPI)
  SET_PROPERTY(TARGET thirdparty-hypre APPEND PROPERTY LINK_FLAGS " -w")
endif (FEAT_HAVE_HYPRE)

########################################################################################################################
########################################################################################################################
########################################################################################################################

if (FEAT_HAVE_DEATH_HANDLER)
  ADD_LIBRARY(thirdparty-deathhandler STATIC "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_DEATHHANDLER}/DeathHandler-master/death_handler.cc")
  ADD_DEPENDENCIES(thirdparty thirdparty-deathhandler)
  SET_PROPERTY(TARGET thirdparty-deathhandler APPEND PROPERTY COMPILE_FLAGS " -w")
  SET_PROPERTY(TARGET thirdparty-deathhandler APPEND PROPERTY LINK_FLAGS " -w")
endif (FEAT_HAVE_DEATH_HANDLER)

########################################################################################################################
########################################################################################################################
########################################################################################################################

if (FEAT_HAVE_ZFP)
  file(GLOB zfp-list "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZFP}/src/*.c")

  ADD_LIBRARY(thirdparty-zfp STATIC ${zfp-list})
  ADD_DEPENDENCIES(thirdparty thirdparty-zfp)
  if (FEAT_COTIRE)
    cotire(thirdparty-zfp)
  endif (FEAT_COTIRE)
  SET_PROPERTY(TARGET thirdparty-zfp APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZFP}/src")
  SET_PROPERTY(TARGET thirdparty-zfp APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZFP}/include")
  SET_PROPERTY(TARGET thirdparty-zfp APPEND PROPERTY COMPILE_FLAGS " -w")
  SET_PROPERTY(TARGET thirdparty-zfp APPEND PROPERTY LINK_FLAGS " -w")
endif (FEAT_HAVE_ZFP)

########################################################################################################################
########################################################################################################################
########################################################################################################################

if (FEAT_HAVE_SUPERLU_DIST)
  file(READ filelist_superlu superlu-list-in)
  STRING(REGEX REPLACE "\n" ";" superlu-list-in "${superlu-list-in}")
  foreach(superlu-list-item ${superlu-list-in})
    SET(superlu-list "${superlu-list}" "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_SUPERLU}/${superlu-list-item}")
  endforeach()
  ADD_LIBRARY (thirdparty-superlu STATIC ${superlu-list})
  ADD_DEPENDENCIES(thirdparty thirdparty-superlu)
  configure_file(${FEAT_SOURCE_DIR}/thirdparty/superlu_dist_config.h.in ${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_SUPERLU}/SRC/superlu_dist_config.h @ONLY)
  SET_PROPERTY(TARGET thirdparty-superlu APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_SUPERLU}/SRC")
  SET_PROPERTY(TARGET thirdparty-superlu PROPERTY C_STANDARD 99)
  if (FEAT_HAVE_MPI)
    SET_PROPERTY(TARGET thirdparty-superlu APPEND PROPERTY COMPILE_FLAGS " -w")
  endif (FEAT_HAVE_MPI)
  SET_PROPERTY(TARGET thirdparty-superlu APPEND PROPERTY LINK_FLAGS " -w")
endif (FEAT_HAVE_SUPERLU_DIST)

########################################################################################################################
########################################################################################################################
########################################################################################################################

if (FEAT_HAVE_ZOLTAN)
  file(READ filelist_zoltan zoltan-list-in)
  STRING(REGEX REPLACE "\n" ";" zoltan-list-in "${zoltan-list-in}")
  foreach(zoltan-list-item ${zoltan-list-in})
    SET(zoltan-list "${zoltan-list}" "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/${zoltan-list-item}")
  endforeach()
  ADD_LIBRARY (thirdparty-zoltan STATIC ${zoltan-list})
  ADD_DEPENDENCIES(thirdparty thirdparty-zoltan)
  configure_file(${FEAT_SOURCE_DIR}/thirdparty/config_zoltan.h.in ${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/include/Zoltan_config.h @ONLY)
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/include")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/all")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/Utilities/shared")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/Utilities/Timer")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/zz")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/coloring")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/graph")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/ha")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/hier")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/hsfc")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/lb")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/matrix")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/order")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/par")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/params")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/phg")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/rcb")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/reftree")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/simple")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/timer")
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY INCLUDE_DIRECTORIES "${FEAT_SOURCE_DIR}/thirdparty/${FEAT_DIRNAME_ZOLTAN}/src/tpls")
  if(WIN32 AND (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
   #define __STDC__ since clang for some reason does not "confirm" to the standard,
   #but in this case this flag should be set to let zoltan compile
    SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY COMPILE_FLAGS "-D__STDC__=1 -w")
  else(WIN32 AND (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
    SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY COMPILE_FLAGS " -w")
  endif(WIN32 AND (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
  SET_PROPERTY(TARGET thirdparty-zoltan APPEND PROPERTY LINK_FLAGS " -w")
endif (FEAT_HAVE_ZOLTAN)
