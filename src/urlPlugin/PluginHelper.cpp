#include "PluginHelper.h"
#include "resource.h"
#include "AboutDlg.h"
#include "URL.h"
#include "Profile.h"
#include <StringHelper.h>
#include <tchar.h>

PluginHelper* PluginHelper::Callback::m_pPluginHelper = nullptr;

PluginHelper::PluginHelper() : m_shortcutCommands(nTotalCommandCount)
{
	PluginHelper::Callback::m_pPluginHelper = this;
}

void PluginHelper::PluginInit(HMODULE hModule)
{
	m_hModule = hModule;
}

void PluginHelper::PluginCleanup()
{
}

void PluginHelper::SetInfo(const NppData& notpadPlusData)
{
	m_NppData = notpadPlusData;
	InitCommandMenu();
	InitToolbarIcon();
	InitConfigPath();
}

const TCHAR* PluginHelper::GetPluginName() const
{
	return PLUGIN_NAME;
}

FuncItem* PluginHelper::GetFuncsArray(int* nbF)
{
	*nbF = nTotalCommandCount;
	return m_shortcutCommands.GetFuncItem();
}

void PluginHelper::ProcessNotification(const SCNotification* notifyCode)
{
	switch (notifyCode->nmhdr.code)
	{
	case NPPN_TBMODIFICATION:
	{
		SetMenuIcon();
		break;
	}

	case NPPN_SHUTDOWN:
	{
		PluginCleanup();
		break;
	}

	default:
		return;
	}
}

LRESULT PluginHelper::MessageProc(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return TRUE;
}

BOOL PluginHelper::IsUnicode()
{
#ifdef  _UNICODE
	return TRUE;
#else
	return FALSE;
#endif //  _UNICODE
}

void PluginHelper::SetMenuIcon()
{
	// For Decode URL Toolbar Icon
	SetMenuIcon(m_hMenuIcon, CallBackID::DECODE_URL);
}

void PluginHelper::SetMenuIcon(toolbarIconsWithDarkMode& tbIcons, CallBackID callbackID)
{
	if (tbIcons.hToolbarIcon || tbIcons.hToolbarBmp)
	{
		auto nCommandId = m_shortcutCommands.GetCommandID(callbackID);
		::SendMessage(m_NppData._nppHandle, NPPM_ADDTOOLBARICON_FORDARKMODE, reinterpret_cast<WPARAM&>(nCommandId), reinterpret_cast<LPARAM>(&tbIcons));
	}
}

void PluginHelper::InitCommandMenu()
{
	m_shortcutCommands.SetShortCut(CallBackID::SHOW_SETTING, { true, true, true, 'S' });
	m_shortcutCommands.SetCommand(CallBackID::SHOW_SETTING, MENU_TOOLS_SETTING, Callback::ShowSettingDlg, false);

	m_shortcutCommands.SetShortCut(CallBackID::ENCODE_URL, { true, true, true, 'E' });
	m_shortcutCommands.SetCommand(CallBackID::ENCODE_URL, MENU_ENCODE, Callback::EncodeURL, false);

	m_shortcutCommands.SetShortCut(CallBackID::DECODE_URL, { true, true, true, 'D' });
	m_shortcutCommands.SetCommand(CallBackID::DECODE_URL, MENU_DECODE, Callback::DecodeURL, false);

	m_shortcutCommands.SetCommand(CallBackID::SEP_1, MENU_SEPERATOR, NULL, true);

	m_shortcutCommands.SetCommand(CallBackID::ABOUT, MENU_ABOUT, Callback::ShowAboutDlg, false);
}

void PluginHelper::InitToolbarIcon()
{
	// For Seeting Toolbar Icon
	InitToolbarIcon(m_hMenuIcon, IDI_ICON_TOOLBAR);
}

