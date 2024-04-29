# pragma once

#if !defined(LIB_LIBCONFIG) && !defined(LIB_FIGCONE)
  #warning "none of file configuration libraries enabled, defaults will be used on each start"
#endif

#include "settings.hpp"
#include <imgui/imgui.h>
#include <type_traits>
#include <filesystem>
#include <iostream>

#define FEXTE_CFG ".cfg"
#define FEXTE_INI ".ini"
#define FNAME_CONFIG_SETTINGS "settings"
inline constexpr auto FNAME_CONFIG_SETTINGS_LIBCONFIG = FNAME_CONFIG_SETTINGS FEXTE_CFG;
inline constexpr auto FNAME_CONFIG_SETTINGS_FIGCONE   = FNAME_CONFIG_SETTINGS FEXTE_INI;

void SettingsUnloadLibconfig();
void SettingsReloadLibconfig();
//void SettingsUnloadFigcone();
//void SettingsReloadFigcone();

inline void SettingsUnload() {
	#if defined(LIB_FIGCONE) && defined(LIB_LIBCONFIG)
		SettingsUnloadLibconfig();
		SettingsUnloadFigcone();

	#elif defined(LIB_LIBCONFIG)
		SettingsUnloadLibconfig();

	#elif defined(LIB_FIGCONE)
		SettingsUnloadFigcone();

	#else
		std::cout << "Warn: none of file configuration libraries enabled, defaults will be used on each start\n";
		//#warning ""
	#endif
}
inline void SettingsReload() {
	#if !defined(LIB_LIBCONFIG) && !defined(LIB_FIGCONE)
		std::cout << "Warn: none of file configuration libraries enabled, active settings not saved\n";
	#endif

	#if defined(LIB_FIGCONE)
		SettingsReloadFigcone();
	#elif defined(LIB_LIBCONFIG)
		SettingsReloadLibconfig();
	#endif
}


#ifdef LIB_LIBCONFIG
#include <libconfig.h++>

using lsett_k = libconfig::Setting::Type; // libconfig setting kind

static_assert(std::is_same_v<ImGuiTreeNodeFlags                 , int>);
static_assert(std::is_same_v<decltype(  bp::sett.tree.nodeFlags), int>);

inline void SettingsUnloadLibconfig()
{
	namespace lc = libconfig;

	lc::Config config;

	//config.setDefaultFormat(lc::Setting::Format::FormatHex);
	config.setTabWidth(0); // switch to tabs
	config.setOptions(
		//lc::Config::OptionOpenBraceOnSeparateLine  |
		//lc::Config::OptionSemicolonSeparators      |
		lc::Config::OptionColonAssignmentForGroups |
		0
	);

	lc::Setting &root = config.getRoot();

	// Convert app settings -> (lib)config
	lc::Setting &app = root.add("application", lsett_k::TypeGroup);
	{
		app.add("showImGuiDemoWindow", lsett_k::TypeBoolean) = bp::sett.app.showImGuiDemoWindow;
	}

	lc::Setting &edit = root.add("editor", lsett_k::TypeGroup);
	{
		edit.add("showDebugTextEncoding", lsett_k::TypeBoolean) = bp::sett.edit.showDebugTextEncoding;
	}

	lc::Setting &tree = root.add("tree", lsett_k::TypeGroup);
	{
		tree.add("showFlagsCheckboxes", lsett_k::TypeBoolean) = bp::sett.tree.showFlagsCheckboxes;
		tree.add("nodeFlags"          , lsett_k::TypeInt    ) = bp::sett.tree.nodeFlags;
	}

	lc::Setting &disp = root.add("display", lsett_k::TypeGroup);
	{
		disp.add("depthLevel", lsett_k::TypeBoolean) = bp::sett.disp.depthLevel;
		//disp.add("nodeEnumer", lsett_k::TypeBoolean) = bp::sett.disp.nodeEnumer;
	}

	// Write out the updated configuration
	{
		try {
			config.writeFile(FNAME_CONFIG_SETTINGS_LIBCONFIG);
			std::cerr << "Info: Program settings saved to: " << FNAME_CONFIG_SETTINGS_LIBCONFIG << std::endl;
		}
		catch (const lc::FileIOException &) {
			std::cerr << "Err: I/O error while writing file: " << FNAME_CONFIG_SETTINGS_LIBCONFIG << std::endl;
			return;
		}
	}
}

inline void SettingsReloadLibconfig()
{
	namespace lc = libconfig;

	if (!std::filesystem::exists(FNAME_CONFIG_SETTINGS_LIBCONFIG)) {
		std::cout << "Warn: settings file `" << FNAME_CONFIG_SETTINGS_LIBCONFIG << "` does not exist, using default settings\n";
		return;
	}

	// Load config
	lc::Config config;
	{ // read file
		try	{
			config.readFile(FNAME_CONFIG_SETTINGS_LIBCONFIG);
		}
		catch (const lc::FileIOException &) {
			std::cerr << "Err: I/O error while reading file." << std::endl;
			return;
		}
		catch (const lc::ParseException &pex) {
			std::cerr << "Err: Parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError() << std::endl;
			return;
		}
	}

	// Parse config: define helper-lambda
	auto Lookup = [](const lc::Setting &libSetting, const char *valName, auto &setting) {
		try {
			libSetting.lookupValue(valName, setting);
			//std::cout << "Trace: `" << std::setw(24) << valName << "` loaded: " << setting << "\n";
		}
		catch (const lc::SettingNotFoundException&) { std::cout << "Warn: `" << valName << "` not found in:" << libSetting.getName() << "\n"; }
	};

	// Parse config: read settings
	const lc::Setting &root = config.getRoot();
	try {
		const auto& group = root["application"];
		Lookup(group, "showImGuiDemoWindow", bp::sett.app.showImGuiDemoWindow);
	}
	catch (const lc::SettingNotFoundException&) { std::cerr << "Err: `application` not found\n"; }

	try {
		const auto& group = root["editor"];
		Lookup(group, "showDebugTextEncoding", bp::sett.edit.showDebugTextEncoding);
	}
	catch (const lc::SettingNotFoundException&) { std::cerr << "Err: `editor` not found\n"; }

	try {
		const auto& group = root["tree"];
		Lookup(group, "showFlagsCheckboxes", bp::sett.tree.showFlagsCheckboxes);
		Lookup(group, "nodeFlags"          , bp::sett.tree.nodeFlags);
	}
	catch (const lc::SettingNotFoundException&) { std::cerr << "Err: `tree` not found\n"; }

	try {
		const auto& group = root["display"];
		Lookup(group, "depthLevel", bp::sett.disp.depthLevel);
	}
	catch (const lc::SettingNotFoundException&) { std::cerr << "Err: `display` not found\n"; }
}

#endif // LIB_LIBCONFIG
