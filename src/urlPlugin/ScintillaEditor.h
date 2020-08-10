#pragma once
#include "Define.h"
#include <string>
#include <utility>

class ScintillaEditor
{
public:
	ScintillaEditor(const NppData& nppData);
	~ScintillaEditor() = default;

	auto GetSelectedText() const->std::string;
	bool ReplaceSelectedText(const std::string& replaceText) const;
	bool PasteSelectedOnNewFile(const std::string& text) const;
	
	auto GetFullFilePath() const->std::wstring;
	bool OpenFile(const std::wstring& filePath) const;

private:
	LRESULT Execute(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0) const;
	auto GetSelection() const->Sci_CharacterRange;
	auto GetSelectedText(char* txt, int size, bool expand = true) const->char*;
	bool ExpandWordSelection() const;
	auto GetWordRange() const->std::pair<int, int>;
	auto GetWordFromRange(char* txt, int size, int pos1, int pos2) const->char*;
	void GetText(char* dest, size_t start, size_t end) const;

private:
	NppData m_NppData = {};
	HWND	m_hScintilla = nullptr;
};

