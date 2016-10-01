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
#define _CRT_SECURE_NO_WARNINGS
#define ISOLATION_AWARE_ENABLED 1

#include "../ini.h"
#include "../parameter.h"
#include "../utility.h"

#include "../resource.h"

#include <Windows.h>
#include <windowsx.h>

#include <CommCtrl.h>
#pragma comment (lib,    "advapi32.lib")
#pragma comment (lib,    "user32.lib")
#pragma comment (lib,    "comctl32.lib")
#pragma comment (linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "  \
                         "version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df'" \
                         " language='*'\"")

#include <process.h>
#include <cstdint>

#include <Windows.h>
#include <Wininet.h>
#pragma comment (lib, "wininet.lib")

extern const wchar_t* __stdcall SK_GetRootPath (void);

enum {
  STATUS_UPDATED   = 1,
  STATUS_REMINDER  = 2,
  STATUS_CANCELLED = 4,
  STATUS_FAILED    = 8
};

struct sk_internet_get_t {
  wchar_t wszHostName  [INTERNET_MAX_HOST_NAME_LENGTH];
  wchar_t wszHostPath  [INTERNET_MAX_PATH_LENGTH];
  wchar_t wszLocalPath [MAX_PATH];
  HWND    hTaskDlg;
  int     status;
};

