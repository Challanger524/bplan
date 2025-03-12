#include "test/csv/CsvGet.hpp"

#include "UA/budget.hpp"
#include "UA/budget/api.hpp"
#include "UA/budget/incomes.hpp"
#include "int/budget/rapidcsv/imgui/imguiDrawEmptyTable.hpp"
#include "bplan/filesystem.hpp"
#include "bplan/chrono.hpp"
#include "bplan/ctype.hpp"
#include "bplan/lenof.hpp"

#include <imgui.h>
#include <rapidcsv.h>

#include <span>
#include <array>
#include <string>
#include <string_view>
//#include <unordered_set>
#include <sstream>
#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
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

namespace test {


//namespace inc   = UA::budget::incomes;
namespace rcv   = rapidcsv    ;
namespace asio  = boost::asio ;
namespace beast = boost::beast;

namespace net {
using namespace boost::asio;
using namespace boost::beast;
}

namespace fs  = std::filesystem;

using namespace UA::budget;
using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

void GetBudget(auto &response, std::string_view request)
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

		const auto      resolved = resolver.resolve(API_HOST, "80");
		tstream.connect(resolved);
		net::http::write(tstream, request);

		net::flat_buffer         buffer;
		net::http::read(tstream, buffer, response);

		tstream.socket().shutdown(net::ip::tcp::socket::shutdown_both);
		//std::cout << "\nTrace:\n" << response << std::endl;
	}
	catch (std::exception& e) { std::cerr << "Exception: " << e.what() << "\n"; throw e; }

}

//void DrawCsvTableExpStatic(const rapidcsv::Document &csv)
void DrawCsvTableIncStatic(const rapidcsv::Document &csv)
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

	if (im::BeginTabBar("csvTabBar", flagsTabBar))
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

				static const std::array labelV {COD_INCO, FUND_TYP, ZAT_AMT, FAKT_AMT, PLANS_AMT}; // C - columns to show
				if (im::BeginTable("csv table", scast<int>(labelV.size()), flagsTable))
				{
					{ // declare column headers
						size_t c = 0;
						im::TableSetupScrollFreeze(0, 1); // pin header (1st row)
						im::TableSetupColumn(labelS[labelV[c++]] /*   COD_INCO */, ImGuiTableColumnFlags_NoHeaderWidth);
						im::TableSetupColumn(labelS[labelV[c++]] /*   FUND_TYP */, ImGuiTableColumnFlags_NoHeaderWidth);
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
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /*   COD_INCO */, row).c_str()); /*scope*/ { // tooltip
							if (im::IsItemHovered()) {
								im::BeginTooltip();   /*and*/ im::PushTextWrapPos(im::GetCursorPos().x + 600.f);
								im::Text("%s", csv.GetCell<std::string>(NAME_INC, row).c_str());
								im::PopTextWrapPos(); /*and*/ im::EndTooltip();
							}
						}
						im::TableNextColumn(); im::TextUnformatted(csv.GetCell<std::string>(labelV[c++] /*   FUND_TYP */, row).c_str());
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

//std::array splitchars = {' ', '-'}; // then use std::ranges - constexpr, but not hashed
//inline const std::unordered_set<char> splitchars = {' ', '.', '+', '-', '*', '/'};

inline std::string ProcessBud(std::span<const char> code) // returns clean 10-digit string
{ // "01.234-56 7\0" -> "0123456700\0"
	std::string processed = BUD_CODE_INIT; // cleaned and expanded budget code
	for (size_t i = 0; i < code.size() - 1; i++) {
		if   (IsDigit(code[i])       ) processed[i] = code[i];
		elif (        code[i] == '\0') break;
	}
	return processed;
}

inline void ZeroExpandBud(std::span<char> code) // modifies input by adding missing zeros
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

std::string ComposeCsvFname(const budget &budget) // Compose csv filename
{
	std::ostringstream oss;
	oss.imbue(std::locale::classic()); // fix clang num separator '1 234' and msvc cringe only first digit streamed '1' from '1234' with further stream breaking (unreadable after '1')

	oss << "local";  //? should be generic option
	oss << '_' << ProcessBud(budget.code);
	oss << '_' << UA::budget::  items   [budget.item  ];
	oss << '_' << budget.year;
	oss << '_' << UA::budget::periodsLow[budget.period];
	//if (budget.item == EXPANSES || budget.item == CREDITS)
	//	oss << '_' << UA::budget::classifs[budget.classif];

	return oss.str();
}

