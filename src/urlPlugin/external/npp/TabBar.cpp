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

#include "TabBar.h"
#include <stdexcept>
#include <Notepad_plus_msgs.h>
#include "menuCmdID.h"

const COLORREF grey      	            = RGB(128,   128,  128);

#define	IDC_DRAG_TAB     1404
#define	IDC_DRAG_INTERDIT_TAB 1405
#define	IDC_DRAG_PLUS_TAB 1406
#define	IDC_DRAG_OUT_TAB 1407

bool TabBarPlus::_doDragNDrop = false;

bool TabBarPlus::_drawTopBar = true;
bool TabBarPlus::_drawInactiveTab = true;
bool TabBarPlus::_drawTabCloseButton = false;
bool TabBarPlus::_isDbClk2Close = false;
bool TabBarPlus::_isCtrlVertical = false;
bool TabBarPlus::_isCtrlMultiLine = false;

COLORREF TabBarPlus::_activeTextColour = ::GetSysColor(COLOR_BTNTEXT);
COLORREF TabBarPlus::_activeTopBarFocusedColour = RGB(250, 170, 60);
COLORREF TabBarPlus::_activeTopBarUnfocusedColour = RGB(250, 210, 150);
COLORREF TabBarPlus::_inactiveTextColour = grey;
COLORREF TabBarPlus::_inactiveBgColour = RGB(192, 192, 192);

HWND TabBarPlus::_hwndArray[nbCtrlMax] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
int TabBarPlus::_nbCtrl = 0;

void TabBar::initTabBar(HINSTANCE hInst, HWND parent, bool isVertical, bool isTraditional, bool isMultiLine)
{
  Window::init(hInst, parent);
  int vertical = isVertical?(TCS_VERTICAL | TCS_MULTILINE | TCS_RIGHTJUSTIFY):0;
  _isTraditional = isTraditional;
  _isVertical = isVertical;
  _isMultiLine = isMultiLine;

  INITCOMMONCONTROLSEX icce;
  icce.dwSize = sizeof(icce);
  icce.dwICC = ICC_TAB_CLASSES;
  InitCommonControlsEx(&icce);
  int multiLine = isMultiLine?(_isTraditional?TCS_MULTILINE:0):0;

  int style = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE |\
    TCS_FOCUSNEVER | TCS_TABS | WS_TABSTOP | vertical | multiLine;

  _hSelf = ::CreateWindowEx(
    0,
    WC_TABCONTROL,
    TEXT("Tab"),
    style,
    0, 0, 0, 0,
    _hParent,
    NULL,
    _hInst,
    0);

  if (!_hSelf)
  {
    throw std::runtime_error("TabBar::initTabBar : CreateWindowEx() function return null");
  }
}

void TabBar::destroy()
{
  if (_hFont)
    DeleteObject(_hFont);

  if (_hLargeFont)
    DeleteObject(_hLargeFont);

  if (_hVerticalFont)
    DeleteObject(_hVerticalFont);

  if (_hVerticalLargeFont)
    DeleteObject(_hVerticalLargeFont);

  ::DestroyWindow(_hSelf);
  _hSelf = NULL;
}

int TabBar::insertAtEnd(const wchar_t *subTabName)
{
  TCITEM tie;
  tie.mask = TCIF_TEXT | TCIF_IMAGE;
  int index = -1;

  if (_hasImgLst)
    index = 0;
  tie.iImage = index;
  tie.pszText = (wchar_t *)subTabName;
  return int(::SendMessage(_hSelf, TCM_INSERTITEM, _nbItem++, reinterpret_cast<LPARAM>(&tie)));
}

void TabBar::getCurrentTitle(wchar_t *title, int titleLen)
{
  TCITEM tci;
  tci.mask = TCIF_TEXT;
  tci.pszText = title;
  tci.cchTextMax = titleLen-1;
  ::SendMessage(_hSelf, TCM_GETITEM, getCurrentTabIndex(), reinterpret_cast<LPARAM>(&tci));
}

void TabBar::setFont (HFONT font)
{
    ::SendMessage(_hSelf, WM_SETFONT, reinterpret_cast<WPARAM>(font), 0);
}

