#pragma once

#include <optional>
#include <string>
#include <memory>
#include "Define.h"
#include "Notepad_plus_msgs.h"
#include "ShortcutCommand.h"
#include "AboutDlg.h"
#include "SettingsDlg.h"
#include "ScintillaEditor.h"


class PluginHelper
{
public:
	PluginHelper();
	~PluginHelper() = default;

	void PluginInit(HMODULE hModule);
	void PluginCleanup();

	// Notepad++ APIs to be implemented
	void SetInfo(const NppData& notpadPlusData);

	const TCHAR* GetPluginName() const;

	FuncItem* GetFuncsArray(int* nbF);

	void ProcessNotification(const SCNotification* notifyCode);

	LRESULT MessageProc(UINT msg, WPARAM wParam, LPARAM lParam);

	BOOL IsUnicode();

private:
	class Callback
	{
		friend class PluginHelper;
		static PluginHelper* m_pPluginHelper;
	public:
		Callback() = default;
		~Callback() = default;

		static void ShowSettingDlg() { m_pPluginHelper->ShowSettingDlg(); }
		static void ShowAboutDlg() { m_pPluginHelper->ShowAboutDlg(); }
		static void EncodeURL() { m_pPluginHelper->EncodeURL(); }
		static void DecodeURL() { m_pPluginHelper->DecodeURL(); }
	};

	void SetMenuIcon();
	void SetMenuIcon(toolbarIcons& tbIcons, CallBackID callbackID);
	void InitCommandMenu();
	void InitToolbarIcon();
	void InitToolbarIcon(toolbarIcons& tbIcons, DWORD iconID);

	void ToggleMenuItemState(int nCmdId, bool bVisible);

	void ShowSettingDlg();
	void ShowAboutDlg();
	void EncodeURL();
	void DecodeURL();

	auto PreOperation()->std::optional<std::string>;
	void PostOperation(const std::string& orgTxt, const std::string& converted, void* convertInfo, bool bEncode);

	bool InitScintilla();
	void ShowError(ErrorCode err);

	const std::vector<std::string> w2a(const std::vector<std::wstring>& vec);

private:
	HMODULE								m_hModule = nullptr;
	toolbarIcons						m_hMenuIcon = {};
	ShortcutCommand						m_shortcutCommands;
	NppData								m_NppData = {};
	std::unique_ptr<SettingsDlg>		m_pSettingsDlg = nullptr;
	std::unique_ptr<AboutDlg>			m_pAboutDlg = nullptr;
	std::unique_ptr<ScintillaEditor>	m_pScintillaEditor = nullptr;
};
