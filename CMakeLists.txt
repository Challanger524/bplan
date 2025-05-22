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

# Setup: SQLite ORM #
set(SQLITE_ORM_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_SOURCE_DIR}/deps/sqlite_orm") # depends on 'SQLite' that must be visible to `find_package()`
# https://discourse.cmake.org/t/correct-usage-of-findsqlite3-module-in-windows/174

# Setup: Boost:: #
if(NOT EXISTS "${_BOOST_SOURCE_DIR_}/boost.png")
  message(FATAL_ERROR
    "_BOOST_SOURCE_DIR_='${_BOOST_SOURCE_DIR_}' - invalid boost path\n"
    "-D_BOOST_SOURCE_DIR_=<boost_path> define must be passed to the cmake command")
endif()

list(APPEND BOOST_INCLUDE_LIBRARIES config beast)

# ::beast # file:///./src/lib/src_beast.cpp
target_compile_definitions(${PROJECT_NAME} PRIVATE "BOOST_BEAST_SEPARATE_COMPILATION")

# ::locale
if (MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(BOOST_LOCALE_ENABLE_STD    OFF CACHE BOOL "" FORCE)
  set(BOOST_LOCALE_ENABLE_WINAPI YES CACHE BOOL "" FORCE)
  list(APPEND BOOST_INCLUDE_LIBRARIES locale)
  target_link_libraries(${PROJECT_NAME} Boost::locale)
endif()

add_subdirectory("${_BOOST_SOURCE_DIR_}" "${CMAKE_BINARY_DIR}/deps/boost" EXCLUDE_FROM_ALL)

# [Workaround] https://github.com/boostorg/beast/issues/672#issuecomment-318225620
# Link missing: `ws2_32` library for windows sockets with boost::asio
if (MINGW)
  target_link_libraries(boost_asio INTERFACE ws2_32) # mswsock wsock32
endif()

# # Apply: project dependencies # #

target_link_libraries     (${PROJECT_NAME} OpenGL::GL glfw imgui sqlite_orm::sqlite_orm Boost::beast)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
  "${CMAKE_SOURCE_DIR}/deps"
  "${CMAKE_SOURCE_DIR}/deps/rapidcsv/src"
  "${CMAKE_SOURCE_DIR}/deps/sqlite_orm/include/sqlite_orm"
) # rooted dirs will be treated as SYSTEM, even if opposite stated explicitly (with: -I)


# # Add: custom build targets # #

add_custom_target( ${PROJECT_NAME}-clean
  COMMENT "Target `${PROJECT_NAME}` - clean"
  COMMAND ${CMAKE_COMMAND} -E rm -rf -- "${PROJECT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}.dir/src/"
)


###Stash###

#set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "" FORCE) # debug compiling

# debug file(GLOB_RECURSE)
#string (REPLACE ";" "\n" SOURCE_FILES_ "${SOURCE_FILES_}")
#message(FATAL_ERROR ${SOURCE_FILES_})

#cmake_path(GET CMAKE_CACHEFILE_DIR FILENAME PRESET_)
#message(${CMAKE_CACHEFILE_DIR})
#message(${PRESET_})

#[[
add_custom_target( ${PROJECT_NAME}-rebuild
  COMMENT "Target `${PROJECT_NAME}` - rebuild"
  DEPENDS ${PROJECT_NAME}-clean
  COMMAND ${CMAKE_COMMAND} --build "." --target ${PROJECT_NAME}
  USES_TERMINAL # enables instant console output
)#]]

#[[ Same as `-clean`, but uses cmake script and can accept list of files/folders
add_custom_target( ${PROJECT_NAME}-clean-batch
  COMMENT "Target `${PROJECT_NAME}` - clean-batch:"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} -DFILES_="${PROJECT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}.dir/src/"
                           -P "misc/cmake/remove_recurse.cmake"
)#]]
