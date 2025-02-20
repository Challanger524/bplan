#pragma once

#include <type_traits>

namespace bplan { // contains <type_traits> extensions

/** ## Content
 * Note: all structs has their appropriate `_t`/`_v` (`::type`/`::value`) `using`/`constexpr bool` aliases
 * struct remove_all_pointers<T>        - remove all pointers
 * struct remove_all_extntptr<T>        - remove all pointers extents
 * struct remove_all_refexptr<T>        - remove all pointers extents reference
 * struct               strip<T>        - strip from T all: references, pointers, subscripts, qualifier: `&` `*` `[]` `const` `volatile`
 * concept          Base_type<T, Base>  - strip `T` and compare it to `Base`
 * concept          Base_char<T>        - strip `T` and compare it to `char`
 * bool           is_same_any<T, Ts...> - is_same T with Ts... || any  can be same
 * bool           is_same_all<T, Ts...> - is_same T with Ts... && all must be same
 * struct          strip_cstr<T>        - return stripped to `char` or `char*` or `const char*`
 * struct             is_cstr<T>        - return true if T is `char*` or `const char*`
 */


// same as: `std::remove_all_extents<>`, but for sequence of `const`/`volatile`/`const volatile` pointers `*`
template<class T> struct remove_all_pointers : std::conditional_t<std::is_pointer_v<T>, remove_all_pointers<std::remove_pointer_t<T>>, std::type_identity<T>>{};
template<class T> using  remove_all_pointers_t = remove_all_pointers<T>::type; // same as: `std::remove_all_extents<>`, but for sequence of `const`/`volatile`/`const volatile` pointers `*`
// "standard c++11 way to remove all pointers of a type": https://stackoverflow.com/a/39492671/11753532        // (C++20, C++17)
// "Writing a remove_all_pointers type trait, part 2"   : https://devblogs.microsoft.com/oldnewthing/?p=109942 //


#if 1 // remove_all_extntptr (std::decay)
// Removes all arrays `[]` and pointers `*` (cv-q) declared in any order. Does nothing if T is `T&`
template<class T> struct remove_all_extntptr : std::conditional_t<!std::is_reference_v<T> && std::is_pointer_v<std::decay_t<T>>,
                         remove_all_extntptr<std::remove_pointer_t<std::decay_t<T>>>, std::type_identity<T>>{};
#else // remove_all_extntptr (std::remove_all_extents)
// Removes all arrays `[]` and pointers `*` (cv-q) declared in any order. Does nothing if T is `T&`
template<class T> struct remove_all_extntptr : std::conditional_t<!std::is_reference_v<T> && std::is_pointer_v<std::decay_t<T>>,
                         remove_all_extntptr<std::remove_pointer_t<std::remove_all_extents_t<T>>>, std::type_identity<T>>{};
#endif // remove_all_extntptr
template<class T> using  remove_all_extntptr_t = remove_all_extntptr<T>::type; // Removes all arrays `[]` and pointers `*` (cv-q) declared in any order. Does nothing if T is `T&`


// Removes ((topmost)) reference `&`/`&&`; Removes all arrays `[]` and all pointers `*` (with cv-q) declared in any order.
template<class T> struct remove_all_refexptr: std::conditional_t<std::is_pointer_v<std::decay_t<T>>,
                         remove_all_refexptr<std::remove_pointer_t<std::decay_t<T>>>, std::type_identity<std::remove_reference_t<T>>>{};
template<class T> using  remove_all_refexptr_t = remove_all_refexptr<T>::type; // Removes ((topmost)) reference `&`/`&&`; Removes all arrays `[]` and all pointers `*` (with cv-q) declared in any order.


#if 1  // strip: independent (STL)
// Strip all `[]` `*` `&` `&&` `const` `valotile` qualifiers down to the bottom-most type (like to: char, int, std::\<container>, ..)
template<class T> struct strip : std::conditional_t<std::is_pointer_v<std::decay_t<T>>,
      /* recursion--> */ strip<std::remove_pointer_t<std::decay_t<T>>>, std::type_identity<std::remove_cv_t<std::decay_t<T>>>>{};
#else  // strip: dependent (bplan::remove_all_extntptr<T>), oneline
// Strip all `[]` `*` `&` `&&` `const` `valotile` qualifiers down to the bottom-most type (like to: char, int, std::\<container>, ..)
template<class T> using  strip   = std::type_identity<std::remove_cv_t<remove_all_extntptr_t<std::remove_reference_t<T>>>>;
#endif // strip:
template<class T> using  strip_t = strip<T>::type; // Strip all `[]` `*` `&` `&&` `const` `valotile` qualifiers down to the bottom-most type (like to: char, int, std::\<container>, ..)


// c-string detection:
template<class T, class Base> concept Base_type = std::is_same_v<strip_t<T>, Base>; // can be called as: `template<class     T> requires(Base_type<int> T)`
template<class T>             concept Base_char = std::is_same_v<strip_t<T>, char>; // can be called as: `template<Base_char T>`

template<class T, class... Ts> inline constexpr bool is_same_any = (std::is_same_v<T, Ts> || ...);
template<class T, class... Ts> inline constexpr bool is_same_all = (std::is_same_v<T, Ts> && ...);


/// @return 0) `char` or 1) `char*` or 2) `const char*`
template<Base_char T> struct strip_cstr : std::conditional_t<
	std::is_same_v<char, std::remove_cv_t<T>>, std::type_identity<T>, std::conditional_t<
		is_same_any<std::remove_cv_t<std::remove_all_extents_t<std::remove_reference_t<T>>>, char*, const char*>,
		std::type_identity<std::remove_all_extents_t<T>>,
		strip_cstr<std::remove_pointer_t<std::remove_all_extents_t<std::remove_reference_t<T>>>>
	> // std::conditional_t<char* || const char*>
>{};  // std::conditional_t<char>
template<class T> using strip_cstr_t = strip_cstr<T>::type; /// @return `char*` `const char*` if such a base present, `char` otherwise


/// @return `std::true_type` on `char*` and `const char*` or `std::false_type`
template<Base_char T> struct is_cstr : std::conditional_t</*1*/ std::is_same_v<char, std::remove_cv_t<T>>, /*2*/ std::false_type, /*3*/ std::conditional_t<
		/*1*/ is_same_any<std::remove_cv_t<std::remove_all_extents_t<std::remove_reference_t<T>>>, char*, const char*>,
		/*2*/ std::true_type,
		/*3*/ is_cstr<std::remove_pointer_t<std::remove_all_extents_t<std::remove_reference_t<T>>>>
	> // second - std::conditional_t<char* || const char*>
>{};  // first  - std::conditional_t<char>
template<class T> inline constexpr bool is_cstr_v = is_cstr<T>::value; /// @return `true` on `char*` and `const char*`, `false` otherwise

} // namespace bplan


