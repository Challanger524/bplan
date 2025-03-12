#include "test/csv/CsvGet.hpp"

#include <imgui.h>

#include <array>
#include <string>
#include <string_view>
#include <sstream>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <assert.h>

#include <util/wignore-push.inl>
//#  include <boost/asio.hpp>
#  include <boost/asio/ip/tcp.hpp>
#  include <boost/asio/connect.hpp>
//#  include <boost/beast.hpp>
#  include <boost/beast/core.hpp>
#  include <boost/beast/http.hpp>
#  include <boost/beast/version.hpp>
#include <util/wignore-pop.inl>

//namespace asio  = boost::asio;
namespace beast = boost::beast;

namespace net {
using namespace boost::asio;
using namespace boost::beast;
}


namespace test {

namespace fs = std::filesystem;
using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

constexpr int64_t BCODE_MAX = 99999'99999; // max 10 digit number
constexpr auto FS_DOWNLOADS = "Downloads/";

//constexpr auto PORT = "80"                   ;
constexpr auto HOST = "api.openbudget.gov.ua";

constexpr auto PATH_BASE = "/api/public/";

constexpr auto PATH_PING         = "ping"               ;
//constexpr auto PATH_GEN_BUD_DAT  = "generalData"        ;
constexpr auto PATH_LOC_BUD_DAT  = "localBudgetData"    ;
//constexpr auto PATH_LOC_BUD_DOC  = "localBudgetDocDay"  ;
//constexpr auto PATH_LOC_BUD_REP  = "localBudgetReport"  ;
//constexpr auto PATH_LOC_BUD_LOAD = "localBudgetLastLoad";

constexpr auto QUERY_BCODE  = "budgetCode"        ;
constexpr auto QUERY_BITEM  = "budgetItem"        ;
constexpr auto QUERY_CLASS  = "classificationType";
constexpr auto QUERY_PERIOD = "period"            ;
constexpr auto QUERY_YEAR   = "year"              ;

//#define   PORT "80"
//#define   HOST "api.openbudget.gov.ua"
//#define TARGET "/api/public/"
//#define BUDGET "localBudgetData"
//#define     P0 "?budgetCode=2555900000"
//#define     P1 "&budgetItem=INCOMES"
//#define     P2 "&period=QUARTER"
//#define     P3 "&year=2024"

inline std::ostream &operator<<(std::ostream &os, const std::filesystem::file_time_type v) { return os << std::format("{:%F %R}:{:.2}", v, std::format("{:%S}", v)); }

void GetBudget(auto &response, std::string_view request)
{
	const std::string target = std::string(PATH_BASE) + std::string(request);
	std::cout << "Trace: target: " << target << "\n";

	try {
		net::io_context                 context ;
		net::ip::tcp::resolver resolver(context);
		net::tcp_stream        tstream (context);

		net::http::request<net::http::empty_body> request;
		request.method(net::http::verb::get);
		request.version(11);
		request.set(net::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		request.set(net::http::field::host      , HOST);
		request.keep_alive(false);
		request.target(target);

		const auto      resolved = resolver.resolve(HOST, "80");
		tstream.connect(resolved);
		net::http::write(tstream, request);

		net::flat_buffer         buffer;
		net::http::read(tstream, buffer, response);

		tstream.socket().shutdown(net::ip::tcp::socket::shutdown_both);
		//std::cout << "\nTrace:\n" << response << std::endl;
	}
	catch (std::exception& e) { std::cerr << "Exception: " << e.what() << "\n"; throw e; }

}

enum        budgetItemsE : int { INCOMES ,  EXPANSES ,  CREDITS ,  FINANCING_DEBTS ,  FINANCING_CREDITOR };
const char* budgetItems[] =    {"INCOMES", "EXPANSES", "CREDITS", "FINANCING_DEBTS", "FINANCING_CREDITOR"};

enum                 classifTypesE : int { PROGRAM ,  FUNCTIONAL ,  ECONOMICS ,  CREDIT };
constexpr std::array classifTypes =      {"PROGRAM", "FUNCTIONAL", "ECONOMICS", "CREDIT"};

enum                 periodsE : int { QUARTER ,  MONTH };
constexpr std::array periods    =   {"QUARTER", "MONTH"};
constexpr std::array periodsLow =   {"quarter", "month"};

void DrawBudgetInput(int64_t &budgetCode, int &budgetItem, int &classifType, int &period, int &year) // Draw budget input forms
{
	// Draw budget input forms
	im::InputScalar("budgetCode", ImGuiDataType_S64, reinterpret_cast<void*>(&budgetCode));
	if (budgetCode > BCODE_MAX) budgetCode = BCODE_MAX;

	im::Combo( "budgetItem", &budgetItem, budgetItems, IM_ARRAYSIZE(budgetItems));

#if 1 // with radio buttons
	if (budgetItem != EXPANSES && budgetItem != CREDITS) im::BeginDisabled();
	/* scope */ {
		im::Text("%s", "classificationType:");
		im::Indent();

		//size_t c = 0;
		im::RadioButton(classifTypes[PROGRAM   ], &classifType, PROGRAM   ); im::SameLine();
		im::RadioButton(classifTypes[FUNCTIONAL], &classifType, FUNCTIONAL); im::SameLine();

		if (budgetItem != EXPANSES) im::BeginDisabled();
		im::RadioButton(classifTypes[ECONOMICS ], &classifType, ECONOMICS ); im::SameLine();
		if (budgetItem != EXPANSES) im::  EndDisabled();

		if (budgetItem != CREDITS) im::BeginDisabled();
		im::RadioButton(classifTypes[CREDIT    ], &classifType, CREDIT    );
		if (budgetItem != CREDITS) im::  EndDisabled();

		im::Unindent();
	}
	if (budgetItem != EXPANSES && budgetItem != CREDITS) im::  EndDisabled();

	if   (budgetItem == EXPANSES && classifType == CREDIT   ) classifType = 0;
	elif (budgetItem == CREDITS  && classifType == ECONOMICS) classifType = 0;

#else // with two drop lists
	static int classifTypeExpan  = 0;
	static int classifTypeCredit = 0;
	const char* classifTypesExpan [] = {"PROGRAM", "FUNCTIONAL", "ECONOMICS"};
	const char* classifTypesCredit[] = {"PROGRAM", "FUNCTIONAL", "CREDIT"   };

	if (budgetItem == 1) { // EXPANSES
		im::Combo( "classificationType", &classifTypeExpan, classifTypesExpan, IM_ARRAYSIZE(classifTypesExpan));
	}
	else { // CREDITS or disabled
		if (budgetItem != 2) im::BeginDisabled();
		im::Combo( "classificationType", &classifTypeCredit, classifTypesCredit, IM_ARRAYSIZE(classifTypesCredit));
		if (budgetItem != 2) im::  EndDisabled();
	}
#endif

#if 1 // with radio buttons
	im::Text("%s", "period:");
	im::SameLine(); im::RadioButton(periods[QUARTER], &period, QUARTER);
	im::SameLine(); im::RadioButton(periods[MONTH  ], &period, MONTH  );
#else // with drop list
	const char* periods[] = {"QUARTER", "MONTH"};
	im::Combo("period", &period, periods, IM_ARRAYSIZE(periods));
#endif

	im::InputInt("year", &year);

	constexpr int BUD_YEAR_MIN = 2015; // budget year min
	constexpr int BUD_YEAR_MAX = 2035; // budget year max

	if   (year < BUD_YEAR_MIN) year = BUD_YEAR_MIN;
	elif (year > BUD_YEAR_MAX) year = BUD_YEAR_MAX;
}

CsvGet::CsvGet()
{
	if (std::filesystem::exists          (FS_DOWNLOADS) == false)
		std::filesystem::create_directory(FS_DOWNLOADS);
}

// Map:
// im::Begin("TEST_CSV_DOWNLOAD", nullptr, wFlags)
//   Draw budget input forms
//   Show action buttons: ping, execute
//     Execute
//       Compose csv filename based on user input
//       Compose API query based on user input
//   Display some status reporting text
//   Handle case when file present on disk: load, download
//     Load existing file from disk
//     Download up-to-date file from network
void CsvGet::operator()()
{
	constexpr ImGuiWindowFlags wFlags =
		ImGuiWindowFlags_None                  |
		ImGuiWindowFlags_NoCollapse            |
		ImGuiWindowFlags_NoSavedSettings       |
		ImGuiWindowFlags_AlwaysAutoResize      |
		0;

	//im::SetNextWindowSize(ImVec2(600, 400));

	if (im::Begin("TEST_CSV_DOWNLOAD", nullptr, wFlags))
	{
		// Draw budget input forms
		static int64_t budgetCode  = 25559'00000; // chernihiv hromada
		static int     budgetItem  = INCOMES;
		static int     classifType = PROGRAM;
		static int     period      = QUARTER;
		static int     year        = 2025;

		DrawBudgetInput(budgetCode, budgetItem, classifType, period, year);

		// Show action buttons: ping, execute
		static std::string text;

		if (im::Button(PATH_PING)) {
			try {
				//net::http::response<net::http::dynamic_body> response;
				net::http::response<net::http::string_body> response;
				GetBudget(response, PATH_PING);
				text = "ping response: (" + std::to_string(response.result_int()) + ") " + response.body();
			}
			catch (std::exception& e) { text = "ping attempt fail with: "s + e.what(); }
		}

		// Execute button
		const auto &ItemSpacing = im::GetStyle().ItemSpacing;

		static bool csvExists = false;
		static  fs::path   csvPath;
		static std::string csvQuery;

		/**/im::SameLine();
		if (im::Button("Execute", ImVec2{im::GetWindowWidth() - im::CalcTextSize(PATH_PING).x - ItemSpacing.x * 4, 0.f}))
		{
			assert(budgetItem == INCOMES); //? other types are not yet supported

			// Compose csv filename based on user input
			std::string csvName; //? move out to function?
			csvName  = "local";  //? should be generic option
			csvName += '_' + std::to_string(budgetCode);
			csvName += '_' + std::string   (budgetItems [budgetItem ]);
			csvName += '_' + std::to_string(year      );
			csvName += '_' + std::string   (periodsLow  [period     ]);
			//if (budgetItem == EXPANSES || budgetItem == CREDITS)         //?
			//	csvName += '_' + std::string(classifTypes[classifType]); //?

			const fs::path csvFile = fs::path(csvName).replace_extension(".csv");
			/*          */ csvPath = FS_DOWNLOADS / csvFile;

			// Check if already downloaded
			if (fs::exists(csvPath)) { // check if requested file is present on disk
				std::stringstream writeTime;
				writeTime << fs::last_write_time(csvPath);
				text = std::format("File already exist on disk:\n{}\n{}\n{} bytes", csvPath.string(), writeTime.str(), fs::file_size(csvPath));
				//text = std::format("File already exist on disk:\n{}\n{}\n{} bytes", csvPath.string(), fs::last_write_time(csvPath), fs::file_size(csvPath));

				csvExists = true;
			}

			// Compose API query based on user input
			//? move out to function?
			//? https://github.com/boostorg/url?
			csvQuery  = "?"s + QUERY_BCODE  + "="s + std::to_string(budgetCode);
			csvQuery += "&"s + QUERY_BITEM  + "="s + std::string   (budgetItems [budgetItem ]);
			if (budgetItem == EXPANSES || budgetItem == CREDITS)
				csvQuery += "&"s + QUERY_CLASS + "="s + std::string(classifTypes[classifType]);
			csvQuery += "&"s + QUERY_PERIOD + "="s + std::string   (periods     [period     ]);
			csvQuery += "&"s + QUERY_YEAR   + "="s + std::to_string(year      );

#if 1		//                     Downloads/ //!
			std::cout << "\nTrace:           " << csvName;
			std::cout << "\nTrace:           " << csvFile.string();
			std::cout << "\nTrace: "           << csvPath.string();
			std::cout << "\nTrace: "           << csvQuery;
			std::cout << '\n';
#endif
		}

		// Display status reporting text
		if (!text.empty()) {
			im::PushTextWrapPos(im::GetCursorPos().x + im::GetWindowWidth() - ItemSpacing.x * 2);
			im::Text("%s", text.c_str());
			im::PopTextWrapPos();
		}

		// Handle case when file present on disk: load, download (refresh)
		if (csvExists)
		{
			// Load existing file from disk
			if (im::Button("load existing")) {
				csvExists = false;
				//csvPath //! todo
				text = "loaded from disk";
			}

			// Download up-to-date file from network
			/**/im::SameLine();
			if (im::Button("refresh"))
			{
				csvExists = false;
#if 1 // dynamic_body
				try {
					net::http::response<net::http::dynamic_body> response;
					GetBudget(response, PATH_LOC_BUD_DAT + csvQuery);

#if 1
					std::cout << "\n";
					std::cout << csvPath << "\n";
					if (const auto it = response.base().find("content-type"       ); it != response.base().end()) std::cout << it->value() << "\n";
					if (const auto it = response.base().find("content-disposition"); it != response.base().end()) std::cout << it->value() << "\n";
					std::cout << "\n" << response.base() << "\n";
#endif

					if (response.result() == net::http::status::ok) {
						std::ofstream csvOfstream(csvPath, std::fstream::trunc);
						csvOfstream << net::buffers_to_string(response.body().data());
						csvOfstream.close();

						std::stringstream writeTime;
						writeTime << fs::last_write_time(csvPath);
						text = std::format("downloaded from network:\n{}\n{}\n{} bytes", csvPath.string(), writeTime.str(), fs::file_size(csvPath));
					}
					else std::runtime_error("Error: " + std::to_string(response.result_int()) + '\n' + net::buffers_to_string(response.body().data()));

#if 1 // validate header params - text/csv, attachment, filename=
					if (const auto it = response.base().find("content-type"); it != response.base().end())
						if (it->value().find("text/csv") == beast::string_view::npos)
							throw std::runtime_error("content-type is not text/csv");

					std::string filename; //? perfectly, way more cases should be parsed: https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers/Content-Disposition
					if (const auto it = response.base().find("content-disposition"); it != response.base().end()) {
						const auto &value = it->value(); // content-disposition field value
						constexpr auto paramAttachment = "attachment"sv;
						constexpr auto paramFilename   =   "filename"sv;

						size_t pos = 0;
						if (pos = value.find(paramAttachment, pos); pos != value.npos) { // attachment
							pos += paramAttachment.size();

							if (pos = value.find(paramFilename, pos); pos != value.npos) { // filename (only '=' and not '*=')
								pos += paramFilename.size();
								//if (value[pos] == '*') { pos += 2; ... }
								if (value[pos] != '=') throw std::runtime_error(std::format("parse_error: value[pos] != '=', it is: {}", value[pos]));
								pos++;
								if (value.find(';', pos) != value.npos) throw std::runtime_error("parse_error: value.find(';') != value.npos, other parameters were not expected");

								filename = value.substr(pos);
								//if (!filename.starts_with('"')) filename = '"' + filename + '"';
							}
							else throw std::runtime_error("content-disposition has no 'filename' parameter");
						}
						else throw std::runtime_error("content-disposition value has no 'attachment' parameter");
					}

					if (csvPath.filename().string() != filename) {
						std::string report = "Warn: content-disposition has different name: expected vs provided:\n" + csvPath.filename().string() + '\n' + filename;
						std::cout << report << '\n';
						text += "\n\n" + report;
					}
#endif // validate header params

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
#endif // also there are switch from empty_body solution: https://stackoverflow.com/questions/76431225/how-to-convert-a-boost-beast-requeststring-body-to-a-requestfile-body

			}
		}

	} im::End();

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
