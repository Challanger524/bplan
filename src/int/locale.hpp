#pragma once

#include <locale>

namespace bplan
{

constexpr auto LOCALE_UA = "uk_UA.utf8"; // @currency=UAH;calendar=gregorian;collation=;numbers=

class no_separator : public std::numpunct<char> // fix broken stream output after thousands separator appearance from locale
{
	const std::numpunct<char> &np; // numpunct

protected:
	string_type do_truename     () const override { return np.truename (); }
	string_type do_falsename    () const override { return np.falsename(); }
	string_type do_grouping     () const override { return "\0"; } // disable "thousands separation" // originally: "\3"
	  char_type do_thousands_sep() const override { return  ' '; } // fix "non-breaking spaces (U+00A0) as thousands separator" // actually very (output) breaking, lol
	  char_type do_decimal_point() const override { return np.decimal_point(); }

public:
	no_separator(const std::locale& base) : np(std::use_facet<std::numpunct<char>>(base)) {}
};

//inline std::locale locale;

}
