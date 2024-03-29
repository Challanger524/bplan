cmake_minimum_required (VERSION 3.26)

set(CMAKE_VERBOSE_MAKEFILE OFF CACHE BOOL "" FORCE)

project (bplan  VERSION       0.1
                DESCRIPTION  "Budget Planner with ImGui"
                HOMEPAGE_URL "https://github.com/Challanger524/bplan"
                LANGUAGES     CXX)

# Setup main project
file(GLOB_RECURSE SOURCE_FILES_ CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/*.cpp"
)
## exclude files starting with uncderscore `_<file>`: https://regex101.com/r/DT6qGi/1
list(FILTER SOURCE_FILES_ EXCLUDE REGEX ".*\\/_.*\\..*")
#string (REPLACE ";" "\n" SOURCE_FILES_ "${SOURCE_FILES_}")
#message(FATAL_ERROR ${SOURCE_FILES_})

add_executable            (${PROJECT_NAME} ${SOURCE_FILES_})
target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_precompile_headers (${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src/config.hpp") # force include to all source files
set_target_properties     (${PROJECT_NAME} PROPERTIES
  CXX_STANDARD          23
  CXX_STANDARD_REQUIRED ON
  #RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}$<0:>" # deal with resource depsendencies
)

# Compile flags (provider: CMakePresets.json)
target_compile_options(${PROJECT_NAME} PRIVATE "$<$<BOOL:_FLAGS_COMPILE_0_>:${_FLAGS_COMPILE_0_}>")
target_compile_options(${PROJECT_NAME} PRIVATE        "${_FLAGS_COMPILE_1_};${_FLAGS_COMPILE_2_}")
# flags: mutual compiler & linker
target_compile_options(${PROJECT_NAME} PRIVATE "$<$<BOOL:_FLAGS_COMP_LINK_>:${_FLAGS_COMP_LINK_}>")
target_link_options   (${PROJECT_NAME} PRIVATE "$<$<BOOL:_FLAGS_COMP_LINK_>:${_FLAGS_COMP_LINK_}>")
# flags: linker
target_link_options   (${PROJECT_NAME} PRIVATE "$<$<BOOL:_FLAGS_LINKER_>:${_FLAGS_LINKER_}>")

# Setup libraries
find_package(OpenGL REQUIRED)

set(GLFW_BUILD_EXAMPLES NO CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    NO CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     NO CACHE BOOL "" FORCE)
add_subdirectory("deps/glfw")

## Setup ImGui library
set(IMGUI_SOURCE_DIR_ "${CMAKE_SOURCE_DIR}/deps/imgui")
set(IMGUI_BACKENDS_SOURCE_FILES_ "${IMGUI_SOURCE_DIR_}/backends/imgui_impl_glfw.cpp" # :2 api-dependend includes!
                                 "${IMGUI_SOURCE_DIR_}/backends/imgui_impl_opengl3.cpp")
file(GLOB IMGUI_SOURCE_FILES_    "${IMGUI_SOURCE_DIR_}/*.cpp")
add_library               (imgui STATIC ${IMGUI_SOURCE_FILES_} ${IMGUI_BACKENDS_SOURCE_FILES_})
target_include_directories(imgui SYSTEM PUBLIC  "${IMGUI_SOURCE_DIR_}"
                                        PRIVATE "${CMAKE_SOURCE_DIR}/deps/glfw/include")
set_target_properties     (imgui PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

# Apply project dependencies
target_link_libraries     (${PROJECT_NAME} OpenGL::GL glfw imgui)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
  "${CMAKE_SOURCE_DIR}/deps" # rooted dirs will be treated as SYSTEM, even if opposite stated explicitly (with: -I)
)
