#include "SettingsDlg.h"
#include "resource.h"
#include "Utility.h"
#include "StringHelper.h"
#include "Profile.h"
#include <string>
#include <commctrl.h>
#include <Uxtheme.h>


SettingsDlg::SettingsDlg(HINSTANCE hIntance, HWND hParent, int nCmdId) : m_nCmdId(nCmdId), StaticDialog()
{
	init(hIntance, hParent);
}


bool SettingsDlg::ShowDlg(bool bShow)
{
	bool bShouldShow = bShow && !isVisible();
	if (bShouldShow)
	{
		if (!isCreated())
			create(IDD_DLG_SETTING);

		// Adjust the position of AboutBox
		goToCenter();
	}
	else
	{
		SendMessage(_hSelf, WM_COMMAND, IDCANCEL, NULL);
	}
	return bShouldShow;
}

INT_PTR SettingsDlg::run_dlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		::SetWindowLongPtr(_hSelf, DWLP_USER, lParam);

		auto enable_dlg_theme = reinterpret_cast<ETDTProc>(::SendMessage(_hParent, NPPM_GETENABLETHEMETEXTUREFUNC, 0, 0));
		if (enable_dlg_theme != nullptr)
			enable_dlg_theme(_hSelf, ETDT_ENABLETAB);

		InitDlg();

		SetFocus(GetDlgItem(_hSelf, IDOK));

		return TRUE;
	}

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
			if (Apply())
			{
				::SendMessage(_hParent, NPPM_SETMENUITEMCHECK, static_cast<WPARAM>(m_nCmdId), false);
				EndDialog(_hSelf, wParam);
			}
			else
			{
				::MessageBox(_hSelf, L"Failed to save the setting. Please try again.",
					L"Setting Save Error", MB_OK | MB_ICONERROR);
			}
			return TRUE;

		case IDCANCEL: // Close this dialog when clicking to close button
			::SendMessage(_hParent, NPPM_SETMENUITEMCHECK, static_cast<WPARAM>(m_nCmdId), false);
			EndDialog(_hSelf, wParam);
			return TRUE;

		case IDC_CHK_MULTILINE:
			EnableDelimter(CUtility::GetCheckboxStatus(::GetDlgItem(_hSelf, IDC_CHK_MULTILINE)));
			return TRUE;
		}
	}
	}
	return FALSE;
}

bool SettingsDlg::Apply()
{
	// Get all the data from the UI

	// Common setting
	m_EncodeInfo.bCopyInSameFile = CUtility::GetCheckboxStatus(::GetDlgItem(_hSelf, IDC_CHK_SAME_FILE));
	m_DecodeInfo.bCopyInSameFile = m_EncodeInfo.bCopyInSameFile;

	// Encode setting
	m_EncodeInfo.bRemoveLineFeed = CUtility::GetCheckboxStatus(::GetDlgItem(_hSelf, IDC_CHK_REMOVE_LINE_FEED));

	// Decode Setting
	m_DecodeInfo.bBreakLine = CUtility::GetCheckboxStatus(::GetDlgItem(_hSelf, IDC_CHK_MULTILINE));
	m_DecodeInfo.strLineBreakDelimiter = CUtility::GetEditCtrlText(::GetDlgItem(_hSelf, IDC_EDT_DELIMITER));

	// For both
	FillExclusionSet();

	return WriteINI();
}

void SettingsDlg::destroy()
{
}

bool SettingsDlg::ReadINI()
{
	return ProfileEncode().GetInfo(m_EncodeInfo) && ProfileDecode().GetInfo(m_DecodeInfo);
}

bool SettingsDlg::WriteINI()
{
	return ProfileEncode().SetInfo(m_EncodeInfo) && ProfileDecode().SetInfo(m_DecodeInfo);
}

void SettingsDlg::InitDlg()
{
	ReadINI();

	// Common setting goes here
	CUtility::SetCheckboxStatus(::GetDlgItem(_hSelf, IDC_CHK_SAME_FILE), m_DecodeInfo.bCopyInSameFile);

	// Encode setting
	CUtility::SetCheckboxStatus(::GetDlgItem(_hSelf, IDC_CHK_REMOVE_LINE_FEED), m_EncodeInfo.bRemoveLineFeed);

	// Decode Setting
	CUtility::SetCheckboxStatus(::GetDlgItem(_hSelf, IDC_CHK_MULTILINE), m_DecodeInfo.bBreakLine);
	EnableDelimter(m_DecodeInfo.bBreakLine);

	// For both
	DisplayExclusionSet();
}

void SettingsDlg::EnableDelimter(bool enable)
{
	EnableWindow(::GetDlgItem(_hSelf, IDC_EDT_DELIMITER), enable);
	EnableWindow(::GetDlgItem(_hSelf, IDC_STC_DELIMETER), enable);

	CUtility::SetEditCtrlText(::GetDlgItem(_hSelf, IDC_EDT_DELIMITER), enable ? m_DecodeInfo.strLineBreakDelimiter : L"");
}

void SettingsDlg::FillExclusionSet()
{
	auto fill = [&](UINT ctrlID, std::vector<std::wstring>& data) {
		std::wstring txt = CUtility::GetEditCtrlText(::GetDlgItem(_hSelf, ctrlID));
		data = StringHelper::Split(txt, L"\r\n");		// because of multiline edit control
	};

	//fill(IDC_EDT_EXCLUDE_ENCODE, m_EncodeInfo.exclusionSet);
	//fill(IDC_EDT_EXCLUDE_DECODE, m_DecodeInfo.exclusionSet);
}

void SettingsDlg::DisplayExclusionSet()
{
	auto fill = [&](UINT ctrlID, const std::vector<std::wstring>& data) {
		std::wstring txt;
		for (const auto& item : data)
			txt += item + L"\r\n";		// because of multiline edit control

		CUtility::SetEditCtrlText(::GetDlgItem(_hSelf, ctrlID), txt);
	};

	//fill(IDC_EDT_EXCLUDE_ENCODE, m_EncodeInfo.exclusionSet);
	//fill(IDC_EDT_EXCLUDE_DECODE, m_DecodeInfo.exclusionSet);
}