unsigned int
WINAPI
DownloadThread (LPVOID user)
{
  sk_internet_get_t* get =
    (sk_internet_get_t *)user;

  auto TaskMsg =
    [get](UINT Msg, WPARAM wParam, LPARAM lParam) ->
      LRESULT
      {
        if (get->hTaskDlg != 0)
          return SendMessage ( get->hTaskDlg,
                                 Msg,
                                   wParam,
                                     lParam );
        return NULL;
      };

  auto SetProgress = 
    [=](auto cur, auto max) ->
      void
      {
        TaskMsg ( TDM_SET_PROGRESS_BAR_POS,
                    MAXUINT16 * ((double)cur / (double)max),
                      0L );
      };

  auto SetErrorState =
    [=](void) ->
      void
      {
        TaskMsg (TDM_SET_PROGRESS_BAR_STATE, PBST_NORMAL, 0L);
        TaskMsg (TDM_SET_PROGRESS_BAR_RANGE, 0L,          MAKEWPARAM (0, 1));
        TaskMsg (TDM_SET_PROGRESS_BAR_POS,   1,           0L);
        TaskMsg (TDM_SET_PROGRESS_BAR_STATE, PBST_ERROR,  0L);
      };

  TaskMsg (TDM_SET_PROGRESS_BAR_RANGE, 0L,          MAKEWPARAM (0, MAXUINT16));
  TaskMsg (TDM_SET_PROGRESS_BAR_STATE, PBST_NORMAL, 0L);

  SetProgress (0, 0);


  HINTERNET hInetRoot =
    InternetOpen (
      L"Special K Auto-Update",
        INTERNET_OPEN_TYPE_DIRECT,
          nullptr, nullptr,
            0x00
    );

  if (! hInetRoot)
    goto CLEANUP;

  DWORD dwInetCtx;

  HINTERNET hInetHost =
    InternetConnect ( hInetRoot,
                        get->wszHostName,
                          INTERNET_DEFAULT_HTTP_PORT,
                            nullptr, nullptr,
                              INTERNET_SERVICE_HTTP,
                                0x00,
                                  (DWORD_PTR)&dwInetCtx );

  if (! hInetHost) {
    InternetCloseHandle (hInetRoot);
    goto CLEANUP;
  }

  PCWSTR rgpszAcceptTypes [] = { L"*/*", nullptr };

  HINTERNET hInetHTTPGetReq =
    HttpOpenRequest ( hInetHost,
                        nullptr,
                          get->wszHostPath,
                            L"HTTP/1.1",
                              nullptr,
                                rgpszAcceptTypes,
                                  INTERNET_FLAG_NO_UI          | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID |
                                  INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID,
                                    (DWORD_PTR)&dwInetCtx );

  if (! hInetHTTPGetReq) {
    InternetCloseHandle (hInetHost);
    InternetCloseHandle (hInetRoot);
    goto CLEANUP;
  }

  if ( HttpSendRequestW ( hInetHTTPGetReq,
                            nullptr,
                              0,
                                nullptr,
                                  0 ) ) {

    DWORD dwContentLength;
    DWORD dwContentLength_Len = sizeof DWORD;
    DWORD dwSize;

    DWORD dwIdx = 0;

    HttpQueryInfo ( hInetHTTPGetReq,
                      HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
                        &dwContentLength,
                          &dwContentLength_Len,
                            nullptr );

    SetProgress (0, dwContentLength);

    DWORD dwTotalBytesDownloaded = 0UL;

    if ( InternetQueryDataAvailable ( hInetHTTPGetReq,
                                        &dwSize,
                                          0x00, NULL )
      )
    {
      HANDLE hGetFile =
        CreateFileW ( get->wszLocalPath,
                        GENERIC_WRITE,
                          FILE_SHARE_READ,
                            nullptr,
                              CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL |
                                FILE_FLAG_SEQUENTIAL_SCAN,
                                  nullptr );

      while (hGetFile != INVALID_HANDLE_VALUE && dwSize > 0) {
        DWORD    dwRead = 0;
        uint8_t *pData  = (uint8_t *)malloc (dwSize);

        if (! pData)
          break;

        if ( InternetReadFile ( hInetHTTPGetReq,
                                  pData,
                                    dwSize,
                                      &dwRead )
          )
        {
          DWORD dwWritten;

          WriteFile ( hGetFile,
                        pData,
                          dwRead,
                            &dwWritten,
                              nullptr );

          dwTotalBytesDownloaded += dwRead;

          SetProgress (dwTotalBytesDownloaded, dwContentLength);
        }

        free (pData);
        pData = nullptr;

        if (! InternetQueryDataAvailable ( hInetHTTPGetReq,
                                             &dwSize,
                                               0x00, NULL
                                         )
           ) break;
      }

      if (hGetFile != INVALID_HANDLE_VALUE)
        CloseHandle (hGetFile);

      else
        SetErrorState ();
    }

    HttpEndRequest ( hInetHTTPGetReq, nullptr, 0x00, 0 );

    get->status = STATUS_UPDATED;
  }

  else {
    SetErrorState ();
  }

  InternetCloseHandle (hInetHTTPGetReq);
  InternetCloseHandle (hInetHost);
  InternetCloseHandle (hInetRoot);

  goto END;

CLEANUP:
  SetErrorState ();
  get->status = STATUS_FAILED;

END:
  //if (! get->hTaskDlg)
    //delete get;

  CloseHandle (GetCurrentThread ());

  return 0;
}

HWND hWndRemind = 0;
#define ID_REMIND 0

