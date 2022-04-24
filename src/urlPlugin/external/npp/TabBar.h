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

#ifndef TAB_BAR_H
#define TAB_BAR_H

#ifndef _WIN32_IE
#define _WIN32_IE	0x0600
#endif //_WIN32_IE

#include <windows.h>
#include <commctrl.h>
#include "Window.h"

//Notification message
#define TCN_TABDROPPED (TCN_FIRST - 10)
#define TCN_TABDROPPEDOUTSIDE (TCN_FIRST - 11)
#define TCN_TABDELETE (TCN_FIRST - 12)

#define WM_TABSETSTYLE	(WM_APP + 0x024)

const int margeValue = 2;
const int nbCtrlMax = 10;

const wchar_t TABBAR_ACTIVEFOCUSEDINDCATOR[64] = TEXT("Active tab focused indicator");
const wchar_t TABBAR_ACTIVEUNFOCUSEDINDCATOR[64] = TEXT("Active tab unfocused indicator");
const wchar_t TABBAR_ACTIVETEXT[64] = TEXT("Active tab text");
const wchar_t TABBAR_INACTIVETEXT[64] = TEXT("Inactive tabs");

struct TBHDR {
  NMHDR hdr;
  int tabOrigin;
};

class TabBar : public Window
{
public:
  TabBar() : Window() {};
  virtual ~TabBar() {};
  virtual void destroy();
  virtual void initTabBar(HINSTANCE hInst, HWND hwnd, bool isVertical = false, bool isTraditional = false, bool isMultiLine = false);
  virtual void reSizeTo(RECT & rc2Ajust);
  int insertAtEnd(const wchar_t *subTabName);
  void activateAt(int index) const;
  void getCurrentTitle(wchar_t *title, int titleLen);
    void setFont(HFONT font);

    auto getCurrentTabIndex() const {
    return ::SendMessage(_hSelf, TCM_GETCURSEL, 0, 0);
  };
  void deletItemAt(size_t index);

  void deletAllItem() {
    ::SendMessage(_hSelf, TCM_DELETEALLITEMS, 0, 0);
    _nbItem = 0;
  };

  void setImageList(HIMAGELIST himl) {
    _hasImgLst = true;
    ::SendMessage(_hSelf, TCM_SETIMAGELIST, 0, (LPARAM)himl);
  };

  auto nbItem() const {
    return _nbItem;
  };

  void setFont(const wchar_t* fontName, int fontSize);

  void setVertical(bool b) {
    _isVertical = b;
  };

  void setMultiLine(bool b) {
    _isMultiLine = b;
  };

protected:
  size_t _nbItem = 0;
  bool _hasImgLst = false;
  HFONT _hFont = nullptr;
  HFONT _hLargeFont = nullptr;
  HFONT _hVerticalFont = nullptr;
  HFONT _hVerticalLargeFont = nullptr;

  int _ctrlID = -1;
  bool _isTraditional = false;

  bool _isVertical = false;
  bool _isMultiLine = false;

  long getRowCount() const {
    return long(::SendMessage(_hSelf, TCM_GETROWCOUNT, 0, 0));
  };
};

struct CloseButtonZone {
  CloseButtonZone(): _width(11), _hight(11), _fromTop(5), _fromRight(3){};
  bool isHit(int x, int y, const RECT & testZone) const;
  RECT getButtonRectFrom(const RECT & tabItemRect) const;

  int _width;
  int _hight;
  int _fromTop; // distance from top in pixzl
  int _fromRight; // distance from right in pixzl
};

class TabBarPlus : public TabBar
{
public :

  TabBarPlus() : TabBar() {};

  enum class tabColourIndex {
    activeText, activeFocusedTop, activeUnfocusedTop, inactiveText, inactiveBg
  };

  static void doDragNDrop(bool justDoIt) {
    _doDragNDrop = justDoIt;
  };

  virtual void initTabBar(HINSTANCE hInst, HWND hwnd, bool isVertical = false, bool isTraditional = false, bool isMultiLine = false);

  static bool doDragNDropOrNot() {
    return _doDragNDrop;
  };