std::string ComposeCsvQuery(const budget &budget) // Compose API query
{ //? https://github.com/boostorg/url?
	std::ostringstream oss;
	oss.imbue(std::locale::classic()); // fix clang num separator '1 234' and msvc cringe only first digit streamed '1' from '1234' with further stream breaking (unreadable after '1')

	oss << '?' << QUERY_BCODE   << '=' << ProcessBud(budget.code);
	oss << '&' << QUERY_BITEM   << '=' << UA::budget::items[budget.item ];
	if (budget.item == EXPANSES || budget.item == CREDITS)
		oss << '&' << QUERY_BCLASS << '=' << UA::budget::classifs[budget.classif];
	oss << '&' << QUERY_BPERIOD << '=' << UA::budget::periods[budget.period];
	oss << '&' << QUERY_BYEAR   << '=' << budget.year;

	return oss.str();
}

void DrawBudgetInput(budget &budget) // Draw budget input forms
{
	static_assert(sizeof(budget::code)/sizeof(char) == BUD_CODE_STRLEN_MIN + 1); // must be at least +1 for `\0`

	// Draw budget input forms

	// budget code
	im::InputText(QUERY_BCODE, budget.code, budget::BUFF_BCODE_SIZE, ImGuiInputTextFlags_CharsDecimal);
	//if (im::Button("test    ProcessBud(code)")) { std::cout <<"Trace: " << ProcessBud(budget.code) << '\n'; }
	//if (im::Button("test ZeroExpandBud(code)")) { ZeroExpandBud(budget.code); }

	// budget item
	im::Combo(QUERY_BITEM, &budget.item, UA::budget::items, IM_ARRAYSIZE(UA::budget::items));

	// classification type
	im::BeginDisabled(budget.item != EXPANSES && budget.item != CREDITS);
	/* free scope */ {
		im::Text("%s:", QUERY_BCLASS);
		im::Indent();

		/*           */ im::RadioButton(classifs[PROGRAM   ], &budget.classif, PROGRAM   ); //  PROGRAM   //
		im::SameLine(); im::RadioButton(classifs[FUNCTIONAL], &budget.classif, FUNCTIONAL); // FUNCTIONAL //

		im::BeginDisabled(budget.item != EXPANSES);
		im::SameLine(); im::RadioButton(classifs[ECONOMICS ], &budget.classif, ECONOMICS ); // ECONOMICS  //
		im::EndDisabled(); //   .item != EXPANSES

		im::BeginDisabled(budget.item != CREDITS);
		im::SameLine(); im::RadioButton(classifs[CREDIT    ], &budget.classif, CREDIT    ); //   CREDIT   //
		im::EndDisabled(); //   .item != CREDITS

		im::Unindent();
	}
	im::EndDisabled(); //   .item != EXPANSES && budget.item != CREDITS

	if   (budget.item == EXPANSES && budget.classif == CREDIT   ) budget.classif = classif_e(0); // reset
	elif (budget.item == CREDITS  && budget.classif == ECONOMICS) budget.classif = classif_e(0); // reset

	// budget period
	im::Text("%s:", QUERY_BPERIOD);
	im::SameLine(); im::RadioButton(periods[MONTH  ], &budget.period, MONTH  );
	im::SameLine(); im::RadioButton(periods[QUARTER], &budget.period, QUARTER);

	// budget year
	im::InputInt(QUERY_BYEAR, &(budget.year));

	if   (budget.year < BUD_YEAR_MIN) budget.year = BUD_YEAR_MIN;
	elif (budget.year > BUD_YEAR_MAX) budget.year = BUD_YEAR_MAX;
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

	if (im::Begin("TEST_CSV_DOWNLOAD", nullptr, wFlags))
	{
		//static budget budget{.code = DEF_BUD_CODE_HRO_CHE_HALF};

		if (im::Button("Open budget..")) im::OpenPopup(NAME_MODAL_INPUT); // button for Modal Input
		this->DrawModalInput();

		// Draw the csv table
		using namespace UA::budget::incomes;
		if (this->csv.GetColumnCount() != 0) { //? replace with `bool`?
			/*
			DrawCsvTableIncDynamic(this->csv);*/
			DrawCsvTableIncStatic (this->csv);
		}
		else UA::budget::DrawEmptyTable(labelS, {COD_INCO/* , FUND_TYP, ZAT_AMT, FAKT_AMT, PLANS_AMT */});

	} im::End();
}

