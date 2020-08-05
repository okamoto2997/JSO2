#!/bin/sh

project_root=`dirname ${0}`
project_root=`cd ${project_root}/../; pwd`

project_name=`basename ${project_root}`

tmp_directory="/dev/shm"
working_directory=${tmp_directory}/${project_name}-wd

vscode_work_space="${project_root}/../${project_name}.code-workspace"

echo "set up the project : ${project_name}"

cd ${project_root}

if [ ! -f "CMakeLists.txt" ]; then
	echo "preparing CMakeLists.txt"
	echo "
cmake_minimum_required(VERSION 3.16)

project(${project_name} CXX)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS    \"-Wall -Wextra -mtune=native -O2\" )

# include_directories( <include path of external library> )

# add_library( <library> STATIC <path to source code> [...] )

# add_executable( <executable_name> <path to source code> [...] )
# target_link_libraries( <executable_name> <libary_name> [...] )

# Exsample :
# include_directories(\$ENV{HOME}/local/include/eigen3)

# add_library(NumericalExperiment STATIC src/Experiment.cpp src/UUID.cpp src/Model.cpp src/ODE_Solver.cpp)
# add_library(JSONParser STATIC src/JSONParser.cpp)

# add_executable(run_numerical_experiment src/run_numerical_experiment.cpp)
# target_link_libraries(run_numerical_experiment NumericalExperiment uuid)

add_executable(greeting src/hallo_welt.cpp)

" > CMakeLists.txt
fi

if [ ! -d "${working_directory}" ]; then
	echo "preparing working directory : " ${working_directory}
	mkdir -p ${working_directory}
fi

if [ ! -f "${vscode_work_space}" ]; then
	echo "preparing vscode workspace config file : " ${vscode_work_space}
	echo "
{
	\"folders\": [
		{
			\"path\": \"${project_root}\"
		},
		{
			\"path\": \"${working_directory}\"
		}
	],
	\"settings\": {}
}
	" > ${vscode_work_space}
fi

cd ${working_directory}
cmake ${project_root} && make && echo "set up completed."
