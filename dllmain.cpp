#include "pch.h"
#include <reshade.hpp>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <map>
#include <vector>
#include <filesystem> // Required for filesystem operations
#include "dllmain.h"
#include <format> // Include for std::format
#include <sstream> // Include for stringstream
#include <Windows.h> // Include for GetCurrentProcessId
#include <cmath> // Include for round function

using json = nlohmann::json;
namespace fs = std::filesystem; // Namespace alias for filesystem

// --- Global Addon State ---
namespace shadingway
{
	// Global addon state variables
	std::string current_preset_path; // Stores the path of the currently active preset
	static bool effects_enabled_state = true; // Tracks whether effects are enabled or disabled
	static bool debug_mode = false; // Debug flag: set to 'true' to enable logging of definitions and static properties

	// --- Function Forward Declarations ---
	void write_options_to_json(reshade::api::effect_runtime* runtime); // Forward declaration

	// --- Display Information Functions ---
	struct DisplayInfo {
		uint32_t width = 0;
		uint32_t height = 0;
		float aspect_ratio = 0.0f;
		std::string screen_type = "unknown";
	};
	
	DisplayInfo get_display_info(reshade::api::effect_runtime* runtime);
	std::string determine_screen_type(uint32_t width, uint32_t height);

	// --- Logging Wrappers ---
	namespace logging {
		void message(reshade::log::level level, const std::string& message);
		void debug(const std::string& message);
		void info(const std::string& message);
		void warning(const std::string& message);
		void error(const std::string& message);
	}

	// --- ReShade Event Handlers ---
	static void on_preset_change(reshade::api::effect_runtime* runtime, const char* path);
	static bool on_technique_state_change(reshade::api::effect_runtime* runtime, reshade::api::effect_technique technique, bool enabled);

	// --- Utility Functions ---
	struct PresetInfo;
	PresetInfo get_detailed_preset_info();
	std::map<std::string, std::string> fetch_preprocessor_data(reshade::api::effect_runtime* runtime, const std::vector<std::string>& names, const char* log_prefix);
	std::pair<std::map<std::string, std::string>, std::map<std::string, std::string>> get_preprocessor_definitions(reshade::api::effect_runtime* runtime);
} // namespace shadingway


// --- DLL Entry Point ---
BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	using namespace shadingway; // Bring namespace shadingway into scope for DllMain

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		if (!reshade::register_addon(hModule)) {
			logging::error("DllMain: Failed to register addon!");
			return FALSE; // DLL initialization failed
		}
		logging::info("DllMain: DLL_PROCESS_ATTACH, addon registered");

		// Register ReShade event handlers
		reshade::register_event<reshade::addon_event::reshade_set_current_preset_path>(on_preset_change);
		reshade::register_event<reshade::addon_event::reshade_set_technique_state>(on_technique_state_change);
		logging::info("DllMain: Registered ReShade events");

		write_options_to_json(nullptr); // Initial state save on addon load
		logging::info("DllMain: Initial state saved to JSON on addon load.");
		break;

	case DLL_PROCESS_DETACH:
		logging::info("DllMain: DLL_PROCESS_DETACH, addon unregistered");

		// Unregister ReShade event handlers
		reshade::unregister_event<reshade::addon_event::reshade_set_current_preset_path>(on_preset_change);
		reshade::unregister_event<reshade::addon_event::reshade_set_technique_state>(on_technique_state_change);
		logging::info("DllMain: Unregistered ReShade events");

		reshade::unregister_addon(hModule); // Unregister the addon from ReShade
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break; // Thread attach/detach events are not handled in this addon
	}
	return TRUE; // DLL entry point success
}


// --- ReShade Event Handlers ---
namespace shadingway
{
	struct PresetInfo {
		std::string path;
		std::string name;
		std::string collection;
	};

	// Explicitly define NAME and DESCRIPTION *inside* the namespace
	const char* NAME = "Shadingway"; // Addon name
	const char* DESCRIPTION = "Logs ReShade preset path, preset name, preset collection, effect status, screen resolution to a JSON file. Preprocessor definitions and static properties are logged only when debug mode is enabled.";
	const char* AUTHOR = "Leon Aquitaine"; // Addon author

	static void on_preset_change(reshade::api::effect_runtime* runtime, const char* path)
	{
		logging::info(std::format("on_preset_change: Event triggered, new preset path: {}", (path ? path : "(nullptr)")));
		current_preset_path = path ? path : ""; // Update preset path, handle null path (preset unloaded)
		write_options_to_json(runtime); // Write state to JSON on preset change
		logging::info("on_preset_change: Finished");
	}

	static bool on_technique_state_change(reshade::api::effect_runtime* runtime, reshade::api::effect_technique technique, bool enabled)
	{
		write_options_to_json(runtime); // Write state to JSON on technique state change
		logging::info("on_technique_state_change: Finished");
		return false;
	}


