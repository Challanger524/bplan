#include "test/csv/CsvGet.hpp"

#include "UA/budget.hpp"
#include "UA/budget/api.hpp"
#include "UA/budget/general/incomes.hpp"
#include "UA/budget/local/incomes.hpp"
#include "UA/budget/local/expenses/program.hpp"
#include "int/budget/rapidcsv/imgui/imguiDrawEmptyTable.hpp"
#include "bplan/filesystem.hpp"
#include "bplan/boost/net.hpp"
#include "bplan/chrono.hpp"
#include "bplan/ctype.hpp"
#include "bplan/lenof.hpp"

#include <imgui.h>
#include <rapidcsv.h>

#include <span>
#include <array>
#include <string>
#include <string_view>
#include <sstream>
#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <expected>
#include <stdexcept>
#include <filesystem>
#include <assert.h>

#include <util/wignore-push.inl>
#  include <boost/asio/ip/tcp.hpp>
#  include <boost/asio/connect.hpp>
#  include <boost/beast/core.hpp>
#  include <boost/beast/http.hpp>
#  include <boost/beast/version.hpp>
#include <util/wignore-pop.inl>


namespace bplan {
#ifndef __clang__
extern const std::chrono::time_zone *timezone;
#endif
}


namespace test {


//namespace inc   = UA::budget::incomes;
namespace rcv   = rapidcsv    ;
namespace asio  = boost::asio ;
namespace beast = boost::beast;

namespace fs  = std::filesystem;

using namespace UA::budget;
using namespace std::literals::chrono_literals;
using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

std::expected<void, std::string> CsvDownload(const fs::path &csvPath, const std::string &csvQuery, std::string &feedback);
void DrawCsvTableGenIncS (const rcv::Document &csv); // Draw Csv Table General Incomes  Static
void DrawCsvTableIncS    (const rcv::Document &csv); // Draw Csv Table Local   Incomes  Static
void DrawCsvTableExpProgS(const rcv::Document &csv); // Draw Csv Table Local   Expanses Static
void DrawBudgetInput(budget &budget); // Draw budget input forms
void        ZeroExpandBud(std::span<      char> code); // modifies input by adding missing zeros
std::string GetProcessBud(std::span<const char> code); // returns clean 10-digit string
std::string ComposeCsvFname(const budget &budget); // Compose CSV filename
std::string ComposeCsvQuery(const budget &budget); // Compose API query

// throwing error handling, out-param
void OpenbudgetGet(std::string_view request, auto &response);
void OpenbudgetGet(std::string_view request, auto &response, std::chrono::steady_clock::duration timeout);

// non-throwing error handling versions with `Response` output via return `std::expected` (not an out param)
template<class Response> std::expected<Response, std::string> OpenbudgetGet(std::string_view request)
{
	const std::string target = std::string(API_PATH_BASE) + std::string(request);
	std::cout << "Trace: target: " << target << "\n";

	try {
		net::io_context                 context ;
		net::ip::tcp::resolver resolver(context);
		net::tcp_stream        tstream (context);

		net::http::request<net::http::empty_body> request;
		request.method(net::http::verb::get);
		request.version(11);
		request.set(net::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		request.set(net::http::field::host      , API_HOST);
		request.keep_alive(false);
		request.target(target);
		request.set(net::http::field::accept    , "*/*");

		//std::cout << "\n" << request.base() << "\n";

		const auto      resolved = resolver.resolve(API_HOST, "80");
		tstream.connect(resolved);
		net::http::write(tstream, request);

		net::flat_buffer         buffer;
		     Response                    response;
		net::http::read(tstream, buffer, response);

		net::error_code erc;
		tstream.socket().shutdown(net::ip::tcp::socket::shutdown_both, erc);
		if (erc && erc != net::errc::not_connected) return std::unexpected("shutdown: " + erc.message());

		return response;
	}
	catch (std::exception& e) { return std::unexpected(e.what()); }
}
template<class Response> std::expected<Response, std::string> OpenbudgetGet(std::string_view request, std::chrono::steady_clock::duration timeout)
{
	const std::string target = std::string(API_PATH_BASE) + std::string(request);
	/*  */std::string error; //? replace with std::error_code or boost::beast::error_code or custom one

	//using namespace std::literals::chrono_literals;
	std::cout << "Trace: target: " << target << "\n";

	net::io_context                          ioc;
	     Response                                 response;
	std::make_shared<net::Session<Response>>(ioc, response, &error)->Set(API_HOST, net::PORT_HTTP, target, timeout);
	ioc.run(); // Dequeue and execute (this is a blocking call)

	if (!error.empty()) return std::unexpected(error);

	return response;
}


CsvGet::CsvGet()
{
	if (std::filesystem::exists          (FS_DOWNLOADS) == false)
		std::filesystem::create_directory(FS_DOWNLOADS);
}

void CsvGet::operator()()
{
	constexpr ImGuiWindowFlags wFlags =
		ImGuiWindowFlags_NoCollapse            |
		ImGuiWindowFlags_NoSavedSettings       |
		ImGuiWindowFlags_AlwaysAutoResize      |
		0;

	im::SetNextWindowSizeConstraints(ImVec2{0, 0}, ImVec2{FLT_MAX, 400});
	///**/im::SetNextWindowContentSize(ImVec2{0.f, 400.f});
	if (im::Begin("TEST_CSV_DOWNLOAD", nullptr, wFlags))
	{
		//static budget budget{.code = DEF_BUD_CODE_HRO_CHE_HALF};

		if (im::Button("Open budget..")) im::OpenPopup(NAME_MODAL_INPUT); // button for Modal Input
		this->DrawModalInput();

		// Draw CSV Budget Table
		using namespace UA::budget::incomes;
		if (this->csv.GetColumnCount() != 0) { //? replace with `bool`?
			/*
			DrawCsvTableIncDynamic(this->csv);*/
			//DrawCsvTableIncS(this->csv);
			DrawCsvTableS(this->csv);
		}
		else UA::budget::DrawEmptyTable(labelS, {COD_INCO, FUND_TYP, ZAT_AMT, FAKT_AMT/*, PLANS_AMT */});

	} im::End();
}

void CsvGet::DrawModalInput()
{
	static      bool   executed  = false;
	static      bool   csvExists = false;
	static  fs::path   csvPath;
	static std::string csvQuery;
	static std::string feedback; // user feedback

	/* */im::SetNextWindowPos(im::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2{0.6f, 0.6f});
	/* bool openPopup = true;
	if (!im::BeginPopupModal(NAME_MODAL_INPUT, &openPopup, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) return; */
	if (!im::BeginPopup(NAME_MODAL_INPUT, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) return;

	DrawBudgetInput(this->budget);

	if (im::Button(API_PATH_PING)) // ping api.openbudget.gov.ua
	{
	#if 1 // std::expected
		const auto response = OpenbudgetGet<net::http::response<net::http::string_body>>(API_PATH_PING);
		if        (response) feedback = "ping response: (" + std::to_string(response->result_int()) + ") " + response->body();
		else                 feedback = "Ping failed, reason: " + response.error();

	#else // try-catch
		try {
			net::http::response<net::http::string_body> response;
			OpenbudgetGet(API_PATH_PING, response);

			feedback = "ping response: (" + std::to_string(response.result_int()) + ") " + response.body();
		}
		catch (std::exception& e) { feedback = "ping attempt failed with: "s + e.what(); }
	#endif // std::ecpected/try-catch

	}

	// Advance with provided budget input
	/**/im::SameLine();
	if (im::Button("Execute", ImVec2{im::GetContentRegionAvail().x /*- im::GetStyle().ScrollbarSize*/, 0}))
	{
		executed  = true ;
		csvExists = false;
		this->csv.Clear();

		const std::string csvName = ComposeCsvFname(budget);
		const fs ::path   csvFile = fs::path(csvName).replace_extension(".csv");
		csvPath  = FS_DOWNLOADS / csvFile;
		csvQuery = ComposeCsvQuery(budget);

		// Check if already downloaded
		if (fs::exists(csvPath)) { // check if requested file is present on disk
			csvExists = true;   //! fs::file_size(csvPath) - add human readable wrapper for file  size (not just bytes)

			std::string ftime; // file time

			#ifndef __clang__
			const auto ftime_ftt = fs::last_write_time(csvPath); // _ftt = file_time_type
			if (bp::timezone) ftime = std::format("{} {}", bp::chrono::to_string(bp::fs::to_local(*bp::timezone, ftime_ftt)), bp::timezone->name());
			else              ftime = bp::fs::to_string(ftime_ftt);
			#else
			ftime = bp::fs::to_string(fs::last_write_time(csvPath)) + " UTC";
			#endif

			feedback = std::format("File already exist on disk:\n{}\n{}\n{}", csvPath.string(), ftime, bp::fs::filesize(fs::file_size(csvPath)));
			//feedback += std::format("\nUTC: {} UTC", bp::fs::to_string(fs::last_write_time(csvPath)));
		}
		else feedback = std::format("File: {}\nLocal copies: none", csvPath.string());

		#if 1 // trace:             Downloads/
		std::cout << "\nTrace:           " << csvName;
		std::cout << "\nTrace:           " << csvFile.string();
		std::cout << "\nTrace: "           << csvPath.string();
		std::cout << "\nTrace: "           << csvQuery;
		std::cout << '\n';
		#endif

		// Check if picked table drawing is currently supported
		try {
			if   (this->budget.indicator == GENERAL)
			{
				if   (this->budget.item == INCOMES ) this->DrawCsvTableS = DrawCsvTableGenIncS;
				else { this->csv.Clear(); throw std::runtime_error(std::format("budget.item == '{}' is not supported", items[this->budget.item])); }
			}
			elif (this->budget.indicator == LOCAL  )
			{
				if   (this->budget.item == INCOMES ) this->DrawCsvTableS = DrawCsvTableIncS;
				elif (this->budget.item == EXPENSES) {
					if (this->budget.classif == PROGRAM) this->DrawCsvTableS = DrawCsvTableExpProgS;
					else { this->csv.Clear(); throw std::runtime_error(std::format("budget.classif == '{}' is not supported", classifs[this->budget.classif])); }
				}
				else { this->csv.Clear(); throw std::runtime_error(std::format("budget.item == '{}' is not supported", items[this->budget.item])); }
			}
			else { this->csv.Clear(); throw std::runtime_error(std::format("budget.indicator == '{}' is not supported", items[this->budget.indicator])); }
		}
		catch (std::runtime_error& e) { executed = false; feedback = e.what(); }

	}

	// Display user feedback: status reporting text
	if (!feedback.empty()) {
		im::PushTextWrapPos(im::GetContentRegionAvail().x);
		im::Text("%s", feedback.c_str());
		im::PopTextWrapPos();
	}

	// Draw csv load options: disk/network
	if (executed)
	{
		// Handle case when file present on disk: load, download (refresh)
		// b1: Load existing file from disk
		im::BeginDisabled(!csvExists);
		if (im::Button("load (from disk)##bt_load_disk") && csvExists) {
			executed  = false;
			csvExists = false;
			//this->csv.Load(csvPath.string(), rcv::LabelParams(), rcv::SeparatorParams(';', false, true, true, false));
			this->csv.Load(csvPath.string(), rcv::LabelParams(), rcv::SeparatorParams(';', false, true, true, false), rcv::ConverterParams(), rcv::LineReaderParams(false, '#', true));
			std::cout << "Trace: CSV rows: " << this->csv.GetRowCount() << '\n';
			feedback = "loaded: from disk";
		}
		im::EndDisabled();

		// b2: Refresh from network (download & replace)
		/**/im::SameLine();
		if (im::Button(std::format("{} (from network)##bt_load_net", csvExists ? "refresh" : "download").c_str())) {
			executed  = false;
			csvExists = false;

			// Try to download the new file through internet
			const auto ok = CsvDownload(csvPath, csvQuery, feedback);
			if (ok) this->csv.Load(csvPath.string(), rcv::LabelParams(), rcv::SeparatorParams(';', false, true, true, false), rcv::ConverterParams(), rcv::LineReaderParams(false, '#', true));
			else feedback = "File download attempt failed, reason: "s + ok.error();

		}
	}

	im::EndPopup();
}

void DrawBudgetInput(budget &budget) // Draw budget input forms
{
	static_assert(sizeof(budget::code)/sizeof(char) >= BUD_CODE_STRLEN_MIN + 1); // must be at least +1 for `\0`
	constexpr ImGuiTabBarFlags flagsTabBar =
		ImGuiTabBarFlags_None                    |
		ImGuiTabBarFlags_NoTooltip               | // custom tooltip replaces this anyways
		ImGuiTabBarFlags_FittingPolicyScroll     | // no squash - no tooltip with full name
		ImGuiTabBarFlags_DrawSelectedOverline    | // a visual nimb over the selected tab for better differentiation
		//ImGuiTabBarFlags_Reorderable             |
		//ImGuiTabBarFlags_FittingPolicyResizeDown |
		0;

	if (im::BeginTabBar(__func__, flagsTabBar))
	{
		im::TabItemButton("indicators:", ImGuiTabItemFlags_Leading);

		if (im::BeginTabItem(indicators[GENERAL]))
		{
			if (budget.indicator != GENERAL) budget.indicator = GENERAL;

			// attempt to align elements of both tabs
			//im::NewLine();
			//im::SetCursorPosY(im::GetCursorPosY() + im::GetTextLineHeight());
			//im::SetCursorPosY(im::GetCursorPosY() + im::GetFrameHeight() + im::GetStyle().FramePadding.y);

			// budget item: incomes, expenses, ...
			im::Combo(QUERY_BITEM, &budget.item, UA::budget::items, IM_ARRAYSIZE(UA::budget::items));

			// classification type: program, functional, ..
			im::BeginDisabled(budget.item != EXPENSES && budget.item != CREDITS);
			/* free scope */ {
				im::Text("%s:", QUERY_BCLASS);
				im::Indent();

				/*           */ im::RadioButton(classifs[PROGRAM   ], &budget.classif, PROGRAM   ); //  PROGRAM   //
				im::SameLine(); im::RadioButton(classifs[FUNCTIONAL], &budget.classif, FUNCTIONAL); // FUNCTIONAL //

				im::BeginDisabled(budget.item != EXPENSES);
				im::SameLine(); im::RadioButton(classifs[ECONOMICS ], &budget.classif, ECONOMICS ); // ECONOMICS  //
				im::EndDisabled(); //   .item != EXPENSES

				im::BeginDisabled(budget.item != CREDITS);
				im::SameLine(); im::RadioButton(classifs[CREDIT    ], &budget.classif, CREDIT    ); //   CREDIT   //
				im::EndDisabled(); //   .item != CREDITS

				im::Unindent();
			}
			im::EndDisabled(); //   .item != EXPENSES && budget.item != CREDITS

			// budget year
			im::InputInt(QUERY_BYEAR, &budget.year);

			im::EndTabItem();
		}

		if (im::BeginTabItem(indicators[LOCAL  ]))
		{
			if (budget.indicator != LOCAL) budget.indicator = LOCAL;

			// Draw budget input forms

			// budget code
			im::InputText(QUERY_BCODE, budget.code, budget::BUFF_BCODE_SIZE, ImGuiInputTextFlags_CharsDecimal);
			//if (im::Button("test GetProcessBud(code)")) { std::cout <<"Trace: " << GetProcessBud(budget.code) << '\n'; } // test
			//if (im::Button("test ZeroExpandBud(code)")) {                          ZeroExpandBud(budget.code)        ; } // test

			// budget item: incomes, expenses, ...
			im::Combo(QUERY_BITEM, &budget.item, UA::budget::items, IM_ARRAYSIZE(UA::budget::items));

			// classification type: program, functional, ..
			im::BeginDisabled(budget.item != EXPENSES && budget.item != CREDITS);
			/* free scope */ {
				im::Text("%s:", QUERY_BCLASS);
				im::Indent();

				/*           */ im::RadioButton(classifs[PROGRAM   ], &budget.classif, PROGRAM   ); //  PROGRAM   //
				im::SameLine(); im::RadioButton(classifs[FUNCTIONAL], &budget.classif, FUNCTIONAL); // FUNCTIONAL //

				im::BeginDisabled(budget.item != EXPENSES);
				im::SameLine(); im::RadioButton(classifs[ECONOMICS ], &budget.classif, ECONOMICS ); // ECONOMICS  //
				im::EndDisabled(); //   .item != EXPENSES

				im::BeginDisabled(budget.item != CREDITS);
				im::SameLine(); im::RadioButton(classifs[CREDIT    ], &budget.classif, CREDIT    ); //   CREDIT   //
				im::EndDisabled(); //   .item != CREDITS

				im::Unindent();
			}
			im::EndDisabled(); //   .item != EXPENSES && budget.item != CREDITS

			// budget period
			im::Text("%s:", QUERY_BPERIOD);
			im::SameLine(); im::RadioButton(periods[MONTH  ], &budget.period, MONTH  );
			im::SameLine(); im::RadioButton(periods[QUARTER], &budget.period, QUARTER);

			// budget year
			im::InputInt(QUERY_BYEAR, &budget.year);

			im::EndTabItem();
		}

		if   (budget.item == EXPENSES && budget.classif == CREDIT   ) budget.classif = classif_e(0); // reset
		elif (budget.item == CREDITS  && budget.classif == ECONOMICS) budget.classif = classif_e(0); // reset

		if   (budget.year < BUD_YEAR_MIN) budget.year = BUD_YEAR_MIN;
		elif (budget.year > BUD_YEAR_MAX) budget.year = BUD_YEAR_MAX;

		im::EndTabBar();
	}
}

std::expected<void, std::string> CsvDownload(const fs::path &csvPath, const std::string &csvQuery, std::string &feedback)
{
#if 1 // dynamic_body

#if 1 // std::expected
	auto response_e = OpenbudgetGet<net::http::response<net::http::dynamic_body>>(csvQuery, 4s); // _e - expected
	if (!response_e) return std::unexpected(response_e.error());
	else
	{
		auto response = *response_e;

	#if 1  // log
		std::cout << "\n";
		std::cout << csvPath << "\n";
		if (const auto it = response.base().find("content-type"       ); it != response.base().end()) std::cout << "content-type: '"        << it->value() << "'\n";
		if (const auto it = response.base().find("content-disposition"); it != response.base().end()) std::cout << "content-disposition: '" << it->value() << "'\n";
		//std::cout << "\nTrace: `response`:\n" << response << "\n";
		std::cout << "\nTrace: `response.base()`:\n" << response.base() << "\n";
	#endif // log

		// Check response, if OK - write received file on disk
		if (response.result() == net::http::status::ok)
		{
			std::ofstream csvOfstream(csvPath, std::fstream::trunc);
			csvOfstream << net::buffers_to_string(response.body().data()); // write
			csvOfstream.close();

			std::string ftime; // file time

		#ifndef __clang__
			const auto ftime_ftt = fs::last_write_time(csvPath); // _ftt = file_time_type
			if (bp::timezone) ftime = std::format("{} {}", bp::chrono::to_string(bp::fs::to_local(*bp::timezone, ftime_ftt)), bp::timezone->name());
			else              ftime = bp::fs::to_string(ftime_ftt) + " UTC";
		#else // Clang has no chrono::format (c++20) support -_-
			ftime = bp::fs::to_string(fs::last_write_time(csvPath)) + " UTC";
		#endif

			feedback = std::format("Downloaded from network:\n{}\n{}\n{}", csvPath.string(), ftime, bp::fs::filesize(fs::file_size(csvPath)));
		}
		else {
			std::cerr << "Trace: `response.body()`:\n" << net::buffers_to_string(response.body().data()) << "\n";
			return std::unexpected("(" + std::to_string(response.result_int()) + "):\n" + net::buffers_to_string(response.body().data()));
			//throw std::runtime_error("(" + std::to_string(response.result_int()) + "):\n" + net::buffers_to_string(response.body().data()));
		}

	#if 1 // Validate header params - text/csv, attachment, filename=<value>
		if (response.result() == net::http::status::ok)
		{
			// validate: content-type
			if (const auto it = response.base().find("content-type"); it != response.base().end())
				if (it->value().find("text/csv") == beast::string_view::npos)
					throw std::runtime_error("'content-type' is not 'text/csv'");

			// validate: content-type content-disposition //? write own general parser, since next minimalistic code parses fewer cases
			std::string filename; //? perfectly, way more cases should be parsed: https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers/Content-Disposition
			if (const auto it = response.base().find("content-disposition"); it != response.base().end())
			{
				const auto &value = it->value(); // content-disposition field parameter (value)
				constexpr auto paramAttachment = "attachment"sv;
				constexpr auto paramFilename   =   "filename"sv;

				size_t pos = 0;
				if (pos = value.find(paramAttachment, pos); pos != value.npos) // attachment
				{
					pos += paramAttachment.size();

					if (pos = value.find(paramFilename, pos); pos != value.npos) { //? filename (only '=' and not '*=')
						pos += paramFilename.size();
						//if (value[pos] == '*') { pos += 2; ... } // not implemented full-case parsing
						if (value[pos] != '=') return std::unexpected(std::format("parse_error: value[pos] != '=', it is: '{}'", value[pos]));
						pos++;
						if (value.find(';', pos) != value.npos) return std::unexpected("parse_error: value.find(';') != value.npos (other parameters were not expected)");

						filename = value.substr(pos);

					} else return std::unexpected("'content-disposition' has no 'filename' parameter");
				} else return     std::unexpected("'content-disposition' has no 'attachment' parameter");
			} else return         std::unexpected("'content-disposition' header is missing in response"); //? feedback vs fail

			// Compare filenames: expected (composed) filename VS from 'content-disposition' response header parameter
			if (csvPath.filename().string() != filename) {
				const std::string report = "Warn: 'content-disposition' has different file name: expected vs actual:\n" + csvPath.filename().string() + '\n' + filename;
				std::cout << report << '\n';
				feedback += "\n\n" + report;
			}
		}
	#endif // Validate header params - text/csv, attachment, filename=<value>
	}

	return {};

#else // try-catch
	try {
		// Receive response from the openbudget.gov.ua
		net::http::response<net::http::dynamic_body> response;
		OpenbudgetGet(csvQuery, response, 4s);

		#if 1  // log
		std::cout << "\n";
		std::cout << csvPath << "\n";
		if (const auto it = response.base().find("content-type"       ); it != response.base().end()) std::cout << "content-type: '"        << it->value() << "'\n";
		if (const auto it = response.base().find("content-disposition"); it != response.base().end()) std::cout << "content-disposition: '" << it->value() << "'\n";
		//std::cout << "\nTrace: `response`:\n" << response << "\n";
		std::cout << "\nTrace: `response.base()`:\n" << response.base() << "\n";
		#endif // log

		// Check response, if OK - write received file on disk
		if (response.result() == net::http::status::ok) {
			std::ofstream csvOfstream(csvPath, std::fstream::trunc);
			csvOfstream << net::buffers_to_string(response.body().data());
			csvOfstream.close();

			std::string ftime;

		#ifndef __clang__
			const auto ftime_ftt = fs::last_write_time(csvPath); // _ftt = file_time_type
			if (bp::timezone) ftime = std::format("{} {}", bp::chrono::to_string(bp::fs::to_local(*bp::timezone, ftime_ftt)), bp::timezone->name());
			else              ftime = bp::fs::to_string(ftime_ftt) + " UTC";
		#else
			ftime = bp::fs::to_string(fs::last_write_time(csvPath)) + " UTC";
		#endif

			feedback = std::format("Downloaded from network:\n{}\n{}\n{}", csvPath.string(), ftime, bp::fs::filesize(fs::file_size(csvPath)));
		}
		else {
			std::cerr << "Trace: `response.body()`:\n" << net::buffers_to_string(response.body().data()) << "\n";
			throw std::runtime_error("(" + std::to_string(response.result_int()) + "):\n" + net::buffers_to_string(response.body().data()));
		}

		#if 1 // Validate header params - text/csv, attachment, filename=<value>
		if (response.result() == net::http::status::ok) {
			// validate: content-type
			if (const auto it = response.base().find("content-type"); it != response.base().end())
				if (it->value().find("text/csv") == beast::string_view::npos)
					throw std::runtime_error("'content-type' is not 'text/csv'");

			// validate: content-type content-disposition //? write own general parser, since next minimalistic code parses fewr cases (suitable RN)
			std::string filename; //? perfectly, way more cases should be parsed: https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers/Content-Disposition
			if (const auto it = response.base().find("content-disposition"); it != response.base().end()) {
				const auto &value = it->value(); // content-disposition field parameter (value)
				constexpr auto paramAttachment = "attachment"sv;
				constexpr auto paramFilename   =   "filename"sv;

				size_t pos = 0;
				if (pos = value.find(paramAttachment, pos); pos != value.npos) { // attachment
					pos += paramAttachment.size();

					if (pos = value.find(paramFilename, pos); pos != value.npos) { //? filename (only '=' and not '*=')
						pos += paramFilename.size();
						//if (value[pos] == '*') { pos += 2; ... } // not implemented full-case parsing
						if (value[pos] != '=') throw std::runtime_error(std::format("parse_error: value[pos] != '=', it is: '{}'", value[pos]));
						pos++;
						if (value.find(';', pos) != value.npos) throw std::runtime_error("parse_error: value.find(';') != value.npos (other parameters were not expected)");

						filename = value.substr(pos);

					} else throw std::runtime_error("'content-disposition' has no 'filename' parameter");
				} else throw     std::runtime_error("'content-disposition' has no 'attachment' parameter");
			}

			// Compare filenames: expected (composed) filename VS from 'content-disposition' response header parameter
			if (csvPath.filename().string() != filename) {
				std::string report = "Warn: 'content-disposition' has different file name: expected vs actual:\n" + csvPath.filename().string() + '\n' + filename;
				std::cout << report << '\n';
				feedback += "\n\n" + report;
			}
		}
		#endif // Validate header params - text/csv, attachment, filename=<value>

	}
	catch (std::exception& e) { throw std::runtime_error("file download attempt failed with: "s + e.what()); }

	return {};
#endif // std::expected / try-catch

#else //    file_body
	fs::remove(csvPath);
	net::error_code ec;
	net::http::response<net::http::file_body> response;
	response.body().open(csvPath.string().c_str(), net::file_mode::write_new, ec);

	if (!ec.failed()) {
		OpenbudgetGet(PATH_LOC_BUD_DAT + csvQuery, response);

		const auto it = response.base().find("content-disposition");
		std::cout << "\n";
		std::cout << ec.message() << "\n";

		//std::cout << "\nHeaders " << response.base() << "\n";
		if (response.result() == net::http::status::ok) {
			feedback = "downloaded from network";
		}
		else
			feedback = "Error: " + std::to_string(response.result_int());
	}
#endif // dynamic_body/file body // also option to switch from `empty_body` after `read_header`: https://stackoverflow.com/questions/76431225/how-to-convert-a-boost-beast-requeststring-body-to-a-requestfile-body
}

void OpenbudgetGet(std::string_view request, auto &response)
{
	const std::string target = std::string(API_PATH_BASE) + std::string(request);
	std::cout << "Trace: target: " << target << "\n";

	try {
		net::io_context                 context ;
		net::ip::tcp::resolver resolver(context);
		net::tcp_stream        tstream (context);

		net::http::request<net::http::empty_body> request;
		request.method(net::http::verb::get);
		request.version(11);
		request.set(net::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		request.set(net::http::field::host      , API_HOST);
		request.keep_alive(false);
		request.target(target);
		request.set(net::http::field::accept    , "*/*");

		//std::cout << "\n" << request.base() << "\n";

		const auto      resolved = resolver.resolve(API_HOST, "80");
		tstream.connect(resolved);
		net::http::write(tstream, request);

		net::flat_buffer         buffer;
		net::http::read(tstream, buffer, response);

		tstream.socket().shutdown(net::ip::tcp::socket::shutdown_both);
	}
	catch (std::exception& e) { std::cerr << "Exception: " << e.what() << "\n"; throw e; }
}
void OpenbudgetGet(std::string_view request, auto &response, std::chrono::steady_clock::duration timeout)
{
	const std::string target = std::string(API_PATH_BASE) + std::string(request);
	/*  */std::string error; //? replace with std::error_code or boost::beast::error_code or custom one

	using namespace std::literals::chrono_literals;
	std::cout << "Trace: target: " << target << "\n";
	net::io_context ioc;
	std::make_shared<net::Session<decltype(response)>>(ioc, response, &error)->Set(API_HOST, net::PORT_HTTP, target, timeout); //? std::remove_reference_t<decltype(response)>
	ioc.run(); // Dequeue and execute (this is a blocking call)

	if (!error.empty()) throw std::runtime_error("Error: " + error + "\n");
}

//std::array splitchars = {' ', '-'}; // then use std::ranges - constexpr, but not hashed
//inline const std::unordered_set<char> splitchars = {' ', '.', '+', '-', '*', '/'};

inline void        ZeroExpandBud(std::span<      char> code) // modifies input by adding missing zeros
{ // "01.234-56 7\0" -> "01.234-56 700\0"
	const auto &len = code.size() - 1; // last entry is always `\0`
	ushort digits = 0;
	for (size_t i = 0; i < len; i++)
	{
		//if   (splitchars.contains(code[i])) /*continue*/;
		if   (IsDigit(     code[i])       ) digits++;
		elif (             code[i] == '\0') { // early end, yes space for expand
			while (i < len && digits < BUD_CODE_DIGIT_COUNT) { code[i] = '0'; /*and*/ digits++;	/*and*/ i++; }
			//if (digits != BUD_CODE_DIGIT_COUNT) return; // BAD // no space left
			code[i] = '\0';
			return; // OK - main exit
		} // else nothing
	}

	return; // parse failed case, but allowed to reach
}
inline std::string GetProcessBud(std::span<const char> code) // returns clean 10-digit string
{ // "01.234-56 7\0" -> "0123456700\0"
	std::string processed = BUD_CODE_INIT; // cleaned and expanded budget code
	for (size_t i = 0; i < code.size() - 1; i++) {
		if   (IsDigit(code[i])       ) processed[i] = code[i];
		elif (        code[i] == '\0') break;
	}
	return processed;
}

inline std::string ComposeCsvFname(const budget &budget) // Compose csv filename
{
	using namespace UA::budget;

	std::ostringstream oss;
	oss.imbue(std::locale::classic()); // fix clang num separator '1 234' and msvc cringe only first digit streamed '1' from '1234' with further stream breaking (unreadable after '1')

	/*                             */oss        << indicators   [budget.indicator];
	if (budget.indicator == LOCAL) { oss << '_' << GetProcessBud(budget.code     ); }
	/*                             */oss << '_' << items        [budget.item     ];

	if (budget.item == EXPENSES || budget.item == CREDITS) // _optional_
		/*                         */oss << '_' << classifs     [budget.classif  ];
	/*                             */oss << '_' <<               budget.year;
	if (budget.indicator == LOCAL) { oss << '_' << periodsLow   [budget.period   ]; }

	return oss.str();
}
inline std::string ComposeCsvQuery(const budget &budget) // Compose API query
{ //? boost::url - https://github.com/boostorg/url
	using namespace UA::budget;

	std::ostringstream oss;
	oss.imbue(std::locale::classic()); // fix clang num separator '1 234' and msvc cringe only first digit streamed '1' from '1234' with further stream breaking (unreadable after '1')

	/* target:                     */oss << indicatorsQuery[budget.indicator];

	/* parameters:                 */oss << '?';
	if (budget.indicator == LOCAL) { oss        << QUERY_BCODE   << '=' << GetProcessBud(budget.code) << '&'; }
	/*                             */oss        << QUERY_BITEM   << '=' << items        [budget.item   ];

	if (budget.item == EXPENSES || budget.item == CREDITS) // _optional_
		/*                         */oss << '&' << QUERY_BCLASS  << '=' << classifs     [budget.classif];
	if (budget.indicator == LOCAL) { oss << '&' << QUERY_BPERIOD << '=' << periods      [budget.period ]; }
	/*                             */oss << '&' << QUERY_BYEAR   << '=' <<               budget.year;

	return oss.str();
}

void DrawCsvTableGenIncS (const rcv::Document &csv)
{
	using namespace UA::budget::general::incomes;

	if (csv.GetRowCount() == 0) {
		UA::budget::DrawEmptyTable(labelS, {BUDG_TYP, COD_INCO, PLAN_BEGIN_YEAR_AMT, PLAN_CORR_YEAR_AMT, PLAN_CORR_PERIOD_AMT, DONE_PERIOD_AMT});
		return;
	}

	constexpr ImGuiTabBarFlags flagsTabBar =
		ImGuiTabBarFlags_None                    |
		ImGuiTabBarFlags_NoTooltip               | // custom tooltip replaces this anyways
		ImGuiTabBarFlags_FittingPolicyScroll     | // no squash - no tooltip with full name
		ImGuiTabBarFlags_DrawSelectedOverline    | // a visual nimb over the selected tab for better differentiation
		//ImGuiTabBarFlags_Reorderable             |
		//ImGuiTabBarFlags_FittingPolicyResizeDown |
		0;

	std::chrono::year_month period; // holds REP_PERIOD (converted)
	auto timePeriod = csv.GetCell<std::string>(REP_PERIOD, 0);
	bp::chrono::from_stream(timePeriod, "%m.%Y", period);

	if (im::BeginTabBar(__func__, flagsTabBar))
	{
		// Display `year` as leading tab (button)
		im::TabItemButton(std::format("{}:", bp::chrono::get(period.year())).c_str(), ImGuiTabItemFlags_Leading  /*| ImGuiTabItemFlags_NoTooltip*/);
		//im::TabItemButton(          "(month/quarter)"                             , ImGuiTabItemFlags_Trailing /*| ImGuiTabItemFlags_NoTooltip*/);

		size_t countPeriod = 0;
		std::string timePeriod;
		const size_t rowCount = csv.GetRowCount();

		for (size_t row = 0; row < rowCount; /*++*/) // display months/quarters as tabs with a table (with recpective data part)
		{
			timePeriod = csv.GetCell<std::string>(REP_PERIOD, row);
			bp::chrono::from_stream(timePeriod, "%m.%Y", period);

			//const bool beginTabItem = im::BeginTabItem(std::to_string(bp::chrono::get(period.month())).c_str(), nullptr, ImGuiTabItemFlags_NoPushId);
			const bool beginTabItem = im::BeginTabItem(std::format("{}##{}", bp::chrono::get(period.month()), countPeriod).c_str(), nullptr, ImGuiTabItemFlags_NoPushId);
			im::SetItemTooltip("%s", std::format("{:L%B}", period.month()).c_str());

			if (beginTabItem)
			{
				constexpr ImGuiTableFlags flagsTable =
				ImGuiTableFlags_NoSavedSettings        |
				ImGuiTableFlags_Reorderable            |
				ImGuiTableFlags_Resizable              |
				ImGuiTableFlags_Hideable               |
				ImGuiTableFlags_ScrollY                |
				ImGuiTableFlags_ScrollX                |
				ImGuiTableFlags_Borders                |
				ImGuiTableFlags_RowBg                  |
				//ImGuiTableFlags_Sortable               |
				//ImGuiTableFlags_SortMulti              |
				//ImGuiTableFlags_SortTristate           |
				ImGuiTableFlags_SizingFixedFit         | // will have minimal column width possible
				//ImGuiTableFlags_SizingStretchProp      | // will have odd width (more than cell/column needs)
				//ImGuiTableFlags_ContextMenuInBody      |
				//ImGuiTableFlags_HighlightHoveredColumn |
				0;

				static const std::array labelV {BUDG_TYP, FUND_TYP, COD_INCO, PLAN_BEGIN_YEAR_AMT, PLAN_CORR_YEAR_AMT, PLAN_CORR_PERIOD_AMT, DONE_PERIOD_AMT}; // C - columns to show
				if (im::BeginTable("csv table", scast<int>(labelV.size()), flagsTable))
				{
					{ // declare column headers
						size_t c = 0;
						im::TableSetupScrollFreeze(0, 1); // pin header (1st row)
						im::TableSetupColumn(labelS[labelV[c++]] /*   BUDG_TYP           */, ImGuiTableColumnFlags_NoHeaderWidth);
						im::TableSetupColumn(labelS[labelV[c++]] /*   FUND_TYP           */, ImGuiTableColumnFlags_NoHeaderWidth);
						im::TableSetupColumn(labelS[labelV[c++]] /*   COD_INCO           */, ImGuiTableColumnFlags_NoHeaderWidth);
						im::TableSetupColumn(labelS[labelV[c++]] /* PLAN_BEGIN_YEAR_AMT  */, ImGuiTableColumnFlags_NoHeaderWidth | ImGuiTableColumnFlags_DefaultHide);
						im::TableSetupColumn(labelS[labelV[c++]] /* PLAN_CORR_YEAR_AMT   */, ImGuiTableColumnFlags_NoHeaderWidth);
						im::TableSetupColumn(labelS[labelV[c++]] /* PLAN_CORR_PERIOD_AMT */, ImGuiTableColumnFlags_NoHeaderWidth | ImGuiTableColumnFlags_DefaultHide);
						im::TableSetupColumn(labelS[labelV[c++]] /*      DONE_PERIOD_AMT */, ImGuiTableColumnFlags_NoHeaderWidth);
						assert(c == labelV.size());
					}

					// draw column headers (with tooltips)
					im::TableNextRow(ImGuiTableRowFlags_Headers);
					const int columns_count = im::TableGetColumnCount();
					for (int c = 0; c < columns_count; c++)
					{
						if (!im::TableSetColumnIndex(c)) continue;

						const char *const name = im::TableGetColumnName(c);
						im::PushID(c); /*and*/ im::TableHeader(name); /*and*/ im::PopID();

						// tooltip with label description
						if (im::IsItemHovered()) {
							im::BeginTooltip(); /*and>*/ im::Text("%s", labelD[labelV[c]]); /*<and*/ im::EndTooltip();
						}
					}

					// draw rows
					for (/*row*/; row < rowCount && timePeriod == csv.GetCell<std::string>(REP_PERIOD, row); row++)
					{
						size_t c = 0;
						im::TableNextRow();
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /*   BUDG_TYP           */, row).c_str());
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /*   FUND_TYP           */, row).c_str());
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /*   COD_INCO           */, row).c_str()); /*scope*/ { // tooltip
							if (im::IsItemHovered()) {
								im::BeginTooltip();   /*and*/ im::PushTextWrapPos(im::GetCursorPos().x + 600.f);
								im::Text("%s", csv.GetCell<std::string>(COD_INCO_NAME, row).c_str());
								im::PopTextWrapPos(); /*and*/ im::EndTooltip();
							}
						}
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /* PLAN_BEGIN_YEAR_AMT  */, row).c_str());
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /* PLAN_CORR_YEAR_AMT   */, row).c_str());
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /* PLAN_CORR_PERIOD_AMT */, row).c_str());
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /* DONE_PERIOD_AMT      */, row).c_str());
						assert(c == labelV.size());
					}

					im::EndTable();
				}
				else while (++row < rowCount && timePeriod == csv.GetCell<std::string>(REP_PERIOD, row));

				im::EndTabItem();
			}
			else while (++row < rowCount && timePeriod == csv.GetCell<std::string>(REP_PERIOD, row));

			countPeriod++;
		}

		im::EndTabBar();
	}
}
void DrawCsvTableIncS    (const rcv::Document &csv)
{
	using namespace UA::budget;
	using namespace UA::budget::incomes;

	if (csv.GetRowCount() == 0) {
		UA::budget::DrawEmptyTable(labelS, {REP_PERIOD, COD_INCO, ZAT_AMT, FAKT_AMT, PLANS_AMT});
		return;
	}

	constexpr ImGuiTabBarFlags flagsTabBar =
		ImGuiTabBarFlags_None                    |
		ImGuiTabBarFlags_NoTooltip               | // custom tooltip replaces this anyways
		ImGuiTabBarFlags_FittingPolicyScroll     | // no squash - no tooltip with full name
		ImGuiTabBarFlags_DrawSelectedOverline    | // a visual nimb over the selected tab for better differentiation
		//ImGuiTabBarFlags_Reorderable             |
		//ImGuiTabBarFlags_FittingPolicyResizeDown |
		0;

	std::chrono::year_month period; // holds REP_PERIOD (converted)
	auto timePeriod = csv.GetCell<std::string>(REP_PERIOD, 0);
	bp::chrono::from_stream(timePeriod, "%m.%Y", period);

	if (im::BeginTabBar(__func__, flagsTabBar))
	{
		// Display `year` as leading tab (button)
		im::TabItemButton(std::format("{}:", bp::chrono::get(period.year())).c_str(), ImGuiTabItemFlags_Leading  /*| ImGuiTabItemFlags_NoTooltip*/);
		//im::TabItemButton(          "(month/quarter)"                             , ImGuiTabItemFlags_Trailing /*| ImGuiTabItemFlags_NoTooltip*/);

		size_t countPeriod = 0;
		std::string timePeriod;
		const size_t rowCount = csv.GetRowCount();

		for (size_t row = 0; row < rowCount; /*++*/) // display months/quarters as tabs with a table (with recpective data part)
		{
			timePeriod = csv.GetCell<std::string>(REP_PERIOD, row);
			bp::chrono::from_stream(timePeriod, "%m.%Y", period);

			//const bool beginTabItem = im::BeginTabItem(std::to_string(bp::chrono::get(period.month())).c_str(), nullptr, ImGuiTabItemFlags_NoPushId);
			const bool beginTabItem = im::BeginTabItem(std::format("{}##{}", bp::chrono::get(period.month()), countPeriod).c_str(), nullptr, ImGuiTabItemFlags_NoPushId);
			im::SetItemTooltip("%s", std::format("{:L%B}", period.month()).c_str());

			if (beginTabItem)
			{
				constexpr ImGuiTableFlags flagsTable =
				ImGuiTableFlags_NoSavedSettings        |
				ImGuiTableFlags_Reorderable            |
				ImGuiTableFlags_Resizable              |
				ImGuiTableFlags_Hideable               |
				ImGuiTableFlags_ScrollY                |
				ImGuiTableFlags_ScrollX                |
				ImGuiTableFlags_Borders                |
				ImGuiTableFlags_RowBg                  |
				//ImGuiTableFlags_Sortable               |
				//ImGuiTableFlags_SortMulti              |
				//ImGuiTableFlags_SortTristate           |
				ImGuiTableFlags_SizingFixedFit         | // will have minimal column width possible
				//ImGuiTableFlags_SizingStretchProp      | // will have odd width (more than cell/column needs)
				//ImGuiTableFlags_ContextMenuInBody      |
				//ImGuiTableFlags_HighlightHoveredColumn |
				0;

				static const std::array labelV {FUND_TYP, COD_INCO, ZAT_AMT, FAKT_AMT, PLANS_AMT}; // C - columns to show
				if (im::BeginTable("csv table", scast<int>(labelV.size()), flagsTable))
				{
					{ // declare column headers
						size_t c = 0;
						im::TableSetupScrollFreeze(0, 1); // pin header (1st row)
						im::TableSetupColumn(labelS[labelV[c++]] /*   FUND_TYP */, ImGuiTableColumnFlags_NoHeaderWidth);
						im::TableSetupColumn(labelS[labelV[c++]] /*   COD_INCO */, ImGuiTableColumnFlags_NoHeaderWidth);
						im::TableSetupColumn(labelS[labelV[c++]] /*   FAKT_AMT */);
						im::TableSetupColumn(labelS[labelV[c++]] /*    ZAT_AMT */);
						im::TableSetupColumn(labelS[labelV[c++]] /*  PLANS_AMT */, ImGuiTableColumnFlags_DefaultHide);
						assert(c == labelV.size());
					}

					// draw column headers (with tooltips)
					im::TableNextRow(ImGuiTableRowFlags_Headers);
					const int columns_count = im::TableGetColumnCount();
					for (int c = 0; c < columns_count; c++)
					{
						if (!im::TableSetColumnIndex(c)) continue;

						const char *const name = im::TableGetColumnName(c);
						im::PushID(c); /*and*/ im::TableHeader(name); /*and*/ im::PopID();

						// tooltip with label description
						if (im::IsItemHovered()) {
							im::BeginTooltip(); /*and>*/ im::Text("%s", labelD[labelV[c]]); /*<and*/ im::EndTooltip();
						}
					}

					// draw rows
					for (/*row*/; row < rowCount && timePeriod == csv.GetCell<std::string>(REP_PERIOD, row); row++)
					{
						size_t c = 0;
						im::TableNextRow();
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /*   FUND_TYP */, row).c_str());
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /*   COD_INCO */, row).c_str()); /*scope*/ { // tooltip
							if (im::IsItemHovered()) {
								im::BeginTooltip();   /*and*/ im::PushTextWrapPos(im::GetCursorPos().x + 600.f);
								im::Text("%s", csv.GetCell<std::string>(NAME_INC, row).c_str());
								im::PopTextWrapPos(); /*and*/ im::EndTooltip();
							}
						}
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /*   FAKT_AMT */, row).c_str());
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /*    ZAT_AMT */, row).c_str());
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /*  PLANS_AMT */, row).c_str());
						assert(c == labelV.size());
					}

					im::EndTable();
				}
				else while (++row < rowCount && timePeriod == csv.GetCell<std::string>(REP_PERIOD, row));

				im::EndTabItem();
			}
			else while (++row < rowCount && timePeriod == csv.GetCell<std::string>(REP_PERIOD, row));

			countPeriod++;
		}

		im::EndTabBar();
	}
}
void DrawCsvTableExpProgS(const rcv::Document &csv)
{
	//using namespace UA::budget;
	using namespace UA::budget::expenses::program;

	if (csv.GetRowCount() == 0) {
		UA::budget::DrawEmptyTable(labelS, {REP_PERIOD, COD_CONS_MB_PK, COD_CONS_MB_FK, ZAT_AMT, FAKT_AMT, PLANS_AMT});
		return;
	}

	constexpr ImGuiTabBarFlags flagsTabBar =
		ImGuiTabBarFlags_None                    |
		ImGuiTabBarFlags_NoTooltip               | // custom tooltip replaces this anyways
		ImGuiTabBarFlags_FittingPolicyScroll     | // no squash - no tooltip with full name
		ImGuiTabBarFlags_DrawSelectedOverline    | // a visual nimb over the selected tab for better differentiation
		//ImGuiTabBarFlags_Reorderable             |
		//ImGuiTabBarFlags_FittingPolicyResizeDown |
		0;

	std::chrono::year_month period; // holds REP_PERIOD (converted)
	auto timePeriod = csv.GetCell<std::string>(REP_PERIOD, 0);
	bp::chrono::from_stream(timePeriod, "%m.%Y", period);

	if (im::BeginTabBar(__func__, flagsTabBar))
	{
		// Display `year` as leading tab (button)
		im::TabItemButton(std::format("{}:", bp::chrono::get(period.year())).c_str(), ImGuiTabItemFlags_Leading  /*| ImGuiTabItemFlags_NoTooltip*/);
		//im::TabItemButton(          "(month/quarter)"                             , ImGuiTabItemFlags_Trailing /*| ImGuiTabItemFlags_NoTooltip*/);

		size_t countPeriod = 0;
		std::string timePeriod;
		const size_t rowCount = csv.GetRowCount();

		for (size_t row = 0; row < rowCount; /*++*/) // display months/quarters as tabs with a table (with recpective data part)
		{
			timePeriod = csv.GetCell<std::string>(REP_PERIOD, row);
			bp::chrono::from_stream(timePeriod, "%m.%Y", period);

			//const bool beginTabItem = im::BeginTabItem(std::to_string(bp::chrono::get(period.month())).c_str(), nullptr, ImGuiTabItemFlags_NoPushId);
			const bool beginTabItem = im::BeginTabItem(std::format("{}##{}", bp::chrono::get(period.month()), countPeriod).c_str(), nullptr, ImGuiTabItemFlags_NoPushId);
			im::SetItemTooltip("%s", std::format("{:L%B}", period.month()).c_str());

			if (beginTabItem)
			{
				constexpr ImGuiTableFlags flagsTable =
				ImGuiTableFlags_NoSavedSettings        |
				ImGuiTableFlags_Reorderable            |
				ImGuiTableFlags_Resizable              |
				ImGuiTableFlags_Hideable               |
				ImGuiTableFlags_ScrollY                |
				ImGuiTableFlags_ScrollX                |
				ImGuiTableFlags_Borders                |
				ImGuiTableFlags_RowBg                  |
				//ImGuiTableFlags_Sortable               |
				//ImGuiTableFlags_SortMulti              |
				//ImGuiTableFlags_SortTristate           |
				ImGuiTableFlags_SizingFixedFit         | // will have minimal column width possible
				//ImGuiTableFlags_SizingStretchProp      | // will have odd width (more than cell/column needs)
				//ImGuiTableFlags_ContextMenuInBody      |
				//ImGuiTableFlags_HighlightHoveredColumn |
				0;

				static const std::array labelV {FUND_TYP, COD_CONS_MB_PK, COD_CONS_MB_FK, ZAT_AMT, FAKT_AMT, PLANS_AMT}; // C - columns to show
				if (im::BeginTable("csv table", scast<int>(labelV.size()), flagsTable))
				{
					{ // declare column headers
						size_t c = 0;
						im::TableSetupScrollFreeze(0, 1); // pin header (1st row)
						im::TableSetupColumn(labelS[labelV[c++]] /* FUND_TYP       */, ImGuiTableColumnFlags_NoHeaderWidth);
						im::TableSetupColumn(labelS[labelV[c++]] /* COD_CONS_MB_PK */, ImGuiTableColumnFlags_NoHeaderWidth);
						im::TableSetupColumn(labelS[labelV[c++]] /* COD_CONS_MB_FK */, ImGuiTableColumnFlags_NoHeaderWidth);
						im::TableSetupColumn(labelS[labelV[c++]] /* FAKT_AMT       */);
						im::TableSetupColumn(labelS[labelV[c++]] /* ZAT_AMT        */);
						im::TableSetupColumn(labelS[labelV[c++]] /* PLANS_AMT      */, ImGuiTableColumnFlags_DefaultHide);
						assert(c == labelV.size());
					}

					// draw column headers (with tooltips)
					im::TableNextRow(ImGuiTableRowFlags_Headers);
					const int columns_count = im::TableGetColumnCount();
					for (int c = 0; c < columns_count; c++)
					{
						if (!im::TableSetColumnIndex(c)) continue;

						const char *const name = im::TableGetColumnName(c);
						im::PushID(c); /*and*/ im::TableHeader(name); /*and*/ im::PopID();

						// tooltip with label description
						if (im::IsItemHovered()) {
							im::BeginTooltip(); /*and>*/ im::Text("%s", labelD[labelV[c]]); /*<and*/ im::EndTooltip();
						}
					}

					// draw rows
					for (/*row*/; row < rowCount && timePeriod == csv.GetCell<std::string>(REP_PERIOD, row); row++)
					{
						size_t c = 0;
						im::TableNextRow();
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /* FUND_TYP       */, row).c_str());
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /* COD_CONS_MB_PK */, row).c_str()); /*scope*/ { // tooltip
							if (im::IsItemHovered()) {
								im::BeginTooltip();   /*and*/ im::PushTextWrapPos(im::GetCursorPos().x + 600.f);
								im::Text("%s", csv.GetCell<std::string>(COD_CONS_MB_PK_NAME, row).c_str());
								im::PopTextWrapPos(); /*and*/ im::EndTooltip();
							}
						}
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /* COD_CONS_MB_FK */, row).c_str()); /*scope*/ { // tooltip
							if (im::IsItemHovered()) {
								im::BeginTooltip();   /*and*/ im::PushTextWrapPos(im::GetCursorPos().x + 600.f);
								im::Text("%s", csv.GetCell<std::string>(COD_CONS_MB_FK_NAME, row).c_str());
								im::PopTextWrapPos(); /*and*/ im::EndTooltip();
							}
						}
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /* FAKT_AMT       */, row).c_str());
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /* ZAT_AMT        */, row).c_str());
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /* PLANS_AMT      */, row).c_str());
						assert(c == labelV.size());
					}

					im::EndTable();
				}
				else while (++row < rowCount && timePeriod == csv.GetCell<std::string>(REP_PERIOD, row));

				im::EndTabItem();
			}
			else while (++row < rowCount && timePeriod == csv.GetCell<std::string>(REP_PERIOD, row));

			countPeriod++;
		}

		im::EndTabBar();
	}
}

} // namespace test


// Stash //
