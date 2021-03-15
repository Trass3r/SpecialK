/**
 * This file is part of Special K.
 *
 * Special K is free software : you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by The Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Special K is distributed in the hope that it will be useful,
 *
 * But WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Special K.
 *
 *   If not, see <http://www.gnu.org/licenses/>.
 *
**/

#ifndef __SK__WINDOW_H__
#define __SK__WINDOW_H__

#include <Windows.h>

#undef GetWindowLong
#undef GetWindowLongPtr
#undef SetWindowLong
#undef SetWindowLongPtr
#undef CallWindowProc
#undef DefWindowProc

void SK_HookWinAPI        (void);
void SK_InstallWindowHook (HWND hWnd);
void SK_InitWindow        (HWND hWnd, bool fullscreen_exclusive = false);
void SK_AdjustWindow      (void);
void SK_AdjustBorder      (void);

void SK_CenterWindowAtMouse (BOOL remember_pos);

void SK_SetWindowResX (LONG x);
void SK_SetWindowResY (LONG y);

int __stdcall SK_GetSystemMetrics (_In_ int nIndex);
LPRECT        SK_GetGameRect      (void);
bool          SK_DiscontEpsilon   (int x1, int x2, int tolerance);

HWND  __stdcall SK_RealizeForegroundWindow (HWND hWndForeground);
HWND  __stdcall SK_GetGameWindow           (void);

HRESULT
WINAPI
SK_DWM_EnableMMCSS (BOOL enable);

using CreateWindowExA_pfn    = HWND (WINAPI *)(
    _In_     DWORD     dwExStyle,
    _In_opt_ LPCSTR    lpClassName,
    _In_opt_ LPCSTR    lpWindowName,
    _In_     DWORD     dwStyle,
    _In_     int       X,
    _In_     int       Y,
    _In_     int       nWidth,
    _In_     int       nHeight,
    _In_opt_ HWND      hWndParent,
    _In_opt_ HMENU     hMenu,
    _In_opt_ HINSTANCE hInstance,
    _In_opt_ LPVOID    lpParam );

using CreateWindowExW_pfn    = HWND (WINAPI *)(
    _In_     DWORD     dwExStyle,
    _In_opt_ LPCWSTR   lpClassName,
    _In_opt_ LPCWSTR   lpWindowName,
    _In_     DWORD     dwStyle,
    _In_     int       X,
    _In_     int       Y,
    _In_     int       nWidth,
    _In_     int       nHeight,
    _In_opt_ HWND      hWndParent,
    _In_opt_ HMENU     hMenu,
    _In_opt_ HINSTANCE hInstance,
    _In_opt_ LPVOID    lpParam );

using MoveWindow_pfn         = BOOL (WINAPI *)(
  _In_ HWND hWnd,
  _In_ int  X,
  _In_ int  Y,
  _In_ int  nWidth,
  _In_ int  nHeight,
  _In_ BOOL bRedraw
);

using SetWindowPlacement_pfn = BOOL (WINAPI *)(
  _In_       HWND              hWnd,
  _In_ const WINDOWPLACEMENT *lpwndpl
);

using SetWindowPos_pfn       = BOOL (WINAPI *)(
  _In_     HWND hWnd,
  _In_opt_ HWND hWndInsertAfter,
  _In_     int  X,
  _In_     int  Y,
  _In_     int  cx,
  _In_     int  cy,
  _In_     UINT uFlags
);

using SetWindowLong_pfn      = LONG (WINAPI *)(
  _In_ HWND hWnd,
  _In_ int  nIndex,
  _In_ LONG dwNewLong
);

using SetWindowLongPtr_pfn   = LONG_PTR (WINAPI *)(
  _In_ HWND     hWnd,
  _In_ int      nIndex,
  _In_ LONG_PTR dwNewLong
);

using GetWindowLong_pfn      = LONG (WINAPI *)(
  _In_ HWND hWnd,
  _In_ int  nIndex
);

using GetWindowLongPtr_pfn   = LONG_PTR (WINAPI *)(
  _In_ HWND hWnd,
  _In_ int  nIndex
);


using SetClassLong_pfn      = LONG (WINAPI *)(
  _In_ HWND hWnd,
  _In_ int  nIndex,
  _In_ LONG dwNewLong
);

using SetClassLongPtr_pfn   = ULONG_PTR (WINAPI *)(
  _In_ HWND     hWnd,
  _In_ int      nIndex,
  _In_ LONG_PTR dwNewLong
);

using GetClassLong_pfn      = LONG (WINAPI *)(
  _In_ HWND hWnd,
  _In_ int  nIndex
);

