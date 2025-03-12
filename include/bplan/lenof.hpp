#pragma once

#if 0 // Op 1: template[N] // cannot process pointers like: `constexpr const char *cstr = "123";`
#include "bplan/type_traits.hpp"

namespace bplan {

template<class T> concept ConstChar = bplan::is_same_any<T, const char, const char8_t, const char16_t, const char32_t, const wchar_t>;
//template<class T> concept ConstChar = bplan::is_same_any<std::remove_cv_t<T>, char, char8_t, char16_t, char32_t, wchar_t>;

}

template<bplan::ConstChar CChar, size_t N> inline constexpr size_t lenof (CChar (&)[N]) { return N - 1; }
//template<bplan::ConstChar CChar, size_t N> inline constexpr size_t lenof1(CChar (&)[N]) { return N    ; } // lenof() + 1; user must be sure that ending '\0' is present

#else

# include <string>
# if 1 // Op 2: constexpr specialized
inline constexpr size_t lenof(const  char    * s) { return std::char_traits< char    >::length(s); } // length of c-string
inline constexpr size_t lenof(const wchar_t  * s) { return std::char_traits<wchar_t  >::length(s); } // length of c-string
inline constexpr size_t lenof(const  char8_t * s) { return std::char_traits< char8_t >::length(s); } // length of c-string
inline constexpr size_t lenof(const  char16_t* s) { return std::char_traits< char16_t>::length(s); } // length of c-string
inline constexpr size_t lenof(const  char32_t* s) { return std::char_traits< char32_t>::length(s); } // length of c-string

# else // Op 3: constexpr auto
# include <type_traits>
constexpr size_t lenof(const auto* s) {
	static_assert(std::is_pointer_v<decltype(s)>);
	return std::char_traits<std::remove_cv_t<std::remove_pointer_t<decltype(s)>>>::length(s);
}

# endif // Op 2-3: constexpr specialized/auto

//inline constexpr size_t lenof1(auto s) { return lenof(s) + 1u; } // lenof() + 1; user must be sure that ending '\0' is present

#endif // Op 1: template[N]


namespace test_static::lenof_hpp {

#define             TEST_STATIC_LENOF_HPP "123"
static_assert(lenof(TEST_STATIC_LENOF_HPP) == 3);

inline constexpr const char *cstr = "123";
static_assert(lenof(  cstr  ) == 3);

static_assert(lenof(     "" ) == 0); // char (empty string)
static_assert(lenof(  "123" ) == 3); // char
static_assert(lenof(R"(123)") == 3);
static_assert(lenof(u8"123" ) == 3); // char8_t
static_assert(lenof( u"123" ) == 3); // char16_t
static_assert(lenof( U"123" ) == 3); // char32_t
static_assert(lenof( L"123" ) == 3); // wchar_t

static_assert(lenof(  "猫") >  1); // utf-8 (size 3)
//static_assert(lenof(  "Ї" ) != 1); // utf-8
//static_assert(lenof(  "Ї" ) == 2); // utf-8
//static_assert(lenof(u8"Ї" ) == 2); // utf-8

//static_assert(lenof1(  "123") == 4); // char length + (possible) '\0'

} // test_static::test_lenof