	// --- JSON Output Function ---
	void write_options_to_json(reshade::api::effect_runtime* runtime)
	{
		logging::info("write_options_to_json: Started");

		PresetInfo presetInfo = get_detailed_preset_info();
		DWORD processId = GetCurrentProcessId(); // Get current process ID

		json outputJson;

		outputJson["pid"] = processId; // Add process ID to JSON as "pid"
		json presetJson;
		presetJson["path"] = presetInfo.path;
		presetJson["name"] = presetInfo.name;
		presetJson["collection"] = presetInfo.collection;
		outputJson["preset"] = presetJson;
		outputJson["effects"] = {
			{"enabled", effects_enabled_state}
		};
		
		// Add display information to the JSON output
		DisplayInfo displayInfo = get_display_info(runtime);
		json displayJson;
		displayJson["width"] = displayInfo.width;
		displayJson["height"] = displayInfo.height;
		displayJson["aspect_ratio"] = round(displayInfo.aspect_ratio * 1000) / 1000.0f; // Round to 3 decimal places
		displayJson["screen_type"] = displayInfo.screen_type;
		outputJson["display"] = displayJson;

		if (debug_mode)
		{
			logging::debug("write_options_to_json: Debug mode is ON, logging definitions and static properties.");
			json definitionsJson;
			json staticPropertiesJson;
			std::pair<std::map<std::string, std::string>, std::map<std::string, std::string>> preprocessorData = get_preprocessor_definitions(runtime);
			std::map<std::string, std::string> preprocessorDefinitions = preprocessorData.first;
			std::map<std::string, std::string> staticProperties = preprocessorData.second;

			for (const auto& pair : preprocessorDefinitions) {
				definitionsJson[pair.first] = pair.second;
			}
			outputJson["definitions"] = definitionsJson;

			for (const auto& pair : staticProperties) {
				staticPropertiesJson[pair.first] = pair.second;
			}
			outputJson["static_properties"] = staticPropertiesJson;
		}
		else
		{
			logging::debug("write_options_to_json: Debug mode is OFF, skipping definitions and static properties logging.");
		}

		std::string filePath = "shadingway.addon-state.json"; // Output JSON file path
		std::string absoluteFilePath = fs::absolute(filePath).string(); // Get absolute file path for logging

		logging::info(std::format("write_options_to_json: State file: {}", absoluteFilePath));

		std::ofstream outputFile(filePath);
		outputFile << outputJson.dump(4); // Write JSON to file, with indentation for readability

		logging::info("write_options_to_json: Wrote state to JSON file");
		logging::info("write_options_to_json: Finished");
	}


	// --- Utility Function Implementations ---



	PresetInfo get_detailed_preset_info()
	{
		PresetInfo info;
		info.path = current_preset_path;
		info.name = "(unknown)";
		info.collection = "(unknown)";

		logging::info("get_detailed_preset_info: Started");
		if (current_preset_path.empty()) {
			logging::warning("get_detailed_preset_info: current_preset_path is empty.");
			return info;
		}

		info.path = current_preset_path;

		// Extract preset name and collection from path string
		size_t lastSlash = current_preset_path.find_last_of("\\/");
		std::string filename = (lastSlash == std::string::npos) ? current_preset_path : current_preset_path.substr(lastSlash + 1);
		size_t iniExtensionPos = filename.rfind(".ini");
		info.name = (iniExtensionPos != std::string::npos) ? filename.substr(0, iniExtensionPos) : filename;

		size_t collectionStartPos = current_preset_path.find("reshade-presets");
		if (collectionStartPos != std::string::npos) {
			size_t collectionPathStart = collectionStartPos + std::string("reshade-presets").length();
			if (collectionPathStart < current_preset_path.length()) {
				std::string collectionPath = current_preset_path.substr(collectionPathStart);
				if (collectionPath.starts_with("\\") || collectionPath.starts_with("/"))
					collectionPath = collectionPath.substr(1);

				std::stringstream ss(collectionPath);
				std::string segment;
				if (std::getline(ss, segment, '\\') || std::getline(ss, segment, '/'))
					info.collection = segment;
				else
					info.collection = collectionPath;
			}
		}

		logging::info(std::format("get_detailed_preset_info: Path: {}, Preset Name: {}, Collection: {}", info.path, info.name, info.collection));
		logging::info("get_detailed_preset_info: Finished");
		return info;
	}