namespace bplan::test_static {

namespace remove_all_pointers { //  struct
static_assert(true
	&&  std::is_same_v<int, remove_all_pointers_t<int>>
	&&  std::is_same_v<int, remove_all_pointers_t<int *              >>
	&&  std::is_same_v<int, remove_all_pointers_t<int *const         >>
	&&  std::is_same_v<int, remove_all_pointers_t<int *volatile      >>
	&&  std::is_same_v<int, remove_all_pointers_t<int *volatile const>>
	&&  std::is_same_v<int, remove_all_pointers_t<int *const volatile>>
	&&  std::is_same_v<int, remove_all_pointers_t<int *****>>
	&&  std::is_same_v<int, remove_all_pointers_t<int *const  *volatile   *volatile const    *const volatile     *>>
	&&  std::is_same_v<int, remove_all_pointers_t<int *const **volatile ***volatile const ****const volatile *****>>
	&& !std::is_same_v<int, remove_all_pointers_t<const int  >>
	&& !std::is_same_v<int, remove_all_pointers_t<const int *>>
	&& !std::is_same_v<int, remove_all_pointers_t<const int *volatile>>
	&& !std::is_same_v<int, remove_all_pointers_t<const int * []>>
	&& !std::is_same_v<int, remove_all_pointers_t<const int(*)[]>>
);
} // namespace remove_all_pointers
namespace remove_all_extntptr { //  struct
static_assert(true
	&&  std::is_same_v<int, remove_all_extntptr_t<int>>
	&&  std::is_same_v<int, remove_all_extntptr_t<int *   >>
	&&  std::is_same_v<int, remove_all_extntptr_t<int   []>>
	&&  std::is_same_v<int, remove_all_extntptr_t<int * []>>
	&&  std::is_same_v<int, remove_all_extntptr_t<int(*)[]>>
	&&  std::is_same_v<int, remove_all_extntptr_t<int(*)[][2]>>
	&&  std::is_same_v<int, remove_all_extntptr_t<int(**)[][2]>>
	&&  std::is_same_v<int, remove_all_extntptr_t<int(****)[][2][3][4]>>
	&&  std::is_same_v<int, remove_all_extntptr_t<int *volatile(*const(*const volatile)[][1])[2]>> //cv*->[1]->c*->[2]->v*=>T

	&& !std::is_same_v<int, remove_all_extntptr_t<int &   >>
	&& !std::is_same_v<int, remove_all_extntptr_t<int*&   >>
	&& !std::is_same_v<int, remove_all_extntptr_t<int(&)[]>>
	&& !std::is_same_v<int, remove_all_extntptr_t<int *volatile(*const(*const volatile &)[][1])[2]>> // &->cv*->[1]->c*->[2]->v*=>T

	&& !std::is_same_v<int, remove_all_extntptr_t<const    int>>
	&& !std::is_same_v<int, remove_all_extntptr_t<volatile int>>
	&& !std::is_same_v<int, remove_all_extntptr_t<volatile int *>>
	&& !std::is_same_v<int, remove_all_extntptr_t<const int *volatile(*const(*const volatile &)[][1])[2]>> // &->cv*->[1]->c*->[2]->v*=>T
);
} // namespace remove_all_extntptr
namespace remove_all_refexptr { //  struct
static_assert(true
	&&  std::is_same_v<int, remove_all_refexptr_t<int>>
	&&  std::is_same_v<int, remove_all_refexptr_t<int *   >>
	&&  std::is_same_v<int, remove_all_refexptr_t<int   []>>
	&&  std::is_same_v<int, remove_all_refexptr_t<int * []>>
	&&  std::is_same_v<int, remove_all_refexptr_t<int(*)[]>>
	&&  std::is_same_v<int, remove_all_refexptr_t<int(*)[][2]>>
	&&  std::is_same_v<int, remove_all_refexptr_t<int(**)[][2]>>
	&&  std::is_same_v<int, remove_all_refexptr_t<int(****)[][2][3][4]>>
	&&  std::is_same_v<int, remove_all_refexptr_t<int *volatile(*const(*const volatile)[][1])[2]>> // cv*->[1]->c*->[2]->v*=>T

	&&  std::is_same_v<int, remove_all_refexptr_t<int &   >>
	&&  std::is_same_v<int, remove_all_refexptr_t<int*&   >>
	&&  std::is_same_v<int, remove_all_refexptr_t<int(&)[]>>
	&&  std::is_same_v<int, remove_all_refexptr_t<int *volatile(*const(*const volatile &)[][1])[2]>> // &->cv*->[1]->c*->[2]->v*=>T

	&& !std::is_same_v<int, remove_all_refexptr_t<const    int>>
	&& !std::is_same_v<int, remove_all_refexptr_t<volatile int>>
	&& !std::is_same_v<int, remove_all_refexptr_t<volatile int *>>
	&& !std::is_same_v<int, remove_all_refexptr_t<const    int *volatile(*const(*const volatile &)[][1])[2]>> // &->cv*->[1]->c*->[2]->v*=>T
);
} // namespace remove_all_refexptr
namespace strip               { //  struct
static_assert(true
	&&  std::is_same_v<int, strip_t<int>>
	&&  std::is_same_v<int, strip_t<int *   >>
	&&  std::is_same_v<int, strip_t<int   []>>
	&&  std::is_same_v<int, strip_t<int * []>>
	&&  std::is_same_v<int, strip_t<int(*)[]>>
	&&  std::is_same_v<int, strip_t<int(*)[][2]>>
	&&  std::is_same_v<int, strip_t<int(**)[][2]>>
	&&  std::is_same_v<int, strip_t<int(****)[][2][3][4]>>
	&&  std::is_same_v<int, strip_t<int *volatile(*const(*const volatile)[][1])[2]>>         //    cv*->[1]->c*->[2]->v*=>T

	&&  std::is_same_v<int, strip_t<int &   >>
	&&  std::is_same_v<int, strip_t<int*&   >>
	&&  std::is_same_v<int, strip_t<int(&)[]>>
	&&  std::is_same_v<int, strip_t<int *volatile(*const(*const volatile &)[][1])[2]>>       // &->cv*->[1]->c*->[2]->v*=>T

	&&  std::is_same_v<int, strip_t<const    int>>
	&&  std::is_same_v<int, strip_t<volatile int>>
	&&  std::is_same_v<int, strip_t<volatile int *>>
	&&  std::is_same_v<int, strip_t<const    int *volatile(*const(*const volatile &)[][1])[2]>> // &->cv*->[1]->c*->[2]->v*=>сT

	&&  std::is_same_v<int, strip_t<const          int ***volatile(**const(*const volatile &)[][1])[2][3][4]>>
	&&  std::is_same_v<int, strip_t<volatile       int ***volatile(**const(*const volatile &)[][1])[2][3][4]>>
	&&  std::is_same_v<int, strip_t<volatile const int ***volatile(**const(*const volatile &)[][1])[2][3][4]>>
	&&  std::is_same_v<int, strip_t<const volatile int ***volatile(**const(*const volatile &)[][1])[2][3][4]>>
);
} // namespace strip
namespace base_type           { // concept
static_assert(true
	&&  Base_type<               int                                                      , int>
	&&  Base_type<               int   *volatile( *const(*const volatile)  [][1])[2]      , int>
	&&  Base_type<const          int   *volatile( *const(*const volatile &)[][1])[2]      , int>
	&&  Base_type<const volatile int ***volatile(**const(*const volatile &)[][1])[2][3][4], int>
	&& !Base_type<int*[], short>
);
} // base_type
namespace base_char           { // concept
static_assert(true
	&&  Base_char<               char                                                      >
	&&  Base_char<               char   *volatile( *const(*const volatile)  [][1])[2]      >
	&&  Base_char<const          char   *volatile( *const(*const volatile &)[][1])[2]      >
	&&  Base_char<const volatile char ***volatile(**const(*const volatile &)[][1])[2][3][4]>
	&& !Base_char<int*[]>
);
} // base_char
namespace is_any_variadic     { //    bool
static_assert(true
	&&  is_same_any<int, long long, long, char, short, int >
	&& !is_same_any<int, long long, long, char, short, int*>
	&&  is_same_any<int, long long, int , long, char, short>
	&& !is_same_any<int, long long, int*, long, char, short>

	&&  is_same_all<int, int, int, int, int, int>
	&& !is_same_all<int, int*, int , int , int , int >
	&& !is_same_all<int, int , int*, int , int , int >
	&& !is_same_all<int, int , int , int*, int , int >
	&& !is_same_all<int, int , int , int , int*, int >
	&& !is_same_all<int, int , int , int , int , int*>
);
} // is_any_variadic
namespace strip_cstr          { //  struct
static_assert(true
	&&  std::is_same_v<char, strip_cstr_t<char  >>
	&&  std::is_same_v<char, strip_cstr_t<char[]>>
	&&  std::is_same_v<char*, strip_cstr_t<char*  >>
	&&  std::is_same_v<char*, strip_cstr_t<char*[]>>
	&&  std::is_same_v<const char*, strip_cstr_t<const char*  >>
	&&  std::is_same_v<const char*, strip_cstr_t<const char*[]>>
	&& !std::is_same_v<const char*, strip_cstr_t<const char(*)[]>>
);
} // namespace strip_cstr
namespace is_cstr             { //  struct
static_assert(true
	&&  is_cstr_v<      char *>
	&&  is_cstr_v<const char *>
	&&  is_cstr_v<      char *&>
	&&  is_cstr_v<const char *&>

	&& !is_cstr_v<         char  >
	&& !is_cstr_v<         char[]>
	&& !is_cstr_v<volatile char* >

	&&  is_cstr_v<      char   *        (*const(*const volatile  )[][1])[2]>
	&&  is_cstr_v<      char   *const   (*const(*const volatile  )[][1])[2]>
	&&  is_cstr_v<const char   *volatile(*const(*const volatile &)[][1])[2]>
	&&  is_cstr_v<const char   *volatile(*const(*const volatile &)[][1])[2]>

	&& !is_cstr_v<const char            (*const(*const volatile &)[][1])[2]>
);
} // is_cstr


#if 0 // Test type traits

#  if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wmissing-declarations"
#elif defined(__GNUG__) || defined(__GNUC__)
#  pragma GCC diagnostic push
//#  pragma GCC diagnostic ignored "-fpermissive" // Cannot be disabled through pragma nor through GCC compile options, Fuu GCC
#elif defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable: 4091)
#endif


//#define TYPE int[][2]
//#define TYPE const int *
//#define TYPE const int &
//#define TYPE int(&)[]
//#define TYPE int[1][2][3]
//#define TYPE std::string**
//#define TYPE const int* const
//#define TYPE int(*(*(&)[1])[2])
//#define TYPE int(*(*(&)[1])[2])
#define TYPE const int*[1][2][3]
//#define TYPE const int*const*const*const&
//#define TYPE int(*const)[1]
//using   TYPE_TEST = TYPE; // corn: cvT(*cv*cv...*cv)& ?[]
//using   TYPE = int(*)[];  // corn: cvT(*cv*cv...*cv)& ?[]

std::remove_extent_t     <TYPE>; // type of a                  array                    //         T[1][2]...[n]->T[2]...[n]                              | T* , T(*)[], T(&)[]
std::remove_extent_t     <TYPE>; // type of a                  array                    //         T[1][2]...[n]->T[2]...[n]                              | T* , T(*)[], T(&)[]
std::remove_all_extents_t<TYPE>; // type of a multidimensional array                    //         T[1][2]...[n]->T                                       | T* , T(*)[], T(&)[]
std::       decay_t      <TYPE>; // equivalent to the passed function argument by value // cT&->T, T[]->T*, T[1][2]->T*[2], T(&)[]->T* F()->F*_t          | cT*, rm_cvref+([]->*)
std::remove_cvref_t      <TYPE>; // removes reference and topmost cv-qualifiers         // cT&->T, cT->T, T&/T&&->T, cT(&)[]->T[], T*c&->T*, T*c*c&->T*c* | cT*,
std::remove_pointer_t    <TYPE>; // removes pointer with it's cv-qualifiers             // T*c->T, T*->T
std::remove_cvref_t  <std::remove_pointer_t<TYPE>>;
std::remove_pointer_t<std::decay_t         <TYPE>>;
remove_all_pointers_t    <TYPE>;
remove_all_pointers_t<std::remove_reference_t<TYPE>>;
remove_all_extntptr_t    <TYPE>;
remove_all_refexptr_t    <TYPE>;
              strip_t    <TYPE>;


#define TEST_TYPEDEF const int *volatile(*const(*const volatile)[1])[2] // cv*->[1]->с*->[2]->v*=>cT
std::decay_t<std::remove_pointer_t<std::remove_pointer_t<std::decay_t<std::remove_pointer_t<TEST_TYPEDEF>>>>>;
#define TEST_RM_ALL_EXTENTPTR const char&
remove_all_extntptr_t    <TEST_RM_ALL_EXTENTPTR>;

strip_t<std::decay_t     <TEST_RM_ALL_EXTENTPTR>>;
strip_t                  <TEST_RM_ALL_EXTENTPTR>;


#  if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUG__) || defined(__GNUC__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning (pop)
#endif

#endif // Test type traits

} // namespace bplan::test_static