INT_PTR
CALLBACK
RemindMeLater_DlgProc (
  _In_ HWND   hWndDlg,
  _In_ UINT   uMsg,
  _In_ WPARAM wParam,
  _In_ LPARAM lParam )
{
  HWND hWndNextCheck =
    GetDlgItem (hWndDlg, IDC_NEXT_VERSION_CHECK);

  switch (uMsg) {
    case WM_INITDIALOG:
    {
      ComboBox_InsertString (hWndNextCheck, 0, L"Next launch");
      ComboBox_InsertString (hWndNextCheck, 1, L"After 15 Minutes");
      ComboBox_InsertString (hWndNextCheck, 2, L"After 1 Hour");
      ComboBox_InsertString (hWndNextCheck, 3, L"After 12 Hours");
      ComboBox_InsertString (hWndNextCheck, 4, L"Tomorrow");
      ComboBox_InsertString (hWndNextCheck, 5, L"Never");

      ComboBox_SetCurSel (hWndNextCheck, 0);

      return 0;
    } break;

    case WM_COMMAND:
    {
      if (LOWORD (wParam) == IDOK)
      {
        FILETIME                  ftNow;
        GetSystemTimeAsFileTime (&ftNow);

        LARGE_INTEGER next_li {
          ftNow.dwLowDateTime, ftNow.dwHighDateTime
        };

        LONGLONG next =
          next_li.QuadPart;

        bool never = false;

        const LONGLONG _Minute =   600000000LL;
        const LONGLONG _Hour   = 36000000000LL;

        switch (ComboBox_GetCurSel (hWndNextCheck))
        {
          // Immediate
          case 0:
          default:
            break;

          case 1:
            next += (15 * _Minute);
            break;

          case 2:
            next += (1 * _Hour);
            break;

          case 3:
            next += (12 * _Hour);
            break;

          case 4:
            next += (24 * _Hour);
            break;

          case 5:
            next  = -1;
            never = true;
            break;
        }

        wchar_t wszInstallFile [MAX_PATH] = { L'\0' };

        wsprintf ( wszInstallFile,
              L"%s\\Version\\installed.ini",
                SK_GetRootPath () );

        iSK_INI install_ini (wszInstallFile);

        install_ini.parse ();

        bool empty = false;

        if (! install_ini.get_sections ().size ())
          empty = true;

        iSK_INISection& user_prefs =
          install_ini.get_section (L"Update.User");

        if (empty)
          user_prefs.set_name (L"Update.User");

        sk::ParameterFactory ParameterFactory;

        if (! never) {
          sk::ParameterInt64* remind_time =
            (sk::ParameterInt64 *)
              ParameterFactory.create_parameter <int64_t> (L"Reminder");

          remind_time->register_to_ini (
            &install_ini,
              L"Update.User",
                L"Reminder"
          );

          remind_time->set_value (next);
          remind_time->store     ();

          delete remind_time;
        }

        else {
          sk::ParameterStringW* frequency =
            (sk::ParameterStringW *)
              ParameterFactory.create_parameter <std::wstring> (
                L"Frequency"
              );

          frequency->register_to_ini (
            &install_ini,
              L"Update.User",
                L"Frequency"
          );

          user_prefs.remove_key (L"Reminder");

          frequency->set_value (L"never");
          frequency->store     ();

          delete frequency;
        }

        install_ini.write (wszInstallFile);

        EndDialog (hWndDlg, 0);
        hWndRemind = (HWND)INVALID_HANDLE_VALUE;

        return 0;
      }
    } break;
  }

  return 0;
}

HRESULT
CALLBACK
DownloadDialogCallback (
  _In_ HWND     hWnd,
  _In_ UINT     uNotification,
  _In_ WPARAM   wParam,
  _In_ LPARAM   lParam,
  _In_ LONG_PTR dwRefData )
{
  sk_internet_get_t* get =
    (sk_internet_get_t *)dwRefData;


  // Don't allow this window to be used while the
  //   reminder dialog is visible.
  if (IsWindow (hWndRemind))
    return S_FALSE;


  extern DWORD WINAPI SK_RealizeForegroundWindow (HWND);

  if (uNotification == TDN_TIMER)
  {
    DWORD dwProcId;
    DWORD dwThreadId =
      GetWindowThreadProcessId (GetForegroundWindow (), &dwProcId);

    if ( dwProcId               == GetCurrentProcessId () &&
         GetForegroundWindow () != GetDesktopWindow    () ) {
      SK_RealizeForegroundWindow (hWnd);
    }

    if ( get->status == STATUS_UPDATED   ||
         get->status == STATUS_CANCELLED ||
         get->status == STATUS_REMINDER ) {
      EndDialog ( hWnd, 0 );
      return S_OK;
    }
  }

  if (uNotification == TDN_HYPERLINK_CLICKED) {
    ShellExecuteW (nullptr, L"open", (wchar_t *)lParam, nullptr, nullptr, SW_SHOW);

    return S_OK;
  }

  if (uNotification == TDN_DIALOG_CONSTRUCTED) {
    SendMessage (hWnd, TDM_SET_PROGRESS_BAR_RANGE, 0L,          MAKEWPARAM (0, 1));
    SendMessage (hWnd, TDM_SET_PROGRESS_BAR_POS,   1,           0L);
    SendMessage (hWnd, TDM_SET_PROGRESS_BAR_STATE, PBST_PAUSED, 0L);

    return S_OK;
  }

  if (uNotification == TDN_CREATED) {
    SK_RealizeForegroundWindow (hWnd);

    return S_OK;
  }

  if (uNotification == TDN_BUTTON_CLICKED)
  {
    if (wParam == ID_REMIND)
    {
      extern HMODULE
      __stdcall
      SK_GetDLL (void);

      hWndRemind =
        CreateDialog ( SK_GetDLL (),
                         MAKEINTRESOURCE (IDD_REMIND_ME_LATER),
                           hWnd,
                             RemindMeLater_DlgProc );

      get->status = STATUS_REMINDER;
    }

    else if (wParam == IDYES)
    {
      get->hTaskDlg = hWnd;

      _beginthreadex ( nullptr,
                         0,
                          DownloadThread,
                            (LPVOID)get,
                              0x00,
                              nullptr );
    }

    else {
      //get->status = STATUS_CANCELLED;
      //return S_OK;
    }
  }

  if (uNotification == TDN_DESTROYED) {
    //sk_internet_get_t* get =
      //(sk_internet_get_t *)dwRefData;

    //delete get;
  }

  return S_FALSE;
}