void TabBar::setFont(const wchar_t* fontName, int fontSize)
{
  if (_hFont)
    ::DeleteObject(_hFont);

  _hFont = ::CreateFont( fontSize, 0,
    (_isVertical) ? 900:0,
    (_isVertical) ? 900:0,
    FW_NORMAL,
    0, 0, 0, 0,
    0, 0, 0, 0,
    fontName);
  if (_hFont)
    ::SendMessage(_hSelf, WM_SETFONT, reinterpret_cast<WPARAM>(_hFont), 0);
}

void TabBar::activateAt(int index) const
{
  if (getCurrentTabIndex() != index)
  {
    ::SendMessage(_hSelf, TCM_SETCURSEL, index, 0);
  }
  TBHDR nmhdr;
  nmhdr.hdr.hwndFrom = _hSelf;
  nmhdr.hdr.code = TCN_SELCHANGE;
  nmhdr.hdr.idFrom = reinterpret_cast<UINT_PTR>(this);
  nmhdr.tabOrigin = index;
}

void TabBar::deletItemAt(size_t index)
{
  if (index == _nbItem-1)
  {
    //prevent invisible tabs. If last visible tab is removed, other tabs are put in view but not redrawn
    //Therefore, scroll one tab to the left if only one tab visible
    if (_nbItem > 1)
    {
      RECT itemRect;
      ::SendMessage(_hSelf, TCM_GETITEMRECT, (WPARAM)index, (LPARAM)&itemRect);
      if (itemRect.left < 5) //if last visible tab, scroll left once (no more than 5px away should be safe, usually 2px depending on the drawing)
      {
        //To scroll the tab control to the left, use the WM_HSCROLL notification
        //Doesn't really seem to be documented anywhere, but the values do match the message parameters
        //The up/down control really is just some sort of scrollbar
        //There seems to be no negative effect on any internal state of the tab control or the up/down control
        int wParam = MAKEWPARAM(SB_THUMBPOSITION, index - 1);
        ::SendMessage(_hSelf, WM_HSCROLL, wParam, 0);

        wParam = MAKEWPARAM(SB_ENDSCROLL, index - 1);
        ::SendMessage(_hSelf, WM_HSCROLL, wParam, 0);
      }
    }
  }
  ::SendMessage(_hSelf, TCM_DELETEITEM, index, 0);
  _nbItem--;
}

void TabBar::reSizeTo(RECT & rc2Ajust)
{
  RECT RowRect;
  int RowCount, TabsLength;

  // Important to do that!
  // Otherwise, the window(s) it contains will take all the resouce of CPU
  // We don't need to resize the contained windows if they are even invisible anyway
  display(rc2Ajust.right > 10);
  RECT rc = rc2Ajust;
  Window::reSizeTo(rc);

  // Do our own calculations because TabCtrl_AdjustRect doesn't work
  // on vertical or multi-lined tab controls

  RowCount = TabCtrl_GetRowCount(_hSelf);
  TabCtrl_GetItemRect(_hSelf, 0, &RowRect);
  if (_isTraditional)
  {
    TabCtrl_AdjustRect(_hSelf, false, &rc2Ajust);
  }
  else if (_isVertical)
  {
    TabsLength  = RowCount * (RowRect.right - RowRect.left);
    TabsLength += GetSystemMetrics(SM_CXEDGE);

    rc2Ajust.left	+= TabsLength;
    rc2Ajust.right	-= TabsLength;
  }
  else
  {
    TabsLength  = RowCount * (RowRect.bottom - RowRect.top);
    TabsLength += GetSystemMetrics(SM_CYEDGE);

    rc2Ajust.top	+= TabsLength;
    rc2Ajust.bottom -= TabsLength;
  }
}