	std::map<std::string, std::string> fetch_preprocessor_data(reshade::api::effect_runtime* runtime, const std::vector<std::string>& names, const char* log_prefix)
	{
		std::map<std::string, std::string> data_map;
		if (runtime == nullptr) {
			logging::warning(std::format("fetch_preprocessor_data: {} runtime is nullptr, returning empty map", log_prefix));
			return data_map;
		}

		char value_buffer[256];
		size_t value_size;
		bool success;

		for (const std::string& name : names)
		{
			value_size = sizeof(value_buffer) - 1;
			success = runtime->get_preprocessor_definition(name.c_str(), value_buffer, &value_size);
			if (success)
			{
				data_map[name] = value_buffer;
			}
			else
			{
				data_map[name] = "not_found";
			}
		}
		return data_map;
	}


	std::pair<std::map<std::string, std::string>, std::map<std::string, std::string>> get_preprocessor_definitions(reshade::api::effect_runtime* runtime)
	{
		logging::info("get_preprocessor_definitions: Started");
		std::map<std::string, std::string> definitions_map;
		std::map<std::string, std::string> static_properties_map;

		std::vector<std::string> definition_names = {
			"RESHADE_DEPTH_LINEARIZATION_FAR_PLANE",
			"RESHADE_DEPTH_INPUT_IS_UPSIDE_DOWN",
			"RESHADE_DEPTH_INPUT_IS_REVERSED",
			"RESHADE_DEPTH_INPUT_IS_LOGARITHMIC",
			"_Copyright_Texture_Source",
			"MXAO_SMOOTHNORMALS"
		};

		std::vector<std::string> static_property_names = {
			"__VENDOR__",
			"__DEVICE__",
			"__RENDERER__",
			"__APPLICATION__",
			"RESHADE_VERSION",
			"RESHADE_API_VERSION"
		};

		definitions_map = fetch_preprocessor_data(runtime, definition_names, "Definitions");
		static_properties_map = fetch_preprocessor_data(runtime, static_property_names, "Static Properties");

		logging::info("get_preprocessor_definitions: Finished, returning definitions and static properties");
		return std::make_pair(definitions_map, static_properties_map);
	}


	// --- Display Information Functions ---
	DisplayInfo get_display_info(reshade::api::effect_runtime* runtime)
	{
		DisplayInfo info;
		
		if (runtime == nullptr) {
			logging::warning("get_display_info: Runtime is nullptr, returning default values");
			return info;
		}
		
		// Get current resolution from ReShade
		uint32_t width = 0, height = 0;
		runtime->get_screenshot_width_and_height(&width, &height);
		
		info.width = width;
		info.height = height;
		
		// Calculate aspect ratio
		if (height > 0) {
			info.aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
		}
		
		// Determine screen type based on resolution
		info.screen_type = determine_screen_type(width, height);
		
		logging::info(std::format("get_display_info: Resolution: {}x{}, Aspect Ratio: {:.3f}, Screen Type: {}", 
			width, height, info.aspect_ratio, info.screen_type));
		
		return info;
	}
	
	std::string determine_screen_type(uint32_t width, uint32_t height)
	{
		// If either dimension is zero, return "unknown"
		if (width == 0 || height == 0) {
			return "unknown";
		}
		
		// Calculate total pixels (roughly corresponds to marketing terms)
		const uint64_t total_pixels = static_cast<uint64_t>(width) * static_cast<uint64_t>(height);
		
		// Determine screen type based on resolution heuristics
		if (total_pixels >= 33177600) { // 8K (7680x4320)
			return "8k";
		}
		else if (total_pixels >= 8294400) { // 4K (3840x2160)
			return "4k";
		}
		else if (total_pixels >= 3686400) { // 2K/1440p (2560x1440)
			return "2k";
		}
		else if (total_pixels >= 2073600) { // 1080p (1920x1080)
			return "1080p";
		}
		else if (total_pixels >= 921600) { // 720p (1280x720)
			return "720p";
		}
		else if (total_pixels >= 480000) { // 480p (800x600)
			return "480p";
		}
		else {
			return "low-res";
		}
	}


	// --- Logging Wrapper Implementations ---
	namespace logging {
		void message(reshade::log::level level, const std::string& message)
		{
			reshade::log::message(level, message.c_str());
		}

		void debug(const std::string& message)
		{
			if (debug_mode)
				logging::message(reshade::log::level::debug, message);
		}

		void info(const std::string& message)
		{
			logging::message(reshade::log::level::info, message);
		}

		void warning(const std::string& message)
		{
			logging::message(reshade::log::level::warning, message);
		}

		void error(const std::string& message)
		{
			logging::message(reshade::log::level::error, message);
		}
	}

} // namespace shadingway


// Explicitly export NAME and DESCRIPTION for ReShade
extern "C" {
	__declspec(dllexport) const char* NAME = shadingway::NAME;
	__declspec(dllexport) const char* DESCRIPTION = shadingway::DESCRIPTION;
	__declspec(dllexport) const char* AUTHOR = shadingway::AUTHOR;
}