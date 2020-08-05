
cmake_minimum_required(VERSION 3.16)

project(ProjectTemplate CXX)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS    "-Wall -Wextra -mtune=native -O2" )

# include_directories( <include path of external library> )

# add_library( <library> STATIC <path to source code> [...] )

# add_executable( <executable_name> <path to source code> [...] )
# target_link_libraries( <executable_name> <libary_name> [...] )

# Exsample :
# include_directories($ENV{HOME}/local/include/eigen3)

# add_library(NumericalExperiment STATIC src/Experiment.cpp src/UUID.cpp src/Model.cpp src/ODE_Solver.cpp)
# add_library(JSONParser STATIC src/JSONParser.cpp)

# add_executable(run_numerical_experiment src/run_numerical_experiment.cpp)
# target_link_libraries(run_numerical_experiment NumericalExperiment uuid)

add_executable(greeting src/hallo_welt.cpp)