void TabBarPlus::initTabBar(HINSTANCE hInst, HWND parent, bool isVertical, bool isTraditional, bool isMultiLine)
{
  Window::init(hInst, parent);
  int vertical = isVertical?(TCS_VERTICAL | TCS_MULTILINE | TCS_RIGHTJUSTIFY):0;
  _isTraditional = isTraditional;
  _isVertical = isVertical;
  _isMultiLine = isMultiLine;

  INITCOMMONCONTROLSEX icce;
  icce.dwSize = sizeof(icce);
  icce.dwICC = ICC_TAB_CLASSES;
  InitCommonControlsEx(&icce);
  int multiLine = isMultiLine?(_isTraditional?TCS_MULTILINE:0):0;

  int style = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE |\
    TCS_TOOLTIPS | TCS_FOCUSNEVER | TCS_TABS | vertical | multiLine;

  style |= TCS_OWNERDRAWFIXED;

  _hSelf = ::CreateWindowEx(
    0,
    WC_TABCONTROL,
    TEXT("Tab"),
    style,
    0, 0, 0, 0,
    _hParent,
    NULL,
    _hInst,
    0);

  if (!_hSelf)
  {
    throw std::runtime_error("TabBarPlus::initTabBar : CreateWindowEx() function return null");
  }
  if (!_isTraditional)
  {
    if (!_hwndArray[_nbCtrl])
    {
      _hwndArray[_nbCtrl] = _hSelf;
      _ctrlID = _nbCtrl;
    }
    else
    {
      int i = 0;
      bool found = false;
      for ( ; i < nbCtrlMax && !found ; i++)
        if (!_hwndArray[i])
          found = true;
      if (!found)
      {
        _ctrlID = -1;
        destroy();
        throw std::runtime_error("TabBarPlus::initTabBar : Tab Control error - Tab Control # is over its limit");
      }
      _hwndArray[i] = _hSelf;
      _ctrlID = i;
    }
    _nbCtrl++;

    ::SetWindowLongPtr(_hSelf, GWLP_USERDATA, (LONG_PTR)this);
    _tabBarDefaultProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(_hSelf, GWLP_USERDATA, (LONG_PTR)TabBarPlus_Proc));
  }

  LOGFONT LogFont;

  _hFont = (HFONT)::SendMessage(_hSelf, WM_GETFONT, 0, 0);

  if (_hFont == NULL)
    _hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);

  if (_hLargeFont == NULL)
    _hLargeFont = (HFONT)::GetStockObject(SYSTEM_FONT);

  if (::GetObject(_hFont, sizeof(LOGFONT), &LogFont) != 0)
  {
    LogFont.lfEscapement  = 900;
    LogFont.lfOrientation = 900;
    _hVerticalFont = CreateFontIndirect(&LogFont);

    LogFont.lfWeight = 900;
    _hVerticalLargeFont = CreateFontIndirect(&LogFont);
  }
}

void TabBarPlus::doOwnerDrawTab()
{
  ::SendMessage(_hwndArray[0], TCM_SETPADDING, 0, MAKELPARAM(6, 0));
  for (int i = 0 ; i < _nbCtrl ; i++)
  {
    if (_hwndArray[i])
    {
      auto style = ::GetWindowLongPtr(_hwndArray[i], GWL_STYLE);
      if (isOwnerDrawTab())
        style |= TCS_OWNERDRAWFIXED;
      else
        style &= ~TCS_OWNERDRAWFIXED;

      ::SetWindowLongPtr(_hwndArray[i], GWL_STYLE, style);
      ::InvalidateRect(_hwndArray[i], NULL, true);

      const int base = 6;
      ::SendMessage(_hwndArray[i], TCM_SETPADDING, 0, MAKELPARAM(_drawTabCloseButton?base+3:base, 0));
    }
  }
}

void TabBarPlus::setColour(COLORREF colour2Set, tabColourIndex i)
{
  using enum tabColourIndex;
  switch (i)
  {
  case activeText:
    _activeTextColour = colour2Set;
    break;
  case activeFocusedTop:
    _activeTopBarFocusedColour = colour2Set;
    break;
  case activeUnfocusedTop:
    _activeTopBarUnfocusedColour = colour2Set;
    break;
  case inactiveText:
    _inactiveTextColour = colour2Set;
    break;
  case inactiveBg :
    _inactiveBgColour = colour2Set;
    break;
  default :
    return;
  }
  doOwnerDrawTab();
}

void TabBarPlus::doVertical()
{
  for (int i = 0 ; i < _nbCtrl ; i++)
  {
    if (_hwndArray[i])
      SendMessage(_hwndArray[i], WM_TABSETSTYLE, isVertical(), TCS_VERTICAL);
  }
}

void TabBarPlus::doMultiLine()
{
  for (int i = 0 ; i < _nbCtrl ; i++)
  {
    if (_hwndArray[i])
      SendMessage(_hwndArray[i], WM_TABSETSTYLE, isMultiLine(), TCS_MULTILINE);
  }
}