//   Draw budget input forms
//   Show action buttons: ping, execute
//     Execute
//       Compose csv filename based on user input
//       Compose API query based on user input
//   Display some status reporting text
//   Handle case when file present on disk: load, download
//     Load existing file from disk
//     Download up-to-date file from network
void CsvGet::DrawModalInput()
{
	bool openPopup = true;
	im::SetNextWindowPos(im::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2{0.6f, 0.6f});
	if (!im::BeginPopupModal(NAME_MODAL_INPUT, &openPopup, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) return;

	DrawBudgetInput(this->budget);

	static std::string text;
	if (im::Button(API_PATH_PING)) {
		try {
			//net::http::response<net::http::dynamic_body> response;
			net::http::response<net::http::string_body> response;
			GetBudget(response, API_PATH_PING);
			text = "ping response: (" + std::to_string(response.result_int()) + ") " + response.body();
		}
		catch (std::exception& e) { text = "ping attempt fail with: "s + e.what(); }
	}

	static      bool   csvExists = false;
	static  fs::path   csvPath;
	static std::string csvQuery;

	// Work with provided budget input
	/**/im::SameLine();
	if (im::Button("Proceed", ImVec2{im::GetContentRegionAvail().x /*- im::GetStyle().ScrollbarSize*/, 0}))
	{
		assert(budget.item == INCOMES); //? other types are not yet supported

		const std::string csvName = ComposeCsvFname(budget);
		const fs ::path   csvFile = fs::path(csvName).replace_extension(".csv");
		csvPath  = FS_DOWNLOADS / csvFile;
		csvQuery = ComposeCsvQuery(budget);

		// Check if already downloaded
		if (fs::exists(csvPath)) { // check if requested file is present on disk
			csvExists = true;   //! fs::file_size(csvPath) - add human readable wrapper for file  size (not just bytes)
			text = std::format("File already exist on disk:\n{}\n{}\n{} bytes", csvPath.string(), bp::fs::to_string(fs::last_write_time(csvPath)), fs::file_size(csvPath));
		}

	#if 1 // trace:             Downloads/
		std::cout << "\nTrace:           " << csvName;
		std::cout << "\nTrace:           " << csvFile.string();
		std::cout << "\nTrace: "           << csvPath.string();
		std::cout << "\nTrace: "           << csvQuery;
		std::cout << '\n';
	#endif
	}

	// User feedback: Display status reporting text
	if (!text.empty()) {
		im::PushTextWrapPos(im::GetContentRegionAvail().x);
		im::Text("%s", text.c_str());
		im::PopTextWrapPos();
	}

	// Handle case when file present on disk: load, download (refresh)
	if (csvExists)
	{
		// b1: Load existing file from disk
		if (im::Button("load (from disk)")) {
			csvExists = false;
			this->csv.Load(csvPath.string(), rcv::LabelParams(), rcv::SeparatorParams(';', false, true, false, false));
			text = "loaded: from disk";
		}

		// b2: Refresh from network (download & replace)
		/**/im::SameLine();
		if (im::Button("refresh (from network)"))
		{
			csvExists = false;

			#if 1 // dynamic_body
			try {
				// Receive response from network
				net::http::response<net::http::dynamic_body> response;
				GetBudget(response, API_PATH_LOC_BUD_DAT + csvQuery);

				#if 1  // log
				std::cout << "\n";
				std::cout << csvPath << "\n";
				if (const auto it = response.base().find("content-type"       ); it != response.base().end()) std::cout << it->value() << "\n";
				if (const auto it = response.base().find("content-disposition"); it != response.base().end()) std::cout << it->value() << "\n";
				std::cout << "\n" << response.base() << "\n";
				#endif // log

				// Check response, if OK - write received file on disk
				if (response.result() == net::http::status::ok) {
					std::ofstream csvOfstream(csvPath, std::fstream::trunc);
					csvOfstream << net::buffers_to_string(response.body().data());
					csvOfstream.close();

					text = std::format("downloaded from network:\n{}\n{}\n{} bytes", csvPath.string(), bp::fs::to_string(fs::last_write_time(csvPath)), fs::file_size(csvPath));
				}
				else std::runtime_error("Error: " + std::to_string(response.result_int()) + '\n' + net::buffers_to_string(response.body().data()));

				// Validate header params - text/csv, attachment, filename=<value>
				#if 1 // validate header params (content-related)
				// validate: content-type
				if (const auto it = response.base().find("content-type"); it != response.base().end())
					if (it->value().find("text/csv") == beast::string_view::npos)
						throw std::runtime_error("content-type is not 'text/csv'");

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
							//if (value[pos] == '*') { pos += 2; ... }
							if (value[pos] != '=') throw std::runtime_error(std::format("parse_error: value[pos] != '=', it is: {}", value[pos]));
							pos++;
							if (value.find(';', pos) != value.npos) throw std::runtime_error("parse_error: value.find(';') != value.npos, other parameters were not expected");

							filename = value.substr(pos);
						}
						else throw std::runtime_error("content-disposition has no 'filename' parameter");
					}
					else throw std::runtime_error("content-disposition value has no 'attachment' parameter");
				}

				// Compare filenames: expected (composed) filename VS from 'content-disposition' response header parameter
				if (csvPath.filename().string() != filename) {
					std::string report = "Warn: content-disposition has different name: expected vs provided:\n" + csvPath.filename().string() + '\n' + filename;
					std::cout << report << '\n';
					text += "\n\n" + report;
				}
				#endif // validate header params (content-related)

				this->csv.Load(csvPath.string(), rcv::LabelParams(), rcv::SeparatorParams(';', false, true, false, false));
				text = "loaded: from network";
			}
			catch (std::exception& e) { text = "file download attempt fail with: "s + e.what(); }

	#else //    file_body
			fs::remove(csvPath);

			net::error_code ec;
			net::http::response<net::http::file_body> response;
			response.body().open(csvPath.string().c_str(), net::file_mode::write_new, ec);

			if (!ec.failed()) {
				GetBudget(response, PATH_LOC_BUD_DAT + csvQuery);

				const auto it = response.base().find("content-disposition");
				std::cout << "\n";
				std::cout << ec.message() << "\n";

				//std::cout << "\nHeaders " << response.base() << "\n";
				if (response.result() == net::http::status::ok) {
					text = "downloaded from network";
				}
				else
					text = "Error: " + std::to_string(response.result_int());
			}
	#endif // string/file body // also option to switch from `empty_body` after `read_header`: https://stackoverflow.com/questions/76431225/how-to-convert-a-boost-beast-requeststring-body-to-a-requestfile-body

		}
	}

	im::EndPopup();
}


} // namespace test

