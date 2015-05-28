CMAKE_MINIMUM_REQUIRED(VERSION 2.8) 

PROJECT (spectrum_viz)

#
#	dir variables
#
SET(incl_dir "${CMAKE_CURRENT_SOURCE_DIR}/incl")
MESSAGE (STATUS  "** incl_dir - ${incl_dir}")
SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}/src")
MESSAGE (STATUS  "** src_dir - ${src_dir}")

SET(glew_dir "${CMAKE_CURRENT_SOURCE_DIR}/ext/glew")
MESSAGE (STATUS  "** glew_dir - ${glew_dir}")

SET(bass_dir "${CMAKE_CURRENT_SOURCE_DIR}/ext/bass")
MESSAGE (STATUS  "** bass_dir - ${bass_dir}")

SET(res_dir "${CMAKE_CURRENT_SOURCE_DIR}/res")
MESSAGE (STATUS  "** res_dir - ${res_dir}")

#
#	source files
#
FILE(GLOB_RECURSE PROJECT_INCL_FILES "${incl_dir}/*.h")
FILE(GLOB PROJECT_SRC_FILES "${src_dir}/*.cpp")

ADD_DEFINITIONS(-Drdir_="${res_dir}/")

#
#	register project as executable
#
ADD_EXECUTABLE( ${CMAKE_PROJECT_NAME} WIN32 
				${PROJECT_INCL_FILES} ${PROJECT_SRC_FILES} )

TARGET_LINK_LIBRARIES(	${CMAKE_PROJECT_NAME} "${bass_dir}/bass.lib" "${glew_dir}/glew32s.lib")

SET_TARGET_PROPERTIES(	${CMAKE_PROJECT_NAME} 
						PROPERTIES INCLUDE_DIRECTORIES "${incl_dir};${glew_dir};${bass_dir}")

#
#	output directory and path to executable
#
SET(exec_dir $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>) 

#
#	copy bass.dll into the same directory as the built executable
#
ADD_CUSTOM_COMMAND(	TARGET ${CMAKE_PROJECT_NAME} PRE_BUILD
               		COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   	"${bass_dir}/bass.dll" ${exec_dir})
