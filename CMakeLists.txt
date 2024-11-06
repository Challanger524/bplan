cmake_minimum_required (VERSION 3.26)

project (bplan  DESCRIPTION  "Budget Planner (UA)"
                HOMEPAGE_URL "https://github.com/Challanger524/bplan"
                LANGUAGES     CXX)


# # # Setup: project # # #

add_executable            (${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/include"
                                                   "${PROJECT_SOURCE_DIR}/src"           )
target_precompile_headers (${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src/config.hpp") # force include to all source files
set_target_properties     (${PROJECT_NAME} PROPERTIES
  CXX_STANDARD          23
  CXX_STANDARD_REQUIRED ON
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}$<0:>" # deal with resource depsendencies ($<0:> - dummy gen-expression to enforce exact value):
  EXPORT_COMPILE_COMMANDS   ON
)

# # Apply: compile definitions # #
target_compile_definitions(${PROJECT_NAME} PRIVATE "${_DEFINITIONS_}")

# # Gather: sources # #
file(GLOB_RECURSE SOURCE_FILES_ CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")
list(FILTER SOURCE_FILES_ EXCLUDE REGEX ".*\\/_.*\\..*") # exclude files starting with uncderscore `_<file>`: https://regex101.com/r/DT6qGi/1
target_sources        (${PROJECT_NAME} PRIVATE ${SOURCE_FILES_})

# # Apply: compiler and linker flags (from: CMakePresets.json) # #
target_link_options   (${PROJECT_NAME} PRIVATE "${_FLAGS_LINKER_}")    # flags: linker
# flags: compiler & linker (mutual)
target_link_options   (${PROJECT_NAME} PRIVATE "${_FLAGS_COMP_LINK_}") #
target_compile_options(${PROJECT_NAME} PRIVATE "${_FLAGS_COMP_LINK_}") #
# flags: compliler
target_compile_options(${PROJECT_NAME} PRIVATE "${_FLAGS_COMPILE_0_}"  #
                                               "${_FLAGS_COMPILE_1_}"  #
                                               "${_FLAGS_COMPILE_2_}") #


# # Setup: libraries # #

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_EXAMPLES NO CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    NO CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     NO CACHE BOOL "" FORCE)
add_subdirectory("deps/glfw")

# Setup: ImGui library #
set(IMGUI_SOURCE_DIR_ "${CMAKE_SOURCE_DIR}/deps/imgui")
set(IMGUI_BACKENDS_SOURCE_FILES_ "${IMGUI_SOURCE_DIR_}/backends/imgui_impl_glfw.cpp" # :2 api-dependend includes!
                                 "${IMGUI_SOURCE_DIR_}/backends/imgui_impl_opengl3.cpp")
file(GLOB IMGUI_SOURCE_FILES_    "${IMGUI_SOURCE_DIR_}/*.cpp")
add_library               (imgui STATIC ${IMGUI_SOURCE_FILES_} ${IMGUI_BACKENDS_SOURCE_FILES_})
target_include_directories(imgui SYSTEM PUBLIC  "${IMGUI_SOURCE_DIR_}"
                                        PRIVATE "${CMAKE_SOURCE_DIR}/deps/glfw/include")
target_compile_definitions(imgui        PRIVATE IMGUI_USER_CONFIG="${PROJECT_SOURCE_DIR}/src/conf/imconfig.h")
set_target_properties     (imgui PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)


# # Apply: project dependencies # #

target_link_libraries     (${PROJECT_NAME} OpenGL::GL glfw imgui)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
  "${CMAKE_SOURCE_DIR}/deps"
  "${CMAKE_SOURCE_DIR}/deps/rapidcsv/src"
) # rooted dirs will be treated as SYSTEM, even if opposite stated explicitly (with: -I)


# # Add: custom build targets # #

add_custom_target(${PROJECT_NAME}-clean
  COMMENT "Cleaning `${PROJECT_NAME}` target:"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} -DFILES_="${PROJECT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}.dir/src/"
                           -P "misc/cmake/remove_recurse.cmake"
)

add_custom_target(${PROJECT_NAME}-rebuild
  COMMENT "Rebuilding `${PROJECT_NAME}` target:"
  DEPENDS ${PROJECT_NAME}-clean
  COMMAND ${CMAKE_COMMAND} --build "." --target ${PROJECT_NAME}
)


###Stash###

#set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "" FORCE) # debug compiling

# debug file(GLOB_RECURSE)
#string (REPLACE ";" "\n" SOURCE_FILES_ "${SOURCE_FILES_}")
#message(FATAL_ERROR ${SOURCE_FILES_})

#cmake_path(GET CMAKE_CACHEFILE_DIR FILENAME PRESET_)
#message(${CMAKE_CACHEFILE_DIR})
#message(${PRESET_})