void PluginHelper::InitToolbarIcon(toolbarIconsWithDarkMode& tbIcons, DWORD iconID)
{
	auto dpi = GetDeviceCaps(GetWindowDC(m_NppData._nppHandle), LOGPIXELSX);
	int size = 16 * dpi / 96;

	tbIcons.hToolbarIcon = reinterpret_cast<HICON>(::LoadImage(static_cast<HINSTANCE>(m_hModule), MAKEINTRESOURCE(iconID), IMAGE_ICON, size, size, 0));
	tbIcons.hToolbarIconDarkMode = reinterpret_cast<HICON>(::LoadImage(static_cast<HINSTANCE>(m_hModule), MAKEINTRESOURCE(iconID), IMAGE_ICON, size, size, 0)); // handle later for dark mode

	ICONINFO iconinfo;
	GetIconInfo(tbIcons.hToolbarIcon, &iconinfo);
	tbIcons.hToolbarBmp = iconinfo.hbmColor;
}

void PluginHelper::InitConfigPath()
{
	// Get config dir path
	WCHAR szPath[_MAX_PATH]{};
	SendMessage(m_NppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, reinterpret_cast<LPARAM>(&szPath));
	m_configPath = std::wstring(szPath) + TEXT("\\") + STR_PROFILE_NAME;
}

void PluginHelper::ToggleMenuItemState(int nCmdId, bool bVisible)
{
	::SendMessage(m_NppData._nppHandle, NPPM_SETMENUITEMCHECK, static_cast<WPARAM>(nCmdId), bVisible);
}

void PluginHelper::ShowSettingDlg()
{
	auto nCmdId = m_shortcutCommands.GetCommandID(CallBackID::SHOW_SETTING);

	if (!m_pSettingsDlg)
	{
		m_pSettingsDlg = std::make_unique<SettingsDlg>(reinterpret_cast<HINSTANCE>(m_hModule), m_NppData._nppHandle, nCmdId, m_configPath);
	}

	if (m_pSettingsDlg)	// Hope it is constructed by now.
	{
		bool bVisible = !m_pSettingsDlg->isVisible();
		m_pSettingsDlg->ShowDlg(bVisible);
		ToggleMenuItemState(nCmdId, bVisible);
	}
}

void PluginHelper::ShowAboutDlg()
{
	auto nCmdId = m_shortcutCommands.GetCommandID(CallBackID::ABOUT);

	if (!m_pAboutDlg)
		m_pAboutDlg = std::make_unique<AboutDlg>(reinterpret_cast<HINSTANCE>(m_hModule), m_NppData._nppHandle, nCmdId);
	bool isShown = m_pAboutDlg->ShowDlg(true);

	ToggleMenuItemState(nCmdId, isShown);
}

void PluginHelper::EncodeURL()
{
	auto selectedText = PreOperation();
	if (selectedText.has_value() && !selectedText->empty())
	{
		EncodeInfo info;
		ProfileEncode(m_configPath).GetInfo(info);

		auto pEncode = std::make_unique<Encode>();
		pEncode->setUrl(selectedText.value());
		pEncode->setExclusion(w2a(info.exclusionSet));
		auto encoded = pEncode->encode(info.bRetainLineFeed);

		PostOperation(selectedText.value(), encoded, &info, true);
	}
}

void PluginHelper::DecodeURL()
{
	auto selectedText = PreOperation();
	if (selectedText.has_value() && !selectedText->empty())
	{
		DecodeInfo info;
		ProfileDecode(m_configPath).GetInfo(info);

		auto pDecoder = std::make_unique<Decode>();
		pDecoder->setUrl(selectedText.value());
		pDecoder->setExclusion(w2a(info.exclusionSet));
		auto decoded = pDecoder->decode();

		PostOperation(selectedText.value(), decoded, &info, false);
	}
}

auto PluginHelper::PreOperation() -> std::optional<std::string>
{
	std::optional<std::string> retVal{ std::nullopt };

	if (InitScintilla())
	{
		auto selectedText = m_pScintillaEditor->GetSelectedText();
		if (selectedText.empty())
			ShowError(ErrorCode::SCINTILLA_COPY);
		else
			retVal = selectedText;
	}

	return retVal;
}

