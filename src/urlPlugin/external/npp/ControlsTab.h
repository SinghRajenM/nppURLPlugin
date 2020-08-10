// This file is part of Notepad++ project
// Copyright (C)2003 Don HO <don.h@free.fr>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Note that the GPL places important restrictions on "derived works", yet
// it does not provide a detailed definition of that term.  To avoid
// misunderstandings, we consider an application to constitute a
// "derivative work" for the purpose of this license if it does any of the
// following:
// 1. Integrates source code from Notepad++.
// 2. Integrates/includes/aggregates Notepad++ into a proprietary executable
//    installer, such as those produced by InstallShield.
// 3. Links to a library or executes a program that does any of the above.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef CONTROLS_TAB_H
#define CONTROLS_TAB_H

#include <vector>
#include <string>

#ifndef TAB_BAR_H
#include "TabBar.h"
#endif //TAB_BAR_H

typedef std::basic_string<wchar_t> generic_string;
struct DlgInfo {
  Window *_dlg;
  generic_string _name;
  generic_string _internalName;

  DlgInfo(Window *dlg, const wchar_t *name, const wchar_t *internalName = NULL): _dlg(dlg), _name(name), _internalName(internalName?internalName:TEXT("")) {};
};

typedef std::vector<DlgInfo> WindowVector;

class ControlsTab : public TabBar
{
public :
  ControlsTab() : TabBar(), _pWinVector(NULL), _current(0){};
  ~ControlsTab(){};

  void initTabBar(HINSTANCE hInst, HWND hwnd, bool isVertical = false, bool isTraditional = false, bool isMultiLine = false) override {
    _isVertical = isVertical;
    TabBar::initTabBar(hInst, hwnd, false, isTraditional, isMultiLine);
  };
  void createTabs(WindowVector & winVector);

  void destroy() override {
    TabBar::destroy();
  };

  void reSizeTo(RECT & rc) override;
  void activateWindowAt(int index);

  void clickedUpdate()
  {
    int indexClicked = int(::SendMessage(_hSelf, TCM_GETCURSEL, 0, 0));
    activateWindowAt(indexClicked);
  };
  void renameTab(int index, const wchar_t *newName);
  bool renameTab(const wchar_t *internalName, const wchar_t *newName);

private :
  WindowVector *_pWinVector;
  int _current;
};

#endif //CONTROLS_TAB_H