LRESULT TabBarPlus::runProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
  switch (Message)
  {
    // Custom window message to change tab control style on the fly
  case WM_TABSETSTYLE:
    {
      //::SendMessage(upDownWnd, UDM_SETBUDDY, NULL, 0);
      auto style = ::GetWindowLongPtr(hwnd, GWL_STYLE);

      if (wParam > 0)
        style |= lParam;
      else
        style &= ~lParam;

      _isVertical  = ((style & TCS_VERTICAL) != 0);
      _isMultiLine = ((style & TCS_MULTILINE) != 0);

      ::SetWindowLongPtr(hwnd, GWL_STYLE, style);
      ::InvalidateRect(hwnd, NULL, true);

      return true;
    }

  case WM_LBUTTONDOWN :
    {
      if (_drawTabCloseButton)
      {
        int xPos = LOWORD(lParam);
        int yPos = HIWORD(lParam);

        if (_closeButtonZone.isHit(xPos, yPos, _currentHoverTabRect))
        {
          _whichCloseClickDown = getTabIndexAt(xPos, yPos);
          ::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_REFRESHTABAR, 0);
          return true;
        }
      }

      ::CallWindowProc(_tabBarDefaultProc, hwnd, Message, wParam, lParam);
      int currentTabOn = static_cast<int> (::SendMessage(_hSelf, TCM_GETCURSEL, 0, 0));

      if (wParam == 2)
        return true;

      if (_doDragNDrop)
      {
        _nSrcTab = _nTabDragged = currentTabOn;

        POINT point;
        point.x = LOWORD(lParam);
        point.y = HIWORD(lParam);
        ::ClientToScreen(hwnd, &point);
        if(::DragDetect(hwnd, point))
        {
          // Yes, we're beginning to drag, so capture the mouse...
          _isDragging = true;
          ::SetCapture(hwnd);
        }
      }

      TBHDR nmhdr;
      nmhdr.hdr.hwndFrom = _hSelf;
      nmhdr.hdr.code = NM_CLICK;
      nmhdr.hdr.idFrom = reinterpret_cast<UINT_PTR>(this);
      nmhdr.tabOrigin = currentTabOn;

      ::SendMessage(_hParent, WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmhdr));

      return true;
    }
  case WM_RBUTTONDOWN :	//rightclick selects tab aswell
    {
      ::CallWindowProc(_tabBarDefaultProc, hwnd, WM_LBUTTONDOWN, wParam, lParam);
      return true;
    }
    //#define NPPM_INTERNAL_ISDRAGGING 40926
  case WM_MOUSEMOVE :
    {
      if (_isDragging)
      {
        POINT p;
        p.x = LOWORD(lParam);
        p.y = HIWORD(lParam);
        exchangeItemData(p);

        // Get cursor position of "Screen"
        // For using the function "WindowFromPoint" afterward!!!
        ::GetCursorPos(&_draggingPoint);
        /*
        HWND h = ::WindowFromPoint(_draggingPoint);
        ::SetFocus(h);
        */

        draggingCursor(_draggingPoint);
        //::SendMessage(h, NPPM_INTERNAL_ISDRAGGING, 0, 0);
        return true;
      }

      if (_drawTabCloseButton)
      {
        int xPos = LOWORD(lParam);
        int yPos = HIWORD(lParam);

        int index = getTabIndexAt(xPos, yPos);

        if (index != -1)
        {
          // Reduce flicker by only redrawing needed tabs

          bool oldVal = _isCloseHover;
          int oldIndex = _currentHoverTabItem;
          RECT oldRect;

          ::SendMessage(_hSelf, TCM_GETITEMRECT, index, (LPARAM)&_currentHoverTabRect);
          ::SendMessage(_hSelf, TCM_GETITEMRECT, oldIndex, (LPARAM)&oldRect);
          _currentHoverTabItem = index;
          _isCloseHover = _closeButtonZone.isHit(xPos, yPos, _currentHoverTabRect);

          if (oldVal != _isCloseHover)
          {
            InvalidateRect(hwnd, &oldRect, false);
            InvalidateRect(hwnd, &_currentHoverTabRect, false);
          }
        }
      }
      break;
    }

  case WM_LBUTTONUP :
    {
      int xPos = LOWORD(lParam);
      int yPos = HIWORD(lParam);
      int currentTabOn = getTabIndexAt(xPos, yPos);
      if (_isDragging)
      {
        if(::GetCapture() == _hSelf)
          ::ReleaseCapture();

        // Send a notification message to the parent with wParam = 0, lParam = 0
        // nmhdr.idFrom = this
        // destIndex = this->_nSrcTab
        // scrIndex  = this->_nTabDragged
        TBHDR nmhdr;
        nmhdr.hdr.hwndFrom = _hSelf;
        nmhdr.hdr.code = _isDraggingInside?TCN_TABDROPPED:TCN_TABDROPPEDOUTSIDE;
        nmhdr.hdr.idFrom = reinterpret_cast<UINT_PTR>(this);
        nmhdr.tabOrigin = currentTabOn;

        ::SendMessage(_hParent, WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmhdr));
        return true;
      }

      if (_drawTabCloseButton)
      {
        if ((_whichCloseClickDown == currentTabOn) && _closeButtonZone.isHit(xPos, yPos, _currentHoverTabRect))
        {
          TBHDR nmhdr;
          nmhdr.hdr.hwndFrom = _hSelf;
          nmhdr.hdr.code = TCN_TABDELETE;
          nmhdr.hdr.idFrom = reinterpret_cast<UINT_PTR>(this);
          nmhdr.tabOrigin = currentTabOn;

          ::SendMessage(_hParent, WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmhdr));

          _whichCloseClickDown = -1;
          return true;
        }
        _whichCloseClickDown = -1;
      }

      break;
    }

  case WM_CAPTURECHANGED :
    {
      if (_isDragging)
      {
        _isDragging = false;
        return true;
      }
      break;
    }

  case WM_DRAWITEM :
    {
      drawItem((DRAWITEMSTRUCT *)lParam);
      return true;
    }

  case WM_KEYDOWN :
    {
      if (wParam == VK_LCONTROL)
        ::SetCursor(::LoadCursor(_hInst, MAKEINTRESOURCE(IDC_DRAG_PLUS_TAB)));
      return true;
    }

  case WM_MBUTTONUP:
    {
      int xPos = LOWORD(lParam);
      int yPos = HIWORD(lParam);
      int currentTabOn = getTabIndexAt(xPos, yPos);
      TBHDR nmhdr;
      nmhdr.hdr.hwndFrom = _hSelf;
      nmhdr.hdr.code = TCN_TABDELETE;
      nmhdr.hdr.idFrom = reinterpret_cast<UINT_PTR>(this);
      nmhdr.tabOrigin = currentTabOn;

      ::SendMessage(_hParent, WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmhdr));
      return true;
    }

  case WM_LBUTTONDBLCLK :
    {
      if (_isDbClk2Close)
      {
        int xPos = LOWORD(lParam);
        int yPos = HIWORD(lParam);
        int currentTabOn = getTabIndexAt(xPos, yPos);
        TBHDR nmhdr;
        nmhdr.hdr.hwndFrom = _hSelf;
        nmhdr.hdr.code = TCN_TABDELETE;
        nmhdr.hdr.idFrom = reinterpret_cast<UINT_PTR>(this);
        nmhdr.tabOrigin = currentTabOn;

        ::SendMessage(_hParent, WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmhdr));
      }
      return true;
    }
  }
  return ::CallWindowProc(_tabBarDefaultProc, hwnd, Message, wParam, lParam);
}