using GetClassLongPtr_pfn   = ULONG_PTR (WINAPI *)(
  _In_ HWND hWnd,
  _In_ int  nIndex
);


using AdjustWindowRect_pfn   = BOOL (WINAPI *)(
  _Inout_ LPRECT lpRect,
  _In_    DWORD  dwStyle,
  _In_    BOOL   bMenu
);

using AdjustWindowRectEx_pfn = BOOL (WINAPI *)(
  _Inout_ LPRECT lpRect,
  _In_    DWORD  dwStyle,
  _In_    BOOL   bMenu,
  _In_    DWORD  dwExStyle
);

using GetSystemMetrics_pfn = int (WINAPI *)(
  _In_ int nIndex
);

using GetWindowRect_pfn    = BOOL (WINAPI *)(
  HWND,
  LPRECT
);
using GetClientRect_pfn    = BOOL (WINAPI *)(
  HWND,
  LPRECT
);

using DefWindowProc_pfn    = LRESULT (WINAPI *)(
  _In_ HWND   hWnd,
  _In_ UINT   Msg,
  _In_ WPARAM wParam,
  _In_ LPARAM lParam
);

using CallWindowProc_pfn   = LRESULT (WINAPI *)(
  _In_ WNDPROC lpPrevWndFunc,
  _In_ HWND    hWnd,
  _In_ UINT    Msg,
  _In_ WPARAM  wParam,
  _In_ LPARAM  lParam
);

using GetWindowLongPtr_pfn = LONG_PTR (WINAPI *)(
  _In_ HWND hWnd,
  _In_ int  nIndex
);


using ClipCursor_pfn       = BOOL (WINAPI *)(
  _In_opt_ const RECT *lpRect
);

using GetCursorPos_pfn     = BOOL (WINAPI *)(
  _Out_ LPPOINT lpPoint
);

using GetCursorInfo_pfn    = BOOL (WINAPI *)(
  _Inout_ PCURSORINFO pci
);

using NtUserGetCursorInfo_pfn = BOOL (WINAPI *)(
  _Inout_ PCURSORINFO pci
);

using GetAsyncKeyState_pfn = SHORT (WINAPI *)(
  _In_ int vKey
);

using GetKeyState_pfn      = SHORT (WINAPI *)(
  _In_ int nVirtKey
);

using RegisterRawInputDevices_pfn = BOOL (WINAPI *)(
  _In_ PCRAWINPUTDEVICE pRawInputDevices,
  _In_ UINT             uiNumDevices,
  _In_ UINT             cbSize
);

using GetRawInputData_pfn   = UINT (WINAPI *)(
  _In_      HRAWINPUT hRawInput,
  _In_      UINT      uiCommand,
  _Out_opt_ LPVOID    pData,
  _Inout_   PUINT     pcbSize,
  _In_      UINT      cbSizeHeader
);

using GetRawInputBuffer_pfn = UINT (WINAPI *)(
  _Out_opt_ PRAWINPUT pData,
  _Inout_   PUINT     pcbSize,
  _In_      UINT      cbSizeHeader
);
using GetKeyboardState_pfn  = BOOL (WINAPI *)(PBYTE lpKeyState);


using SetCursorPos_pfn      = BOOL (WINAPI *)(
  _In_ int X,
  _In_ int Y
);

using SendInput_pfn         = UINT (WINAPI *)(
  _In_ UINT    nInputs,
  _In_ LPINPUT pInputs,
  _In_ int     cbSize
);

using mouse_event_pfn       = void (WINAPI *)(
  _In_ DWORD     dwFlags,
  _In_ DWORD     dx,
  _In_ DWORD     dy,
  _In_ DWORD     dwData,
  _In_ ULONG_PTR dwExtraInfo
);

extern ClipCursor_pfn              ClipCursor_Original;
extern SetWindowPos_pfn            SetWindowPos_Original;
extern MoveWindow_pfn              MoveWindow_Original;
extern SetWindowLong_pfn           SetWindowLongW_Original;
extern SetWindowLong_pfn           SetWindowLongA_Original;
extern GetWindowLong_pfn           GetWindowLongW_Original;
extern GetWindowLong_pfn           GetWindowLongA_Original;
extern SetWindowLongPtr_pfn        SetWindowLongPtrW_Original;
extern SetWindowLongPtr_pfn        SetWindowLongPtrA_Original;
extern GetWindowLongPtr_pfn        GetWindowLongPtrW_Original;
extern GetWindowLongPtr_pfn        GetWindowLongPtrA_Original;
extern AdjustWindowRect_pfn        AdjustWindowRect_Original;
extern AdjustWindowRectEx_pfn      AdjustWindowRectEx_Original;

