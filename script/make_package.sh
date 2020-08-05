#!/bin/sh

project_root=`dirname ${0}`
project_root=`cd ${project_root}/../; pwd`

package_name=`basename ${project_root}`

echo ${project_root} ${package_name}

tar cfJ ${package_name}_`date +%Y_%m_%d`.tar.xz -C ${project_root}/../ ${package_name}/src ${package_name}/script ${package_name}/CMakeLists.txt