void TabBarPlus::drawItem(DRAWITEMSTRUCT *pDrawItemStruct)
{
  RECT rect = pDrawItemStruct->rcItem;

  int nTab = pDrawItemStruct->itemID;
  if (nTab < 0)
  {
    ::MessageBox(NULL, TEXT("nTab < 0"), TEXT(""), MB_OK);
    //return ::CallWindowProc(_tabBarDefaultProc, hwnd, Message, wParam, lParam);
  }
  bool isSelected = (nTab == ::SendMessage(_hSelf, TCM_GETCURSEL, 0, 0));

  wchar_t label[MAX_PATH] = {};
  TCITEM tci;
  tci.mask = TCIF_TEXT|TCIF_IMAGE;
  tci.pszText = label;
  tci.cchTextMax = MAX_PATH-1;

  if (!::SendMessage(_hSelf, TCM_GETITEM, nTab, reinterpret_cast<LPARAM>(&tci)))
  {
    ::MessageBox(NULL, TEXT("! TCM_GETITEM"), TEXT(""), MB_OK);
    //return ::CallWindowProc(_tabBarDefaultProc, hwnd, Message, wParam, lParam);
  }
  HDC hDC = pDrawItemStruct->hDC;

  int nSavedDC = ::SaveDC(hDC);

  ::SetBkMode(hDC, TRANSPARENT);
  HBRUSH hBrush = ::CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
  ::FillRect(hDC, &rect, hBrush);
  ::DeleteObject((HGDIOBJ)hBrush);

  if (isSelected)
  {
    if (_drawTopBar)
    {
      RECT barRect = rect;

      if (_isVertical)
      {
        barRect.right = barRect.left + 6;
        rect.left += 2;
      }
      else
      {
        barRect.bottom = barRect.top + 6;
        rect.top += 2;
      }

      hBrush = ::CreateSolidBrush(_activeTopBarFocusedColour);
      ::FillRect(hDC, &barRect, hBrush);
      ::DeleteObject((HGDIOBJ)hBrush);
    }
  }
  else
  {
    if (_drawInactiveTab)
    {
      RECT barRect = rect;

      hBrush = ::CreateSolidBrush(_inactiveBgColour);
      ::FillRect(hDC, &barRect, hBrush);
      ::DeleteObject((HGDIOBJ)hBrush);
    }
  }

  if (_drawTabCloseButton)
  {
    RECT closeButtonRect = _closeButtonZone.getButtonRectFrom(rect);
    if (isSelected)
    {
      if (!_isVertical)
      {
        //closeButtonRect.top  += 2;
        closeButtonRect.left -= 2;
      }
    }
    else
    {
      if (_isVertical)
        closeButtonRect.left += 2;
    }

    HDC hdcMemory;
    hdcMemory = ::CreateCompatibleDC(hDC);

    ::DeleteDC(hdcMemory);
  }

  // Draw image
  HIMAGELIST hImgLst = (HIMAGELIST)::SendMessage(_hSelf, TCM_GETIMAGELIST, 0, 0);

  SIZE charPixel;
  ::GetTextExtentPoint(hDC, TEXT(" "), 1, &charPixel);
  int spaceUnit = charPixel.cx;

  if (hImgLst && tci.iImage >= 0)
  {
    IMAGEINFO info;
    int yPos = 0, xPos = 0;
    int marge = 0;

    ImageList_GetImageInfo(hImgLst, tci.iImage, &info);

    RECT & imageRect = info.rcImage;

    if (_isVertical)
      xPos = (rect.left + (rect.right - rect.left)/2 + 2) - (imageRect.right - imageRect.left)/2;
    else
      yPos = (rect.top + (rect.bottom - rect.top)/2 + (isSelected?0:2)) - (imageRect.bottom - imageRect.top)/2;

    if (isSelected)
      marge = spaceUnit*2;
    else
      marge = spaceUnit;

    if (_isVertical)
    {
      rect.bottom -= imageRect.bottom - imageRect.top;
      ImageList_Draw(hImgLst, tci.iImage, hDC, xPos, rect.bottom - marge, isSelected?ILD_TRANSPARENT:ILD_SELECTED);
      rect.bottom += marge;
    }
    else
    {
      rect.left += marge;
      ImageList_Draw(hImgLst, tci.iImage, hDC, rect.left, yPos, isSelected?ILD_TRANSPARENT:ILD_SELECTED);
      rect.left += imageRect.right - imageRect.left;
    }
  }

  if (_isVertical)
    SelectObject(hDC, _hVerticalFont);
  else
    SelectObject(hDC, _hFont);

  int Flags = DT_SINGLELINE;

  if (_drawTabCloseButton)
  {
    Flags |= DT_LEFT;
  }
  else
  {
    if (!_isVertical)
      Flags |= DT_CENTER;
  }

  // the following uses pixel values the fix alignments issues with DrawText
  // and font's that are rotated 90 degrees
  if (isSelected)
  {
    //COLORREF selectedColor = RGB(0, 0, 255);
    ::SetTextColor(hDC, _activeTextColour);

    if (_isVertical)
    {
      rect.bottom -= 2;
      rect.left   += ::GetSystemMetrics(SM_CXEDGE) + 4;
      rect.top    += (_drawTabCloseButton)?spaceUnit:0;

      Flags |= DT_BOTTOM;
    }
    else
    {
      rect.top -= ::GetSystemMetrics(SM_CYEDGE);
      rect.top += 3;
      rect.left += _drawTabCloseButton?spaceUnit:0;

      Flags |= DT_VCENTER;
    }
  }
  else
  {
    ::SetTextColor(hDC, _inactiveTextColour);
    if (_isVertical)
    {
      rect.top	+= 2;
      rect.bottom += 4;
      rect.left   += ::GetSystemMetrics(SM_CXEDGE) + 2;
    }
    else
    {
      rect.left   += (_drawTabCloseButton)?spaceUnit:0;
    }

    Flags |= DT_BOTTOM;
  }
  ::DrawText(hDC, label, lstrlen(label), &rect, Flags);
  ::RestoreDC(hDC, nSavedDC);
}