extern GetSystemMetrics_pfn        GetSystemMetrics_Original;
extern GetCursorPos_pfn            GetCursorPos_Original;
extern SetCursorPos_pfn            SetCursorPos_Original;
extern GetCursorInfo_pfn           GetCursorInfo_Original;
extern NtUserGetCursorInfo_pfn     NtUserGetCursorInfo_Original;

extern SendInput_pfn               SendInput_Original;
extern mouse_event_pfn             mouse_event_Original;

extern GetKeyState_pfn             GetKeyState_Original;
extern GetAsyncKeyState_pfn        GetAsyncKeyState_Original;
extern GetKeyboardState_pfn        GetKeyboardState_Original;
extern GetRawInputData_pfn         GetRawInputData_Original;
extern GetRawInputBuffer_pfn       GetRawInputBuffer_Original;
extern RegisterRawInputDevices_pfn RegisterRawInputDevices_Original;

#define SK_HWND_DESKTOP                            nullptr
#define SK_HWND_BOTTOM    reinterpret_cast <HWND> (   1   )
#define SK_HWND_DUMMY     reinterpret_cast <HWND> (  -1   )
#define SK_HWND_NOTOPMOST reinterpret_cast <HWND> (  -2   )

#define SK_HWND_TOP        SK_HWND_DESKTOP
#define SK_HWND_TOPMOST    SK_HWND_DUMMY

#include <SpecialK/input/input.h>

struct sk_window_s {
       sk_window_s (void) noexcept { };

  bool       unicode          = false;

  HWND       hWnd             = nullptr;
  WNDPROC    WndProc_Original = nullptr;
  WNDPROC    RawProc_Original = nullptr;

  bool       exclusive_full   = false; //D3D only

  bool       active           = true;

  struct {
    int width  = 0;
    int height = 0;
  } border;

  struct {
    struct {
      LONG   width            = 640;
      LONG   height           = 480;
    } framebuffer;

    //struct {
    RECT   client { 0, 0, 640, 480 };
    RECT   window { 0, 0, 640, 480 };
    //};

    ULONG_PTR style           = 0x0;//WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    ULONG_PTR style_ex        = 0x0;//WS_EX_APPWINDOW     | WS_EX_WINDOWEDGE;
  } game, actual;

  ULONG_PTR   border_style    = WS_CAPTION      | WS_SYSMENU | WS_POPUP |
                                WS_MINIMIZEBOX  | WS_VISIBLE |
                                WS_CLIPSIBLINGS |
                                WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW;
  ULONG_PTR   border_style_ex = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

//  RECT      rect        { 0, 0,
//                          0, 0 };
//  RECT      game_rect   { 0, 0,
//                          0, 0 };

  struct {
    // Will be false if remapping is necessary
    bool     identical        = true;

    struct {
      float  x                = 1.0f;
      float  y                = 1.0f;
    } scale;

    struct {
      float  x                = 0.0f;
      float  y                = 0.0f;
    } offset;
  } coord_remap;

  LONG      render_x         = 640;
  LONG      render_y         = 480;

  LONG      game_x           = 640; // Resolution game thinks it's running at
  LONG      game_y           = 480; // Resolution game thinks it's running at

  RECT      cursor_clip { LONG_MIN, LONG_MIN,
                          LONG_MAX, LONG_MAX };

  // Cursor position when window activation changed
  POINT     cursor_pos  { 0, 0 };

  // State to restore the cursor to
  //  (TODO: Should probably be a reference count to return to)
  bool      cursor_visible   = true;

  // Next call to AdjustBorder will add a border if one does not exist
  bool      attach_border    = false;

  void    getRenderDims (LONG& x, LONG& y) noexcept {
    x = (actual.client.right  - actual.client.left);
    y = (actual.client.bottom - actual.client.top);
  }

  bool    needsCoordTransform (void);
  void    updateDims          (void);

  SetWindowLongPtr_pfn SetWindowLongPtr = SetWindowLongPtrW;
  GetWindowLongPtr_pfn GetWindowLongPtr = GetWindowLongPtrW;
  SetClassLongPtr_pfn  SetClassLongPtr  = ::SetClassLongPtrW;
  GetClassLongPtr_pfn  GetClassLongPtr  = ::GetClassLongPtrW;
  DefWindowProc_pfn    DefWindowProc    = DefWindowProcW;
  CallWindowProc_pfn   CallWindowProc   = CallWindowProcW;