#if 0 // overtesting::arr_ptr_ptr_arr
namespace overtesting::arr_ptr_ptr_arr {
// mine self-answer question: https://stackoverflow.com/questions/79436438/c-define-a-type-alias-pointer-to-array-with-keyword-using-not-typedef

// Option 0: T(*)[]
using         ptr_arr_using = int(*)[]; // pointer to array
typedef int (*ptr_arr_typedef)[];       // pointer to array
typedef int  *arr_ptr_typedef [];       // array   of pointers
using         arr_ptr_using = int * []; // array   of pointers

static_assert( std::is_same_v<ptr_arr_using, ptr_arr_typedef>); // pointer to array
static_assert( std::is_same_v<arr_ptr_using, arr_ptr_typedef>); // array   of pointers

static_assert(!std::is_same_v<arr_ptr_using, ptr_arr_typedef>);
static_assert(!std::is_same_v<ptr_arr_using, arr_ptr_typedef>);

static_assert(!std::is_same_v<arr_ptr_using, ptr_arr_using  >); // arr_ptr != ptr_arr
static_assert(!std::is_same_v<ptr_arr_typedef, arr_ptr_typedef>);
// IDE suggestion will help to render the resulting difference

// "The Clockwise/Spiral Rule" by David Anderson: https://c-faq.com/decl/spiral.anderson.html
// Will explain how to "parse complex declarations in your head"

// Option 1: alias template // https://en.cppreference.com/w/cpp/language/type_alias
template<class T> using ptr_t = T*; using ptr_arr_using_template = ptr_t<int[]>;

// Option 2: alias alias // no IDE hint for `ptr_arr` full type
using arr = int[]; using ptr_arr = arr*;

//const int *volatile(*const(*const volatile)[1])[2] // cv*->[1]->с*->[2]->v*=>cT // T=int

} // namespace overtesting::arr_ptr_ptr_arr
#endif // overtesting::arr_ptr_ptr_arr


