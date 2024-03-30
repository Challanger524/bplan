#pragma once

/** Project's main configuration header
 *  Should be included via CMakeLists.txt:`target_precompile_headers()`
 *	**VSCode**
 *		Intellisense hint option should be present in `.vscode/settings.json`
 *	    Othervise manually add work-/userspace setting:
 *  	`"C_Cpp.default.forcedInclude": ["${workspaceFolder}/src/config.hpp"]`
 */

#ifndef BPLAN_CONFIG_PCH
#define BPLAN_CONFIG_PCH // `config.hpp` presence marking
#endif

#define IMGUI_DEFINE_MATH_OPERATORS // Enable operator overloads for ImGui (like: `ImVec2`)

#include <stdint.h>
using ushort = unsigned short;
using ulong  = unsigned long long;
using uchar  = unsigned char;
using uint   = unsigned int;

//#define ccast       const_cast
  #define scast      static_cast
//#define dcast     dynamic_cast
//#define rcast reinterpret_cast