  int getSrcTabIndex() const {
    return _nSrcTab;
  };

  int getTabDraggedIndex() const {
    return _nTabDragged;
  };

  POINT getDraggingPoint() const {
    return _draggingPoint;
  };

  void resetDraggingPoint() {
    _draggingPoint.x = 0;
    _draggingPoint.y = 0;
  };

  static void doOwnerDrawTab();
  static void doVertical();
  static void doMultiLine();
  static bool isOwnerDrawTab() {return true;};
  static bool drawTopBar() {return _drawTopBar;};
  static bool drawInactiveTab() {return _drawInactiveTab;};
  static bool drawTabCloseButton() {return _drawTabCloseButton;};
  static bool isDbClk2Close() {return _isDbClk2Close;};
  static bool isVertical() { return _isCtrlVertical;};
  static bool isMultiLine() { return _isCtrlMultiLine;};

  static void setDrawTopBar(bool b) {
    _drawTopBar = b;
    doOwnerDrawTab();
  };
  static void setDrawInactiveTab(bool b) {
    _drawInactiveTab = b;
    doOwnerDrawTab();
  };
  static void setDrawTabCloseButton(bool b) {
    _drawTabCloseButton = b;
    doOwnerDrawTab();
  };

  static void setDbClk2Close(bool b) {
    _isDbClk2Close = b;
  };

  static void setVertical(bool b) {
    _isCtrlVertical = b;
    doVertical();
  };

  static void setMultiLine(bool b) {
    _isCtrlMultiLine = b;
    doMultiLine();
  };

  static void setColour(COLORREF colour2Set, tabColourIndex i);

protected:
  // it's the boss to decide if we do the drag N drop
  static bool _doDragNDrop;
  // drag N drop members
  bool _isDragging = false;
  bool _isDraggingInside = false;
  int _nSrcTab = 0;
  int _nTabDragged = -1;
  POINT _draggingPoint = {}; // coordinate of Screen
  WNDPROC _tabBarDefaultProc = nullptr;

  RECT _currentHoverTabRect = {};
  int _currentHoverTabItem = -1;

  CloseButtonZone _closeButtonZone;
  bool _isCloseHover = false;
  int _whichCloseClickDown = -1;
  bool _lmbdHit = false; // Left Mouse Button Down Hit

  LRESULT runProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

  static LRESULT CALLBACK TabBarPlus_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    return (((TabBarPlus *)(::GetWindowLongPtr(hwnd, GWLP_USERDATA)))->runProc(hwnd, Message, wParam, lParam));
  };
  void exchangeItemData(POINT point);

  // it's the boss to decide if we do the ownerDraw style tab
  static bool _drawInactiveTab;
  static bool _drawTopBar;
  static bool _drawTabCloseButton;
  static bool _isDbClk2Close;
  static bool _isCtrlVertical;
  static bool _isCtrlMultiLine;

  static COLORREF _activeTextColour;
  static COLORREF _activeTopBarFocusedColour;
  static COLORREF _activeTopBarUnfocusedColour;
  static COLORREF _inactiveTextColour;
  static COLORREF _inactiveBgColour;

  static int _nbCtrl;
  static HWND _hwndArray[nbCtrlMax];

  void drawItem(DRAWITEMSTRUCT *pDrawItemStruct);
  void draggingCursor(POINT screenPoint);

  int getTabIndexAt(const POINT& p) {
    return getTabIndexAt(p.x, p.y);
  };

  int getTabIndexAt(int x, int y) {
    TCHITTESTINFO hitInfo;
    hitInfo.pt.x = x;
    hitInfo.pt.y = y;
    return static_cast<int> (::SendMessage(_hSelf, TCM_HITTEST, 0, (LPARAM)&hitInfo));
  };

  bool isPointInParentZone(POINT screenPoint) const {
    RECT parentZone;
    ::GetWindowRect(_hParent, &parentZone);
    return (((screenPoint.x >= parentZone.left) && (screenPoint.x <= parentZone.right)) &&
      (screenPoint.y >= parentZone.top) && (screenPoint.y <= parentZone.bottom));
  };
};

#endif // TAB_BAR_H
