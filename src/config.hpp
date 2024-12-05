#pragma once

/** Project's main configuration header
 *  Should be included via CMakeLists.txt:`target_precompile_headers()`
 *	**VSCode**
 *		Intellisense hint option should be present in `.vscode/settings.json`
 *	    Othervise manually add work-/userspace setting:
 *  	`"C_Cpp.default.forcedInclude": ["${workspaceFolder}/src/config.hpp"]`
 */

#include <conf/imconfig.h>

#ifndef BPLAN_CONFIG_PCH
#define BPLAN_CONFIG_PCH // `config.hpp` presence mark
#endif

#if (!defined(NDEBUG) || defined(TESTS_IN_APP)) && !defined(NO_TESTS_IN_APP)
#  ifndef TESTING
#  define TESTING
#  endif
#endif

#ifndef NDEBUG
#define ONDEBUG(src_code) { src_code; }
#else
#define ONDEBUG(src_code)
#endif

//------------------------ Macro functions ------------------------------------

#define  STR(X) #X     // stringizie the `X` (macro) _identifier_
#define MSTR(E) STR(E) // stringizie the `E`  macro  _expression_

#ifndef NDEBUG
#define ONDEBUG(E) { E; } // `E` exression on debug -  enabled
#else
#define ONDEBUG(E)        // `E` exression on debug - disabled
#endif

// Compiler-friendly debug breakpoints (in source code)
// https://stackoverflow.com/questions/173618/is-there-a-portable-equivalent-to-debugbreak-debugbreak/49079078#49079078
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2514r0.html#_implementation_experience
#ifndef _DEBUG
#  define BREAKPOINT()
#endif
#if !defined(BREAKPOINT) && __has_include(<intrin.h>)
#  include <intrin.h>
#  define BREAKPOINT() __debugbreak()
#elif defined(__has_builtin) && !defined(__ibmxl__) // clang/gcc
#  if __has_builtin(__builtin_debugtrap) // clang
#    define BREAKPOINT() __builtin_debugtrap()
//#  elif __has_builtin(__builtin_trap) // gcc
//#    define BREAKPOINT() __builtin_trap() // not much useful
#  endif
#endif
#if !defined(BREAKPOINT) && __has_include(<signal.h>)
#  include <signal.h>
#  if defined(SIGTRAP)
#    define BREAKPOINT() raise(SIGTRAP)
#  else
#    define BREAKPOINT() raise(SIGABRT)
#  endif
#endif

#define ASSERT(condition) if (!(condition)) BREAKPOINT() // Trigger debug breakpoint on `condition` fail


//------------------------ Options and modifications --------------------------

//#define MOD_CHAR8_T // enable `char8_t` stream `operator<<` overloads

#define LIB_LIBCONFIG

namespace      ImGui {} // `Dear ImGui` namespace pre-declaration (to alias it in the next line)
namespace im = ImGui;   // global alias for `ImGui::` namespace

namespace       bplan {} // `Budged Planner` namespace pre-declaration (to alias it in the next line)
namespace bp  = bplan;   // global alias for `bplan::` "Budged Planner" namespace
//using namespace bplan;

#include <stdint.h>
/*
using doublle =         long double;*/
using ldouble =         long double;
using ushort = unsigned short;
using ulong  = unsigned long long;
using uchar  = unsigned char;
using uint   = unsigned int;

using money_t = long double; // should be replaced with `moneycpp` - https://github.com/mariusbancila/moneycpp

//#define ccast       const_cast
  #define scast      static_cast
//#define dcast     dynamic_cast
//#define rcast reinterpret_cast

#ifdef _MSC_VER
constexpr size_t operator ""uz(unsigned long long n) { return n; }
#endif

#if defined(MOD_CHAR8_T) && !defined(NO_OSTREAM_CHAR_8T_TO_CHAR)
#include <iostream>
#include <string>
inline std::ostream &operator<<(std::ostream &os,         char8_t      v) { return os <<      static_cast<      char  >(v        ); }
inline std::ostream &operator<<(std::ostream &os, const   char8_t     *v) { return os << reinterpret_cast<const char *>(v        ); }
inline std::ostream &operator<<(std::ostream &os, const std::u8string &v) { return os << reinterpret_cast<const char *>(v.c_str()); }
#endif