// Stash //

// non-inheriting analog // https://devblogs.microsoft.com/oldnewthing/?p=109942
//template<class T> struct remove_all_pointers_2 { using type = typename std::conditional_t<std::is_pointer_v<T>, remove_all_pointers<std::remove_pointer_t<T>>, std::type_identity<T>>::type; };

//template<class T> struct remove_all_extntptr : std::conditional_t<std::is_pointer_v<T>, remove_all_extntptr<std::decay_t<std::remove_pointer_t<T>>>, std::type_identity<T>>{};

//template <Base_char T> using strip_cstr_i = strip_cstr<T>;      // experimental: attempt to reduce concept ceck from recursive to single
//template <Base_type<char> T> using strip_cstr_i = strip_cstr<T>;   // experimental: attempt to reduce concept ceck from recursive to single (generic)
//template <class T> using strip_cstr_it = strip_cstr_i<T>::type; // experimental:
//strip_cstr_i <TEST_STRIP_CSTR>::type;

#if 0  // macro-comment
#if 0 // is_cstr: independent (STL)
/// @return `std::true_type` on `char*` and `const char*` or `std::false_type`
template<Base_char T> struct is_cstr : std::conditional_t<
	std::is_same_v<char, std::remove_cv_t<T>>, std::false_type, std::conditional_t<
			std::is_same_v<      char*, std::remove_cv_t<std::remove_all_extents_t<std::remove_reference_t<T>>>> ||
			std::is_same_v<const char*, std::remove_cv_t<std::remove_all_extents_t<std::remove_reference_t<T>>>>,
		std::true_type, is_cstr<std::remove_pointer_t<std::remove_all_extents_t<std::remove_reference_t<T>>>>
	>    // std::conditional_t<char* || const char*> - second
