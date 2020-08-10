#include "ScintillaEditor.h"
#include "menuCmdID.h"
#include <cassert>
#include <memory>

ScintillaEditor::ScintillaEditor(const NppData& nppData) : m_NppData(nppData)
{
	int which = -1;
	::SendMessage(m_NppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&which));
	assert(which != -1);
	if (which != -1)
		m_hScintilla = (which == 0) ? m_NppData._scintillaMainHandle : m_NppData._scintillaSecondHandle;
}

auto ScintillaEditor::GetSelectedText() const -> std::string
{
	if (!m_hScintilla)
		return std::string();

	// Get the required length
	auto length = Execute(SCI_GETLENGTH);

	auto pData = std::make_unique<char[]>(length + 1);
	GetSelectedText(pData.get(), static_cast<int>(length + 1), false);

	return pData ? pData.get() : "";
}

bool ScintillaEditor::ReplaceSelectedText(const std::string& replaceText) const
{
	return Execute(SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(replaceText.c_str())) == 0;
}

bool ScintillaEditor::PasteSelectedOnNewFile(const std::string& text) const
{
	// Open a new document
	bool bRetVal = ::SendMessage(m_NppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW) ? true : false;

	// Paste the text
	auto res = Execute(SCI_SETTEXT, 0, reinterpret_cast<LPARAM>(text.c_str()));
	bRetVal &= res == 1;

	// Set file's language Javascript
	bRetVal &= ::SendMessage(m_NppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_LANG_JS) ? true : false;

	return bRetVal;
}

std::wstring ScintillaEditor::GetFullFilePath() const
{
	std::wstring retVal;

	auto pFilePath = std::make_unique<TCHAR[]>(MAX_PATH * 2);
	::SendMessage(m_NppData._nppHandle, NPPM_GETFULLCURRENTPATH, 0, reinterpret_cast<LPARAM>(pFilePath.get()));

	if (pFilePath)
		retVal = pFilePath.get();

	return retVal;
}

bool ScintillaEditor::OpenFile(const std::wstring& filePath) const
{
	bool retVal;
	retVal = ::SendMessage(m_NppData._nppHandle, NPPM_DOOPEN, 0, reinterpret_cast<LPARAM>(filePath.data())) ? true : false;
	return retVal;
}

LRESULT ScintillaEditor::Execute(UINT Msg, WPARAM wParam, LPARAM lParam) const
{
	if (!m_hScintilla)
		return -1;

	try
	{
		return ::SendMessage(m_hScintilla, Msg, wParam, lParam);
	}
	catch (...)
	{
		return -1;
	}
}

Sci_CharacterRange ScintillaEditor::GetSelection() const
{
	Sci_CharacterRange crange;
	crange.cpMin = static_cast<long>(Execute(SCI_GETSELECTIONSTART));
	crange.cpMax = static_cast<long>(Execute(SCI_GETSELECTIONEND));
	return crange;
}

char* ScintillaEditor::GetSelectedText(char* txt, int size, bool expand) const
{
	if (!size)
		return NULL;
	Sci_CharacterRange range = GetSelection();
	if (range.cpMax == range.cpMin && expand)
	{
		ExpandWordSelection();
		range = GetSelection();
	}
	if (!(size > (range.cpMax - range.cpMin)))	//there must be atleast 1 byte left for zero terminator
	{
		range.cpMax = range.cpMin + size - 1;	//keep room for zero terminator
	}

	return GetWordFromRange(txt, size, range.cpMin, range.cpMax);
}

bool ScintillaEditor::ExpandWordSelection() const
{
	std::pair<int, int> wordRange = GetWordRange();
	if (wordRange.first != wordRange.second)
	{
		Execute(SCI_SETSELECTIONSTART, wordRange.first);
		Execute(SCI_SETSELECTIONEND, wordRange.second);
		return true;
	}
	return false;
}

auto ScintillaEditor::GetWordRange() const -> std::pair<int, int>
{
	auto caretPos = Execute(SCI_GETCURRENTPOS, 0, 0);
	int startPos = static_cast<int>(Execute(SCI_WORDSTARTPOSITION, caretPos, true));
	int endPos = static_cast<int>(Execute(SCI_WORDENDPOSITION, caretPos, true));
	return std::pair<int, int>(startPos, endPos);
}

auto ScintillaEditor::GetWordFromRange(char* txt, int size, int pos1, int pos2) const-> char*
{
	if (!size)
		return NULL;
	if (pos1 > pos2)
	{
		int tmp = pos1;
		pos1 = pos2;
		pos2 = tmp;
	}

	if (size < pos2 - pos1)
		return NULL;

	GetText(txt, pos1, pos2);
	return txt;
}

void ScintillaEditor::GetText(char* dest, size_t start, size_t end) const
{
	Sci_TextRange tr;
	tr.chrg.cpMin = static_cast<long>(start);
	tr.chrg.cpMax = static_cast<long>(end);
	tr.lpstrText = dest;
	Execute(SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));
}