  LRESULT WINAPI DefProc (
    _In_ UINT   Msg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
  ) ;

  LRESULT WINAPI CallProc (
    _In_ HWND    hWnd_,
    _In_ UINT    Msg,
    _In_ WPARAM  wParam,
    _In_ LPARAM  lParam
  ) ;

  bool hooked = false;
};

extern sk_window_s game_window;

//sk_window_s*
//SK_Win32_InitializeWindow (void)
//{
//  sk_window_s* game_window = nullptr;
//
//  if (game_window != nullptr)
//  {
//    delete game_window;
//           game_window = nullptr;
//  }
//
//  if (game_window == nullptr)
//  {
//    game_window =
//      new sk_window_s ();
//  }
//
//  if (game_window != nullptr)
//  {
//    delete ::game_window;
//           ::game_window = nullptr;
//  }
//}

struct window_t {
  DWORD proc_id;
  HWND  root;
};

class SK_TLS;

bool
__stdcall
SK_IsGameWindowActive (void);

HWND
WINAPI
SK_GetForegroundWindow (void);

HWND
WINAPI
SK_GetFocus (void);

BOOL
SK_IsChild (HWND hWndParent, HWND hWnd);

BOOL
SK_Win32_IsGUIThread ( DWORD    dwTid = SK_Thread_GetCurrentId (),
                       SK_TLS **ppTLS = nullptr );

window_t
SK_FindRootWindow (DWORD proc_id);

bool
SK_Window_HasBorder (HWND hWnd = game_window.hWnd);

bool
SK_Window_IsFullscreen (HWND hWnd = game_window.hWnd);



BOOL
WINAPI
SK_ClipCursor (const RECT *lpRect);

BOOL
WINAPI
SK_GetCursorPos (LPPOINT lpPoint);

BOOL
WINAPI
SK_GetCursorInfo (PCURSORINFO pci);

BOOL
WINAPI
SK_SetCursorPos (int X, int Y);

UINT
WINAPI
SK_SendInput (UINT nInputs, LPINPUT pInputs, int cbSize);

BOOL
WINAPI
SK_SetWindowPos ( HWND hWnd,
                  HWND hWndInsertAfter,
                  int  X,
                  int  Y,
                  int  cx,
                  int  cy,
                  UINT uFlags );

BOOL
WINAPI
SK_GetWindowRect (HWND hWnd, LPRECT rect);

BOOL
WINAPI
SK_GetClientRect (HWND hWnd, LPRECT rect);


LONG
WINAPI
SK_GetWindowLongA  (
  _In_ HWND hWnd,
  _In_ int  nIndex );

LONG_PTR
WINAPI
SK_GetWindowLongPtrA   (
  _In_ HWND     hWnd,
  _In_ int      nIndex );

LONG
WINAPI
SK_SetWindowLongA     (
  _In_ HWND hWnd,
  _In_ int  nIndex,
  _In_ LONG dwNewLong );

LONG_PTR
WINAPI
SK_SetWindowLongPtrA      (
  _In_ HWND     hWnd,
  _In_ int      nIndex,
  _In_ LONG_PTR dwNewLong );

LONG
WINAPI
SK_GetWindowLongW  (
  _In_ HWND hWnd,
  _In_ int  nIndex );

LONG_PTR
WINAPI
SK_GetWindowLongPtrW   (
  _In_ HWND     hWnd,
  _In_ int      nIndex );

LONG
WINAPI
SK_SetWindowLongW     (
  _In_ HWND hWnd,
  _In_ int  nIndex,
  _In_ LONG dwNewLong );

LONG_PTR
WINAPI
SK_SetWindowLongPtrW      (
  _In_ HWND     hWnd,
  _In_ int      nIndex,
  _In_ LONG_PTR dwNewLong );

BOOL
WINAPI
SK_AdjustWindowRect (
  _Inout_ LPRECT lpRect,
  _In_    DWORD  dwStyle,
  _In_    BOOL   bMenu );

BOOL
WINAPI
SK_AdjustWindowRectEx (
  _Inout_ LPRECT lpRect,
  _In_    DWORD  dwStyle,
  _In_    BOOL   bMenu,
  _In_    DWORD  dwExStyle );

LRESULT
WINAPI
SK_DispatchMessageW (_In_ const MSG *lpMsg);

BOOL
WINAPI
SK_GetMessageW (LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);

HWND
WINAPI
SK_SetActiveWindow (HWND hWnd);

extern void SK_Window_RepositionIfNeeded (void);
void SKX_Window_EstablishRoot     (void);

#endif /* __SK__WINDOW_H__ */