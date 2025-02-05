# CMakeLists for directory "applications/ccnd"

# enable compiler output
SET (CMAKE_VERBOSE_MAKEFILE ON)

# add all executables
ADD_EXECUTABLE(ccnd-steady-dfg95-2d ccnd_steady_dfg95.cpp)
ADD_EXECUTABLE(ccnd-steady-dfg95-3d ccnd_steady_dfg95.cpp)
ADD_EXECUTABLE(ccnd-steady-taygre-2d ccnd_steady_taygre.cpp)
ADD_EXECUTABLE(ccnd-steady-ypipe-2d ccnd_steady_ypipe.cpp)
ADD_EXECUTABLE(ccnd-unsteady-dfg95-2d ccnd_unsteady_dfg95.cpp)
ADD_EXECUTABLE(ccnd-unsteady-dfg95-3d ccnd_unsteady_dfg95.cpp)

# define 2D app list
SET(app_list_2d
  ccnd-steady-dfg95-2d
  ccnd-unsteady-dfg95-2d
  ccnd-steady-taygre-2d
  ccnd-steady-ypipe-2d
)

# define 3D app list
SET(app_list_3d
  ccnd-steady-dfg95-3d
  ccnd-unsteady-dfg95-3d
)


# list of all applications
SET(app_list ${app_list_2d} ${app_list_3d})

# set dimension
FOREACH(app ${app_list_2d})
  TARGET_COMPILE_DEFINITIONS(${app} PUBLIC FEAT_CCND_APP_DIM=2)
ENDFOREACH(app)
FOREACH(app ${app_list_3d})
  TARGET_COMPILE_DEFINITIONS(${app} PUBLIC FEAT_CCND_APP_DIM=3)
ENDFOREACH(app)

# set the other compile options for all applications
FOREACH(app ${app_list})
  TARGET_LINK_LIBRARIES(${app} feat)
  TARGET_COMPILE_DEFINITIONS(${app} PUBLIC FEAT_CCND_APP_ISOPARAM)
  IF(MPI_C_COMPILE_FLAGS)
    SET_TARGET_PROPERTIES(${app} PROPERTIES COMPILE_FLAGS "${MPI_C_COMPILE_FLAGS}")
  ENDIF()
  IF(MPI_C_LINK_FLAGS)
    SET_TARGET_PROPERTIES(${app} PROPERTIES LINK_FLAGS "${MPI_C_LINK_FLAGS}")
  ENDIF()
ENDFOREACH(app)
