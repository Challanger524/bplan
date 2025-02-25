// <util/wignore-pop.inl>
//   Restore compiler warnings from dirty includes
//   Mandatory to include opening <wignore-push.inl>


#ifndef  UTIL_WIGNORE_INL_GUARD
# error "include <util/wignore-push.inl> not paired"
#endif


#undef UTIL_WIGNORE_INL_GUARD


#if   defined(__GNUG__) || defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop // alias in Clang (but "#pragma clang diagnostic" will be ignored in GCC)
#elif defined(_MSC_VER)
#  pragma warning (pop)
#endif
