#pragma once

#include "bplan/chrono.hpp"
#include "bplan/type_traits.hpp"

#include <locale>
#include <chrono>
#include <string>
#include <concepts>
#include <type_traits>
#include <assert.h>

namespace bplan {

// Overloaded template declaration for _PODs_ and other _Types_ with defined `operator <` (less)

template<class T> concept CanHold_utf8     = Base_type<T, std::string> || (Base_type<T, char> && is_cstr_v<T>); // can     contain/represent UTF-8
template<class T> concept CanHold_utf8_not = !CanHold_utf8<T>;                                                  // can not contain/represent UTF-8

template<CanHold_utf8_not T> inline short compare(const T &l, const T &r) { return l < r ? -1 : (r < l) ? +1 : 0; }

// Asserted (statically) template definition (not for instantiation)
template<class T> inline short compare       (const std::string & , const std::string & ) { static_assert(false, "custom template specialization required for given type"); return {}; }
// Specialization for `utf-8` string that respects global locale compare rules
template<> inline short compare<std:: string>(const std::string &l, const std::string &r) { return std::locale().operator()(l, r) ? -1 : std::locale().operator()(r, l) ? +1 : 0; }
template<> inline short compare<uint64_t    >(const std::string &l, const std::string &r) { return compare(std::stoull(l), std::stoull(r)); }
template<> inline short compare<ldouble     >(const std::string &l, const std::string &r) { return compare(std::stold (l), std::stold (r)); }

/** Overloaded template declaration for `std::chrono::<Date>` _calendar_ types
 * @tparam Date   accepted by `std::chrono::from_stream(,, Date{})`, can be `std::chrono::` `year` `month` `day` `year_month` and others
 * @param  format accepted by `std::chrono::from_stream(, format,)` */
template<class Date> short compare(const std::string &l, const std::string &r, auto fromat) {
	Date dl, dr;
	bp::chrono::from_stream(l, fromat, dl);
	bp::chrono::from_stream(r, fromat, dr);
	return compare(dl, dr);
}

} // namespace bplan


// Stash //

//template<class T> inline std::enable_if_t<!std::is_same_v<T, std::string>, short> compare(T l, T r) { return l == r ? 0 : (l < r) ? -1 : +1; }
//template<class T> inline std::enable_if_t<!std::is_same_v<T, std::string>, short> compare(const T &l, const T &r) { return l < r ? -1 : (r < l) ? +1 : 0; }
//template<class T> inline auto compare(const T &l, const T &r) -> std::enable_if_t<!std::is_same_v<strip_t<T>, std::string>, short> { return l < r ? -1 : (r < l) ? +1 : 0; }
//template<class T> inline short compare (const T &l, const T &r) requires (!CanHold_utf8<T>) { return l < r ? -1 : (r < l) ? +1 : 0; }
//template<class T> requires (!CanHold_utf8<T>) inline short compare (const T &l, const T &r) { return l < r ? -1 : (r < l) ? +1 : 0; }

// `std::string` that can hold `utf-8` encoding
//template<> inline short compare<std:: string>(const std::string &l, const std::string &r) { return std::locale().operator()(l, r) ? -1 : std::locale().operator()(r, l) ? +1 : 0; }
//template<> inline short compare<std:: string>(const std::string &l, const std::string &r) { return l == r ? 0 : std::locale().operator()(l, r) ? -1 : +1;  } //! bad == cmp
//template<> inline short compare<std:: string>(const std::string &l, const std::string &r) { return l == r ? 0 :  bp::locale  .operator()(l, r) ? -1 : +1;  }
//template<> inline short compare<std::wstring>(const std::string &l, const std::string &r) { return l == r ? 0 :      locale  .operator()(l, r) ? -1 : +1;  }
//template<> inline short compare<std:: string>(const std::string &l, const std::string &r) { return scast<short>(l.compare(r)); }
//template<> inline short compare<char        >(const std::string &l, const std::string &r) { return compare(l.empty() ? '\0' : l[0], r.empty() ? '\0' : r[0]); }
