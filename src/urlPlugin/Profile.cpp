#include "Profile.h"
#include "Utility.h"
#include "StringHelper.h"
#include <shlobj.h>
#include <memory>

Profile::Profile()
{
	Init();
}

bool Profile::ReadValue(const std::wstring& section, const std::wstring& key, std::wstring& retVal) const
{
	bool bRetVal = false;

	// Try with MAX_PATH
	constexpr DWORD nBufSize = MAX_PATH * 2;
	auto pData = std::make_unique<TCHAR[]>(nBufSize);
	GetPrivateProfileString(section.c_str(), key.c_str(), nullptr, pData.get(), nBufSize, m_ProfileFilePath.c_str());

	if (pData)
	{
		bRetVal = true;
		retVal = pData.get();
	}

	return bRetVal;
}

bool Profile::WriteValue(const std::wstring& section, const std::wstring& key, const std::wstring& value) const
{
	return WritePrivateProfileString(section.c_str(), key.c_str(), value.c_str(), m_ProfileFilePath.c_str()) ? true : false;
}

void Profile::Init()
{
	auto appDatapath = CUtility::GetSpecialFolderLocation(CSIDL_APPDATA);
	if (appDatapath.empty())
	{
		::MessageBox(NULL, L"Failed to get %appdata% path. Please contact developer. Inconvenience regretted.", L"Unhandled Error", MB_OK);
		return;
	}

	appDatapath += STR_PROFILE_PATH;
	if (!CUtility::DirExist(appDatapath) && !CUtility::CreateDir(appDatapath))
	{
		std::wstring msg = L"Failed to get below directory. Please contact developer. Inconvenience regretted.";
		msg += L"\n\n" + appDatapath;

		::MessageBox(NULL, msg.c_str(), L"Unhandled Error", MB_OK);
		return;
	}

	m_ProfileFilePath = appDatapath + L"\\" + STR_PROFILE_NAME;
}

bool ProfileEncode::GetInfo(EncodeInfo& info) const
{
	bool bRetVal = true;

	std::wstring txt;
	bRetVal &= ReadValue(STR_INI_ENCODE_SEC, STR_INI_ENCODE_SAMEFILE, txt);
	if (bRetVal && txt == L"1")
		info.bCopyInSameFile = true;

	bRetVal &= ReadValue(STR_INI_ENCODE_SEC, STR_INI_ENCODE_CONTEXT, txt);
	if (bRetVal && txt == L"1")
		info.bEnableContextMenu = true;

	txt = L"";
	bRetVal &= ReadValue(STR_INI_ENCODE_SEC, STR_INI_ENCODE_REMOVE_FEED, txt);
	if (bRetVal && txt == L"1")
		info.bRemoveLineFeed = true;

	txt = L"";
	bRetVal &= ReadValue(STR_INI_ENCODE_SEC, STR_INI_ENCODE_EXCLUDE_TEXT, txt);
	if (bRetVal && !txt.empty())
		info.exclusionSet = StringHelper::Split(txt, L";");

	return bRetVal;
}

bool ProfileEncode::SetInfo(const EncodeInfo& info) const
{
	bool bRetVal = true;

	bRetVal &= WriteValue(STR_INI_ENCODE_SEC, STR_INI_ENCODE_SAMEFILE, info.bCopyInSameFile ? L"1" : L"0");
	bRetVal &= WriteValue(STR_INI_ENCODE_SEC, STR_INI_ENCODE_CONTEXT, info.bEnableContextMenu ? L"1" : L"0");
	bRetVal &= WriteValue(STR_INI_ENCODE_SEC, STR_INI_ENCODE_REMOVE_FEED, info.bRemoveLineFeed ? L"1" : L"0");

	std::wstring txt;
	for (const auto& ch : info.exclusionSet)
		txt += ch + STR_CONVERSION_SEPARATOR;
	bRetVal &= WriteValue(STR_INI_ENCODE_SEC, STR_INI_ENCODE_EXCLUDE_TEXT, txt);

	return bRetVal;
}

bool ProfileDecode::GetInfo(DecodeInfo& info) const
{
	bool bRetVal = true;

	std::wstring txt;
	bRetVal &= ReadValue(STR_INI_DECODE_SEC, STR_INI_DECODE_SAMEFILE, txt);
	if (bRetVal && txt == L"1")
		info.bCopyInSameFile = true;

	bRetVal &= ReadValue(STR_INI_DECODE_SEC, STR_INI_DECODE_CONTEXT, txt);
	if (bRetVal && txt == L"1")
		info.bEnableContextMenu = true;

	txt = L"";
	bRetVal &= ReadValue(STR_INI_DECODE_SEC, STR_INI_DECODE_BREAKLINE, txt);
	if (bRetVal && txt == L"1")
		info.bBreakLine = true;

	txt = L"";
	bRetVal &= ReadValue(STR_INI_DECODE_SEC, STR_INI_DECODE_BREAK_DELIMITER, txt);
	if (bRetVal && !txt.empty())
		info.strLineBreakDelimiter = txt;

	txt = L"";
	bRetVal &= ReadValue(STR_INI_DECODE_SEC, STR_INI_ENCODE_EXCLUDE_TEXT, txt);
	if (bRetVal && !txt.empty())
		info.exclusionSet = StringHelper::Split(txt, L";");

	return bRetVal;
}

bool ProfileDecode::SetInfo(const DecodeInfo& info) const
{
	bool bRetVal = true;

	bRetVal &= WriteValue(STR_INI_DECODE_SEC, STR_INI_DECODE_SAMEFILE, info.bCopyInSameFile ? L"1" : L"0");
	bRetVal &= WriteValue(STR_INI_DECODE_SEC, STR_INI_DECODE_CONTEXT, info.bEnableContextMenu ? L"1" : L"0");
	bRetVal &= WriteValue(STR_INI_DECODE_SEC, STR_INI_DECODE_BREAKLINE, info.bBreakLine ? L"1" : L"0");
	bRetVal &= WriteValue(STR_INI_DECODE_SEC, STR_INI_DECODE_BREAK_DELIMITER, info.strLineBreakDelimiter);

	std::wstring txt;
	for (const auto& ch : info.exclusionSet)
		txt += ch + STR_CONVERSION_SEPARATOR;
	bRetVal &= WriteValue(STR_INI_DECODE_SEC, STR_INI_DECODE_EXCLUDE_TEXT, txt);

	return bRetVal;
}