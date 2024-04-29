cmake_minimum_required (VERSION 3.26)

project (bplan  VERSION       0.1
                DESCRIPTION  "Budget Planner for Hromadas"
                HOMEPAGE_URL "https://github.com/Challanger524/bplan"
                LANGUAGES     CXX)

# # # Setup: main project # # #
add_executable            (${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_precompile_headers (${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src/config.hpp") # force include to all source files
set_target_properties     (${PROJECT_NAME} PROPERTIES
  CXX_STANDARD          23
  CXX_STANDARD_REQUIRED ON
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}$<0:>" # deal with resource depsendencies
  EXPORT_COMPILE_COMMANDS   ON
)

# # Apply: compile definitions # #
target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<BOOL:_DEFINITIONS_>:${_DEFINITIONS_}>")

# # Gather: sources # #
file(GLOB_RECURSE SOURCE_FILES_ CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")
list(FILTER SOURCE_FILES_ EXCLUDE REGEX ".*\\/_.*\\..*") # exclude files starting with uncderscore `_<file>`: https://regex101.com/r/DT6qGi/1
target_sources        (${PROJECT_NAME} PRIVATE ${SOURCE_FILES_})

# # Apply: compiler flags (from: CMakePresets.json) # #
target_compile_options(${PROJECT_NAME} PRIVATE "$<$<BOOL:_FLAGS_COMPILE_0_>:${_FLAGS_COMPILE_0_}>")
target_compile_options(${PROJECT_NAME} PRIVATE        "${_FLAGS_COMPILE_1_};${_FLAGS_COMPILE_2_}")
# flags: compiler & linker (mutual)
target_compile_options(${PROJECT_NAME} PRIVATE "$<$<BOOL:_FLAGS_COMP_LINK_>:${_FLAGS_COMP_LINK_}>")
target_link_options   (${PROJECT_NAME} PRIVATE "$<$<BOOL:_FLAGS_COMP_LINK_>:${_FLAGS_COMP_LINK_}>")
# flags: linker
target_link_options   (${PROJECT_NAME} PRIVATE "$<$<BOOL:_FLAGS_LINKER_>:${_FLAGS_LINKER_}>")

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
set_target_properties     (imgui PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

# Setup: libconfig #
# ehh... This command applies <patch> with `git`. Git fails if patch "already applied", so 2 commands are needed.
execute_process(
  COMMAND git apply "misc/patch/libconfig_wincompat_mingw.patch"            # if patch already applied - will fail
  COMMAND git apply "misc/patch/libconfig_wincompat_mingw.patch" --check -R # if succeeds - patch sucessfully applied - conf OK
  WORKING_DIRECTORY      "${CMAKE_SOURCE_DIR}"                              #    oterwise - patch is not applied - conf FAIL
  COMMAND_ERROR_IS_FATAL LAST   # helps to avoid configure fail on false-error when patch is already applied
  COMMAND_ECHO           STDOUT # express patching presence                           (optional)
  ERROR_QUIET                   # first command false-errors if patch already applied (optional) but real issue with 1st cmd..
  TIMEOUT                5      #                                                     (optional) ..can be missed this way
) #!workaround

  set(LIBCONFIG_    "config++")
if(MSVC)
  set(LIBCONFIG_ "libconfig++")
endif()
set(BUILD_SHARED_LIBS NO CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES    NO CACHE BOOL "" FORCE)
set(BUILD_TESTS       NO CACHE BOOL "" FORCE)
add_subdirectory("deps/libconfig")
target_compile_definitions(${LIBCONFIG_} PRIVATE LIBCONFIG_STATIC) #!workaround

# # Apply: project dependencies # #
target_link_libraries     (${PROJECT_NAME} OpenGL::GL glfw imgui ${LIBCONFIG_})
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
  "${CMAKE_SOURCE_DIR}/deps" # rooted dirs will be treated as SYSTEM, even if opposite stated explicitly (with: -I)
)


###Stash###

#set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "" FORCE) # debug compiling

# debug file(GLOB_RECURSE)
#string (REPLACE ";" "\n" SOURCE_FILES_ "${SOURCE_FILES_}")
#message(FATAL_ERROR ${SOURCE_FILES_})