void TabBarPlus::draggingCursor(POINT screenPoint)
{
  HWND hWin = ::WindowFromPoint(screenPoint);
  if (_hSelf == hWin)
    ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
  else
  {
    wchar_t className[256];
    ::GetClassName(hWin, className, 256);
    if ((!lstrcmp(className, TEXT("Scintilla"))) || (!lstrcmp(className, WC_TABCONTROL)))
    {
      if (::GetKeyState(VK_LCONTROL) & 0x80000000)
        ::SetCursor(::LoadCursor(_hInst, MAKEINTRESOURCE(IDC_DRAG_PLUS_TAB)));
      else
        ::SetCursor(::LoadCursor(_hInst, MAKEINTRESOURCE(IDC_DRAG_TAB)));
    }
    else if (isPointInParentZone(screenPoint))
      ::SetCursor(::LoadCursor(_hInst, MAKEINTRESOURCE(IDC_DRAG_INTERDIT_TAB)));
    else // drag out of application
      ::SetCursor(::LoadCursor(_hInst, MAKEINTRESOURCE(IDC_DRAG_OUT_TAB)));
  }
}

void TabBarPlus::exchangeItemData(POINT point)
{
  // Find the destination tab...
  int nTab = getTabIndexAt(point);

  // The position is over a tab.
  //if (hitinfo.flags != TCHT_NOWHERE)
  if (nTab != -1)
  {
    _isDraggingInside = true;

    if (nTab != _nTabDragged)
    {
      //1. set to focus
      ::SendMessage(_hSelf, TCM_SETCURSEL, nTab, 0);

      //2. shift their data, and insert the source
      TCITEM itemData_nDraggedTab, itemData_shift;
      itemData_nDraggedTab.mask = itemData_shift.mask = TCIF_IMAGE | TCIF_TEXT | TCIF_PARAM;
      const int stringSize = 256;
      wchar_t str1[stringSize];
      wchar_t str2[stringSize];

      itemData_nDraggedTab.pszText = str1;
      itemData_nDraggedTab.cchTextMax = (stringSize);

      itemData_shift.pszText = str2;
      itemData_shift.cchTextMax = (stringSize);

      ::SendMessage(_hSelf, TCM_GETITEM, _nTabDragged, reinterpret_cast<LPARAM>(&itemData_nDraggedTab));

      if (_nTabDragged > nTab)
      {
        for (int i = _nTabDragged ; i > nTab ; i--)
        {
          ::SendMessage(_hSelf, TCM_GETITEM, i-1, reinterpret_cast<LPARAM>(&itemData_shift));
          ::SendMessage(_hSelf, TCM_SETITEM, i, reinterpret_cast<LPARAM>(&itemData_shift));
        }
      }
      else
      {
        for (int i = _nTabDragged ; i < nTab ; i++)
        {
          ::SendMessage(_hSelf, TCM_GETITEM, i+1, reinterpret_cast<LPARAM>(&itemData_shift));
          ::SendMessage(_hSelf, TCM_SETITEM, i, reinterpret_cast<LPARAM>(&itemData_shift));
        }
      }
      ::SendMessage(_hSelf, TCM_SETITEM, nTab, reinterpret_cast<LPARAM>(&itemData_nDraggedTab));

      //3. update the current index
      _nTabDragged = nTab;
    }
  }
  else
  {
    //::SetCursor(::LoadCursor(_hInst, MAKEINTRESOURCE(IDC_DRAG_TAB)));
    _isDraggingInside = false;
  }
}
bool CloseButtonZone::isHit(int x, int y, const RECT & testZone) const
{
  if (((x + _width + _fromRight) < testZone.right) || (x > (testZone.right - _fromRight)))
    return false;

  if (((y - _hight - _fromTop) > testZone.top) || (y < (testZone.top + _fromTop)))
    return false;

  return true;
}

RECT CloseButtonZone::getButtonRectFrom(const RECT & tabItemRect) const
{
  RECT rect;
  rect.right = tabItemRect.right - _fromRight;
  rect.left = rect.right - _width;
  rect.top = tabItemRect.top + _fromTop;
  rect.bottom = rect.top + _hight;

  return rect;
}