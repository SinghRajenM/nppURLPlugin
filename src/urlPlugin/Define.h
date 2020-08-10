#pragma once
#include "PluginInterface.h"
#include <string>
#include <vector>

// Define the number of plugin commands here
constexpr int nTotalCommandCount = 5;

// Define plugin name here
constexpr TCHAR PLUGIN_NAME[] = TEXT("URL Plugin");

// Text which can be considered for localization
constexpr TCHAR TITLE_TOOLS_SETTING[] = TEXT("URL Plugin Settings");

constexpr TCHAR MENU_TOOLS_SETTING[] = TEXT("URL Plugin &Settings");
constexpr TCHAR MENU_ENCODE[] = TEXT("&Encode URL");
constexpr TCHAR MENU_DECODE[] = TEXT("&Decode URL");
constexpr TCHAR MENU_ABOUT[] = TEXT("&About");
constexpr TCHAR MENU_SEPERATOR[] = TEXT("-SEPARATOR-");

constexpr TCHAR URL_SOURCE_CODE[] = TEXT("https://github.com/SinghRajenM");
constexpr TCHAR URL_REPORT_ISSUE[] = TEXT("https://github.com/SinghRajenM");

constexpr TCHAR STR_VERSION[] = TEXT("Version: ");

constexpr TCHAR STR_PROFILE_NAME[] = TEXT("URLPlugin.ini");
constexpr TCHAR STR_PROFILE_PATH[] = TEXT("\\Notepad++\\plugins\\URLPlugin");

constexpr TCHAR STR_CONVERSION_SEPARATOR[] = TEXT(";");

constexpr TCHAR STR_INI_ENCODE_SEC[] = TEXT("ENCODE");
constexpr TCHAR STR_INI_ENCODE_SAMEFILE[] = TEXT("REPLACE_IN_SAME_FILE");
constexpr TCHAR STR_INI_ENCODE_CONTEXT[] = TEXT("ENABLE_CONTEXT");
constexpr TCHAR STR_INI_ENCODE_REMOVE_FEED[] = TEXT("REMOVE_FEED");
constexpr TCHAR STR_INI_ENCODE_EXCLUDE_TEXT[] = TEXT("EXCLUDE_TEXT");

constexpr TCHAR STR_INI_DECODE_SEC[] = TEXT("DECODE");
constexpr TCHAR STR_INI_DECODE_SAMEFILE[] = TEXT("REPLACE_IN_SAME_FILE");
constexpr TCHAR STR_INI_DECODE_CONTEXT[] = TEXT("ENABLE_CONTEXT");
constexpr TCHAR STR_INI_DECODE_BREAKLINE[] = TEXT("BREAKLINES");
constexpr TCHAR STR_INI_DECODE_BREAK_DELIMITER[] = TEXT("LINE_BREAK_DELIMITER");
constexpr TCHAR STR_INI_DECODE_EXCLUDE_TEXT[] = TEXT("EXCLUDE_TEXT");

struct CommonInfo
{
	bool bCopyInSameFile = false;
	bool bEnableContextMenu = false;
};

struct EncodeInfo :CommonInfo
{
	bool bRemoveLineFeed = false;
	std::vector<std::wstring> exclusionSet;
};

struct DecodeInfo :CommonInfo
{
	bool bBreakLine = false;
	std::wstring strLineBreakDelimiter;
	std::vector<std::wstring> exclusionSet;
};

enum class ErrorCode {
	SCINTILLA_INIT,
	SCINTILLA_COPY,
	SCINTILLA_PASTE,
	SCINTILLA_NEW_FILE,

	CONVERT_ENCODE_UNKNOWN,
	CONVERT_DECODE_UNKNOWN,

	CONVERT_ENCODE_SAME,
	CONVERT_DECODE_SAME,
};
