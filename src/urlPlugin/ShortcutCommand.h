#pragma once
#include "Define.h"
#include <memory>

enum class CallBackID :int { SHOW_SETTING = 0, ENCODE_URL, DECODE_URL, SEP_1, ABOUT };

class ShortcutCommand
{
public:
	ShortcutCommand(int nCommandCount);
	~ShortcutCommand() = default;

	auto GetCommandID(CallBackID id) const->int { return m_pFuncItem[At(id)]._cmdID; }
	auto GetFuncItem() const->FuncItem* { return m_pFuncItem.get(); }

	bool SetCommand(CallBackID id, const TCHAR* cmdName, const PFUNCPLUGINCMD pFunc, bool checkOnInit);
	bool SetShortCut(CallBackID id, const ShortcutKey& scKey);

private:
	int At(CallBackID id) const { return static_cast<int>(id); }

private:
	std::unique_ptr<FuncItem[]>		m_pFuncItem = nullptr;
	std::unique_ptr<ShortcutKey[]>	m_pShortcutKeys = nullptr;
	int								m_nCmdCount = 0;
};

