// <util/wignore-push.inl>
//   Disable compiler warnings from dirty includes
//   Mandatory to include enclosing <wignore-pop.inl>


#ifdef UTIL_WIGNORE_INL_GUARD
# error "include <util/wignore-pop.inl> not paired"
#endif


#define UTIL_WIGNORE_INL_GUARD


// Order matters: `__GNUC__` and `__GNUG__` defined on Clang too
#if   defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Weverything" // disables all warnings on Clang, but not all on GCC
#elif defined(__GNUG__) || defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wall"
#  pragma GCC diagnostic ignored "-Wextra"
#  pragma GCC diagnostic ignored "-Wpedantic"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#  pragma GCC diagnostic ignored "-Woverloaded-virtual"
#  pragma GCC diagnostic ignored "-Weffc++" // GCC only
#elif defined(_MSC_VER)
// "Ignoring warnings from system headers is supported with MSVC with the Ninja generators as of CMake 3.22"
// "and the Visual Studio generators as of CMake 3.24. Of course, an MSVC of at least (compiler) version 19.29.30036.3"
// "is required (the 14.29 toolchain in the Visual Studio installer)."
//  https://discourse.cmake.org/t/marking-headers-as-system-does-not-suppress-warnings-on-windows/6415/2?u=int_main
#  pragma warning (push)
/*
#  pragma warning (disable: 4091)*/
#endif