// call CsvGet()
  // curl -X GET "https://api.openbudget.gov.ua/api/public/localBudgetData?budgetCode=2555900000&budgetItem=INCOMES&period=QUARTER&year=2025" -H "accept: text/csv;charset=windows-1251"
  // https://api.openbudget.gov.ua/api/public/localBudgetData?budgetCode=2555900000&budgetItem=INCOMES&period=QUARTER&year=2025
  /*
	content-disposition: attachment; filename=local_2555900000_INCOMES_2025_quarter.csv
 	content-security-policy: default-src 'self'; prefetch-src 'self' https:; connect-src https:; font-src https: data:; frame-src https:; frame-ancestors https:; img-src https: data:; media-src https: data:; object-src https: blob:; script-src 'self' https: 'unsafe-inline' 'unsafe-eval'; style-src 'self' https: 'unsafe-inline' 'unsafe-eval'; form-action 'self'; base-uri 'self';
 	content-type: text/csv;charset=windows-1251
 	date: Thu, 13 Mar 2025 12:46:21 GMT
 	feature-policy: geolocation 'none'; midi 'none'; microphone 'none'; camera 'none'; magnetometer 'none'; gyroscope 'none'; speaker 'none'; fullscreen 'self'; payment 'none'
 	referrer-policy: no-referrer-when-downgrade
 	server: nginx
 	strict-transport-security: max-age=31536000, max-age=31536000
 	x-content-type-options: nosniff
 	x-frame-options: SAMEORIGIN
 	x-xss-protection: 1; mode=block
  */
 	// "/api/public/localBudgetData?budgetCode=2555900000&budgetItem=INCOMES&period=QUARTER&year=2024"
