#pragma once
#include "StaticDialog.h"
#include "ControlsTab.h"
#include "Define.h"
#include <memory>

class SettingsDlg : public StaticDialog
{
public:
	SettingsDlg(HINSTANCE hIntance, HWND hParent, int nCmdId);
	~SettingsDlg() = default;

	bool ShowDlg(bool bShow);

protected:
	virtual INT_PTR CALLBACK run_dlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
	bool Apply();
	void destroy() override;
	bool ReadINI();
	bool WriteINI();
	void InitDlg();

	void EnableDelimter(bool enable);

	void FillExclusionSet();
	void DisplayExclusionSet();

private:
	int				m_nCmdId = -1;

	EncodeInfo		m_EncodeInfo;
	DecodeInfo		m_DecodeInfo;
};