HRESULT
__stdcall
SK_UpdateSoftware (const wchar_t* wszProduct)
{
  int               nButtonPressed =   0;
  TASKDIALOGCONFIG  task_config    = { 0 };

  int idx = 0;

  task_config.cbSize             = sizeof (task_config);
  task_config.hInstance          = GetModuleHandle (nullptr);
  task_config.hwndParent         = GetDesktopWindow ();
  task_config.pszWindowTitle     = L"Special K Auto-Update";

  TASKDIALOG_BUTTON buttons [3];

  buttons [0].nButtonID     = IDYES;
  buttons [0].pszButtonText = L"Yes";

  buttons [1].nButtonID     = 0;
  buttons [1].pszButtonText = L"Remind me later (or disable)";


  task_config.pButtons           = buttons;
  task_config.cButtons           = 2;
  task_config.dwCommonButtons    = 0x00;
  task_config.nDefaultButton     = 0;

  task_config.dwFlags            = TDF_ENABLE_HYPERLINKS | TDF_SHOW_PROGRESS_BAR   | TDF_SIZE_TO_CONTENT |
                                   TDF_CALLBACK_TIMER    | TDF_EXPANDED_BY_DEFAULT | TDF_EXPAND_FOOTER_AREA;
  task_config.pfCallback         = DownloadDialogCallback;

  task_config.pszMainInstruction = L"Software Update Available for Download";
  task_config.pszMainIcon        = TD_INFORMATION_ICON;

  task_config.pszContent         = L"Would you like to upgrade now?";

  wchar_t wszRepoFile    [MAX_PATH] = { L'\0' };
  wchar_t wszInstallFile [MAX_PATH] = { L'\0' };

  wsprintf ( wszRepoFile,
              L"%s\\Version\\repository.ini",
                SK_GetRootPath () );

  wsprintf ( wszInstallFile,
              L"%s\\Version\\installed.ini",
                SK_GetRootPath () );

  iSK_INI install_ini (wszInstallFile);
  iSK_INI repo_ini    (wszRepoFile);

  install_ini.parse ();
  repo_ini.parse    ();

  struct {
    int installed;

    struct {
      int     in_branch; // TODO
      int     overall;
      wchar_t package [128];
    } latest;
  } build;

  bool empty = false;

  if (! install_ini.get_sections ().size ())
    empty = true;

  // Not exactly empty, but certainly not in a working state.
  if (! install_ini.contains_section (L"Version.Local"))
    empty = true;

  iSK_INISection& installed_ver =
    install_ini.get_section (L"Version.Local");

  if (empty)
    installed_ver.set_name (L"Version.Local");

  wchar_t wszDontCare [128];

  swscanf ( installed_ver.get_value (L"InstallPackage").c_str (),
              L"%128[^,],%lu",
                wszDontCare,
                  &build.installed );

  if (empty)
    build.installed = -1;

  // Set the reminder in case the update fails... we do not want
  //   the repository.ini file's updated timestamp to delay a second
  //     attempt by the user to upgrade.
  install_ini.import (L"[Update.User]\nReminder=0\n");
  install_ini.write  (wszInstallFile);

  iSK_INISection& latest_ver =
    repo_ini.get_section (L"Version.Latest");

  wchar_t wszFullDetails [4096];
  wsprintf ( wszFullDetails,
              L"<a href=\"%s\">%s</a>\n\n"
              L"%s",
                latest_ver.get_value (L"ReleaseNotes").c_str    (),
                  latest_ver.get_value (L"Title").c_str         (),
                    latest_ver.get_value (L"Description").c_str () );

  swscanf ( latest_ver.get_value (L"InstallPackage").c_str (),
              L"%128[^,],%lu",
                build.latest.package,
                  &build.latest.in_branch );

  if (build.latest.in_branch > build.installed) {
    wchar_t wszArchiveName [MAX_PATH];
    wsprintf ( wszArchiveName,
                L"Archive.%s",
                  build.latest.package );

    iSK_INISection& archive =
      repo_ini.get_section (wszArchiveName);

    sk_internet_get_t* get =
      new sk_internet_get_t;

    URL_COMPONENTSW    urlcomps;

    ZeroMemory (get,       sizeof *get);
    ZeroMemory (&urlcomps, sizeof URL_COMPONENTSW);

    urlcomps.dwStructSize     = sizeof URL_COMPONENTSW;

    urlcomps.lpszHostName     = get->wszHostName;
    urlcomps.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH;

    urlcomps.lpszUrlPath      = get->wszHostPath;
    urlcomps.dwUrlPathLength  = INTERNET_MAX_PATH_LENGTH;

    if ( InternetCrackUrl ( archive.get_value (L"URL").c_str (),
                              archive.get_value (L"URL").length (),
                                0x00,
                                  &urlcomps ) )
    {
      task_config.lpCallbackData = (LONG_PTR)get;

      wchar_t   wszUpdateFile [MAX_PATH] = { L'\0' };

      lstrcatW (wszUpdateFile, SK_GetRootPath ());
      lstrcatW (wszUpdateFile, L"Version\\");

      wchar_t wszUpdateTempFile [MAX_PATH];

      swprintf ( wszUpdateTempFile, 
                   L"%s\\Version\\%s.7z",
                     SK_GetRootPath (),
                       build.latest.package );

      wcsncpy (get->wszLocalPath, wszUpdateTempFile, MAX_PATH - 1);

      task_config.pszExpandedInformation  = wszFullDetails;

      task_config.pszExpandedControlText  = L"Hide Details";
      task_config.pszCollapsedControlText = L"Show More Details";

      int nButton = 0;

      if (SUCCEEDED (TaskDialogIndirect (&task_config, &nButton, nullptr, nullptr))) {
        if (get->status == STATUS_UPDATED) {
          ShellExecuteW ( GetDesktopWindow (),
                            L"Open",
                              wszUpdateTempFile,
                                nullptr,
                                  nullptr,
                                    SW_SHOWNORMAL );

          if (empty) {
            install_ini.import ( L"[Version.Local]\n"
                                 L"Branch=Latest\n"
                                 L"InstallPackage= \n\n"

                                 L"[Update.User]\n"
                                 L"Frequency=6h\n\n" );
          }

          installed_ver.get_value (L"InstallPackage") =
            latest_ver.get_value (L"InstallPackage");

          // Remove reminder if we successfully install...
          install_ini.get_section (L"Update.User").remove_key (L"Reminder");
          install_ini.write (wszInstallFile);

          TerminateProcess (GetCurrentProcess (), 0x00);
        }
      }

      delete get;
    }

    else {
      delete get;
      return E_FAIL;
    }
  }

  return S_FALSE;
}