void PluginHelper::PostOperation(const std::string& orgTxt, const std::string& converted, void* convertInfo, bool bEncode)
{
	if (converted.empty())
	{
		ShowError(bEncode ? ErrorCode::CONVERT_ENCODE_UNKNOWN : ErrorCode::CONVERT_DECODE_UNKNOWN);
		return;
	}

	std::string finalConvertedText = converted;
	bool bCopyInSameFile = false;

	if (bEncode)
	{
		EncodeInfo& info = *reinterpret_cast<EncodeInfo*>(convertInfo);
		bCopyInSameFile = info.bCopyInSameFile;
	}
	else
	{
		DecodeInfo& info = *reinterpret_cast<DecodeInfo*>(convertInfo);
		bCopyInSameFile = info.bCopyInSameFile;

		if (info.bBreakLine && !info.strLineBreakDelimiter.empty())
		{
			finalConvertedText.clear();
			finalConvertedText.reserve(converted.length());

			auto delimeter = StringHelper::ToString(info.strLineBreakDelimiter);
			auto allText = StringHelper::Split(converted, delimeter);

			auto len = allText.size();
			for (const auto& eachline : allText)
			{
				--len;
				if (len)
					finalConvertedText += eachline + "\r\n" + delimeter;
				else
					finalConvertedText += eachline;
			}
		}
	}


	if (orgTxt == finalConvertedText)
	{
		ShowError(bEncode ? ErrorCode::CONVERT_ENCODE_SAME : ErrorCode::CONVERT_DECODE_SAME);
		return;
	}

	auto res = bCopyInSameFile ?
		m_pScintillaEditor->ReplaceSelectedText(finalConvertedText) :
		m_pScintillaEditor->PasteSelectedOnNewFile(finalConvertedText);

	if (!res)
	{
		ShowError(bCopyInSameFile ? ErrorCode::SCINTILLA_PASTE : ErrorCode::SCINTILLA_NEW_FILE);
		return;
	}
}

bool PluginHelper::InitScintilla()
{
	if (!m_pScintillaEditor)
	{
		m_pScintillaEditor = std::make_unique<ScintillaEditor>(m_NppData);

		if (!m_pScintillaEditor)
			ShowError(ErrorCode::SCINTILLA_INIT);
	}

	return m_pScintillaEditor ? true : false;
}

void PluginHelper::ShowError(ErrorCode err)
{
	UINT uType = MB_OK;
	std::wstring title = L"Error";
	std::wstring msg = L"An error occured.";
	std::wstring msgSuffix = L"Please contact developer if error continues.";

	switch (err)
	{
	case ErrorCode::SCINTILLA_INIT:
		title = L"Internal Error";
		msg = L"Failed to intialize scintilla. " + msgSuffix;
		break;
	case ErrorCode::SCINTILLA_COPY:
		title = L"Internal Error";
		msg = L"Scintilla error: Failed to copy the text selected txt. \nPlease insure that you've selected the required text and try agian.\n\n" + msgSuffix;
		break;
	case ErrorCode::SCINTILLA_PASTE:
		title = L"Internal Error";
		msg = L"Scintilla error: Failed to paste the text. " + msgSuffix;
		break;
	case ErrorCode::SCINTILLA_NEW_FILE:
		title = L"Internal Error";
		msg = L"Scintilla error: Failed to create new file. " + msgSuffix;
		break;
	case ErrorCode::CONVERT_ENCODE_UNKNOWN:
		title = L"Internal Error";
		msg = L"Failed to encode URL in the selected text. Please try again. " + msgSuffix;
		break;
	case ErrorCode::CONVERT_DECODE_UNKNOWN:
		title = L"Internal Error";
		msg = L"Failed to decode the URL in the selected text. Please try again. " + msgSuffix;
		break;
	case ErrorCode::CONVERT_ENCODE_SAME:
		title = L"Nothing to Convert";
		msg = L"There is nothing to encode for selected URL text or selected text appears to be already encoded.";
		break;
	case ErrorCode::CONVERT_DECODE_SAME:
		title = L"Nothing to Convert";
		msg = L"There is nothing to decode from the selected URL text or selected text appears to be already decoded.";
		break;
	default:
		break;
	}

	::MessageBox(m_NppData._nppHandle, msg.c_str(), title.c_str(), uType);
}

const std::vector<std::string> PluginHelper::w2a(const std::vector<std::wstring>& vec)
{
	std::vector<std::string> ret;

	if (vec.size())
	{
		ret.resize(vec.size());

		for (const auto& item : vec)
			ret.push_back(StringHelper::ToString(item));
	}

	return ret;
}