>{}; // std::conditional_t<char> - first
#elif 0 // is_cstr with personalized helper
template<class T> inline constexpr bool is_any_cstr = std::is_same_v<std::remove_cv_t<std::remove_all_extents_t<std::remove_reference_t<T>>>,       char*> ||
                                                      std::is_same_v<std::remove_cv_t<std::remove_all_extents_t<std::remove_reference_t<T>>>, const char*>;

/// @return `std::true_type` on `char*` and `const char*` or `std::false_type`
template<Base_char T> struct is_cstr : std::conditional_t<std::is_same_v<char, std::remove_cv_t<T>>, std::false_type,
	std::conditional_t<is_any_cstr<T>, std::true_type, is_cstr<std::remove_pointer_t<std::remove_all_extents_t<std::remove_reference_t<T>>>>
	>    // std::conditional_t<char* || const char*> - second
>{}; // std::conditional_t<char> - first
#elif 1 // is_cstr: with generalized helper (currently in use)
template<class T, class... Ts> inline constexpr bool is_same_any = (std::is_same_v<T, Ts> || ...);

/// @return `std::true_type` on `char*` and `const char*` or `std::false_type`
template<Base_char T> struct is_cstr : std::conditional_t</*1*/ std::is_same_v<char, std::remove_cv_t<T>>, /*2*/ std::false_type, /*3*/ std::conditional_t<
		/*1*/ is_same_any<std::remove_cv_t<std::remove_all_extents_t<std::remove_reference_t<T>>>, char*, const char*>,
		/*2*/ std::true_type,
		/*3*/ is_cstr<std::remove_pointer_t<std::remove_all_extents_t<std::remove_reference_t<T>>>>
	> // second - std::conditional_t<char* || const char*>
>{};  // first  - std::conditional_t<char>
#endif
#endif // macro-comment
