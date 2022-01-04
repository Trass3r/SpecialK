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

#include <SpecialK/stdafx.h>
#include <storefront/epic.h>
#include <storefront/achievements.h>

#ifdef  __SK_SUBSYSTEM__
#undef  __SK_SUBSYSTEM__
#endif
#define __SK_SUBSYSTEM__ L"EpicOnline"

class SK_EOS_AchievementManager : public SK_AchievementManager
{
public:
  void unlock (const char* szAchievement)
  {
    std::ignore = szAchievement;

    if (config.steam.achievements.play_sound)
    {
      SK_PlaySound ( (LPCWSTR)unlock_sound, nullptr, SND_ASYNC |
                                                     SND_MEMORY );
    }

    // If the user wants a screenshot, but no popups (why?!), this is when
    //   the screenshot needs to be taken.
    if (       config.steam.achievements.take_screenshot )
    {
      SK::SteamAPI::TakeScreenshot ();
    }
  }

  static EOS_Achievements_GetPlayerAchievementCount_pfn   GetPlayerAchievementCount;;
  static EOS_Achievements_GetUnlockedAchievementCount_pfn GetUnlockedAchievementCount;;
};

class SK_EOS_OverlayManager
{
public:
  bool isActive (void) const {
    return active_;
  }

  void OnActivate (const EOS_UI_OnDisplaySettingsUpdatedCallbackInfo* Data)
  {
    std::lock_guard <SK_Thread_HybridSpinlock>
         lock (callback_cs);

    // If the game has an activation callback installed, then
    //   it's also going to see this event... make a note of that when
    //     tracking its believed overlay state.
    if (SK::EOS::IsOverlayAware ())
        SK::EOS::overlay_state = (Data->bIsExclusiveInput);


    // If we want to use this as our own, then don't let the Epic overlay
    //   unpause the game on deactivation unless the control panel is closed.
    if (config.steam.reuse_overlay_pause && SK::EOS::IsOverlayAware ())
    {
      // Deactivating, but we might want to hide this event from the game...
      if (Data->bIsVisible == 0)
      {
        extern bool SK_ImGui_Visible;

        // Undo the event the game is about to receive.
        if (SK_ImGui_Visible) SK::EOS::SetOverlayState (true);
      }
    }

    const bool wasActive =
                  active_;

    active_ = (Data->bIsExclusiveInput != 0);

    if (wasActive != active_)
    {
      auto& io =
        ImGui::GetIO ();

      static bool capture_keys  = io.WantCaptureKeyboard;
      static bool capture_text  = io.WantTextInput;
      static bool capture_mouse = io.WantCaptureMouse;
      static bool nav_active    = io.NavActive;

      // When the overlay activates, stop blocking
      //   input !!
      if (! wasActive)
      {
        capture_keys  =
          io.WantCaptureKeyboard;
          io.WantCaptureKeyboard = false;

        capture_text  =
          io.WantTextInput;
          io.WantTextInput       = false;

        capture_mouse =
          io.WantCaptureMouse;
          io.WantCaptureMouse    = false;

        nav_active    =
          io.NavActive;
          io.NavActive           = false;

         ImGui::SetWindowFocus (nullptr);
      }

      else
      {
        io.WantCaptureKeyboard = SK_ImGui_Visible ? capture_keys  : false;
        io.WantCaptureMouse    = SK_ImGui_Visible ? capture_mouse : false;
        io.NavActive           = SK_ImGui_Visible ? nav_active    : false;

        ImGui::SetWindowFocus (nullptr);
        io.WantTextInput       = false;//capture_text;
      }
    }
  }

  void invokeCallbacks (bool active)
  {
    std::lock_guard <SK_Thread_HybridSpinlock>
         lock (callback_cs);

    EOS_UI_OnDisplaySettingsUpdatedCallbackInfo
      cbi                   = {    };
      cbi.bIsVisible        = active;
      cbi.bIsExclusiveInput = active;

    for ( const auto &[id,callback] : callbacks )
    {
      cbi.ClientData =
        callback.ClientData;

      callback.NotificationFn (&cbi);
    }
  }

  EOS_NotificationId
  AddNotifyDisplaySettingsUpdated (EOS_HUI                                        Handle,
                             const EOS_UI_AddNotifyDisplaySettingsUpdatedOptions* Options,
                                   void*                                          ClientData,
                             const EOS_UI_OnDisplaySettingsUpdatedCallback        NotificationFn)
  {
    const EOS_NotificationId id =
      AddNotifyDisplaySettingsUpdated_Original (Handle, Options, ClientData, NotificationFn);

    std::lock_guard <SK_Thread_HybridSpinlock>
         lock (callback_cs);

    if (id != 0)
    {
      callbacks [id] =
        { id, Handle, ClientData, NotificationFn, *Options };
    }

    return id;
  }

  void
  RemoveNotifyDisplaySettingsUpdated (EOS_HUI            Handle,
                                      EOS_NotificationId Id)
  {
    RemoveNotifyDisplaySettingsUpdated_Original (Handle, Id);

    std::lock_guard <SK_Thread_HybridSpinlock>
         lock (callback_cs);

    if ( const auto cb  = callbacks.find (Id);
                    cb != callbacks.end  (  ) )
    {
      SK_ReleaseAssert (Handle == cb->second.Handle);

      callbacks.erase (cb);
    }
  }

  bool
  isOverlayAware (void) const
  {
    return
      (! callbacks.empty ());
  }

private:
  bool cursor_visible_ = false; // Cursor visible prior to activation?
  bool active_         = false;

  struct callback_s
  {
    EOS_NotificationId                            Id;
    EOS_HUI                                       Handle;
    void*                                         ClientData;
    EOS_UI_OnDisplaySettingsUpdatedCallback       NotificationFn;
    EOS_UI_AddNotifyDisplaySettingsUpdatedOptions Options;
  };

  std::unordered_map <EOS_NotificationId, callback_s> callbacks;
  SK_Thread_HybridSpinlock                            callback_cs;

public:
  static EOS_UI_AddNotifyDisplaySettingsUpdated_pfn    AddNotifyDisplaySettingsUpdated_Original;
  static EOS_UI_RemoveNotifyDisplaySettingsUpdated_pfn RemoveNotifyDisplaySettingsUpdated_Original;
};

SK_LazyGlobal <SK_EOS_OverlayManager>     eos_overlay;
SK_LazyGlobal <SK_EOS_AchievementManager> eos_achievements;

EOS_UI_AddNotifyDisplaySettingsUpdated_pfn    SK_EOS_OverlayManager::AddNotifyDisplaySettingsUpdated_Original    = nullptr;
EOS_UI_RemoveNotifyDisplaySettingsUpdated_pfn SK_EOS_OverlayManager::RemoveNotifyDisplaySettingsUpdated_Original = nullptr;

EOS_Achievements_GetPlayerAchievementCount_pfn   SK_EOS_AchievementManager::GetPlayerAchievementCount   = nullptr;
EOS_Achievements_GetUnlockedAchievementCount_pfn SK_EOS_AchievementManager::GetUnlockedAchievementCount = nullptr;

// Cache this instead of getting it from the Steam client constantly;
//   doing that is far more expensive than you would think.
size_t
SK_EOS_GetNumPossibleAchievements (void)
{
  if (eos_achievements->GetPlayerAchievementCount == nullptr)
    return 0;

  static std::pair <size_t,bool> possible =
  { 0, false };

  if ( possible.second          == false   &&
       epic_ctx.Achievements () != nullptr )
  {
    //epic_achievements->getAchievements (&possible.first);
    //possible = { possible.first, true };

    EOS_Achievements_GetPlayerAchievementCountOptions opt =
   {EOS_ACHIEVEMENTS_GETPLAYERACHIEVEMENTCOUNT_API_LATEST};

    possible.first  = eos_achievements->GetPlayerAchievementCount (epic_ctx.Achievements (), &opt);

    if (possible.first != 0)
        possible.second = true;
  }

  return possible.first;
}

void
EOS_CALL
SK_EOS_UI_OnDisplaySettingsUpdatedCallback_Proxy (const EOS_UI_OnDisplaySettingsUpdatedCallbackInfo* Data)
{
  SK_ReleaseAssert (Data->ClientData == nullptr || Data->ClientData == eos_overlay.getPtr ());

  epic_log->Log (
    L"SK_EOS_UI_OnDisplaySettingsUpdatedCallback_Proxy ({ bIsVisible=%i, bIsExclusiveInput=%i }",
      Data->bIsVisible, Data->bIsExclusiveInput
  );

  eos_overlay->OnActivate (Data);
}

void
EOS_CALL
SK_EOS_Achievements_OnAchievementsUnlockedCallbackV2_Proxy (const EOS_Achievements_OnAchievementsUnlockedCallbackV2Info* Data)
{
  SK_ReleaseAssert (Data->ClientData == nullptr || Data->ClientData == eos_achievements.getPtr ());

  epic_log->Log (
    L"EOS_Achievements_OnAchievementsUnlockedCallbackV2_Proxy ({ Achievement=%hs })",
      Data->AchievementId
  );

  eos_achievements->unlock (Data->AchievementId);
}

void
EOS_CALL
SK_EOS_UserInfo_QueryUserInfoCallback_Proxy (const EOS_UserInfo_QueryUserInfoCallbackInfo* Data)
{
  if (Data->ResultCode == EOS_EResult::EOS_Success)
  {
    SK_ReleaseAssert (Data->ClientData == nullptr || Data->ClientData == pEOSCtx.getPtr ());

    using EOS_UserInfo_CopyUserInfo_pfn = EOS_EResult (EOS_CALL *)(EOS_HUserInfo                     Handle,
                                                             const EOS_UserInfo_CopyUserInfoOptions* Options,
                                                                   EOS_UserInfo**                    OutUserInfo);
    auto  EOS_UserInfo_CopyUserInfo =
         (EOS_UserInfo_CopyUserInfo_pfn)SK_GetProcAddress (epic_ctx.GetEOSDLL (),
         "EOS_UserInfo_CopyUserInfo");

    using EOS_UserInfo_Release_pfn = void (EOS_CALL *)(EOS_UserInfo* UserInfo);
    auto  EOS_UserInfo_Release     =
         (EOS_UserInfo_Release_pfn)SK_GetProcAddress (epic_ctx.GetEOSDLL (),
         "EOS_UserInfo_Release");

    if (EOS_UserInfo_Release != nullptr && EOS_UserInfo_CopyUserInfo != nullptr)
    {
      EOS_UserInfo_CopyUserInfoOptions opts =
      {
        EOS_USERINFO_COPYUSERINFO_API_LATEST,
        Data->LocalUserId,
        Data->LocalUserId
      };

      EOS_UserInfo*                                              pUserInfo = nullptr;
      EOS_EResult result =
        EOS_UserInfo_CopyUserInfo (epic_ctx.UserInfo (), &opts, &pUserInfo);

      if (result == EOS_EResult::EOS_Success)
      {
        pEOSCtx->user.display_name = pUserInfo->DisplayName;
        pEOSCtx->user.nickname     = pUserInfo->Nickname;

        EOS_UserInfo_Release (pUserInfo);
      }
    }
  }

  //SK_ReleaseAssert (Data->ResultCode == EOS_EResult::EOS_Success);
}

bool
WINAPI
SK_IsEpicOverlayActive (void)
{
  return eos_overlay->isActive ();
}

void
SK_EOS_InvokeOverlayActivationCallback (bool active)
{
  auto orig_se =
  SK_SEH_ApplyTranslator (
    SK_FilteringStructuredExceptionTranslator (EXCEPTION_ACCESS_VIOLATION)
  );
  try
  {
    eos_overlay->invokeCallbacks (active);

    SK::EOS::overlay_state = active;
  }

  catch (const SK_SEH_IgnoredException&)
  {
    // Oh well, we literally tried...
  }
  SK_SEH_RemoveTranslator (orig_se);
}

void
__stdcall
SK::EOS::SetOverlayState (bool active)
{
  if (config.steam.silent)
    return;

  eos_overlay->invokeCallbacks (active);
}

bool
__stdcall
SK::EOS::GetOverlayState (bool real)
{
  return real ? SK_IsEpicOverlayActive () :
    overlay_state;
}

bool
__stdcall
SK::EOS::IsOverlayAware (void)
{
  return
    eos_overlay->isOverlayAware ();
}


EOS_NotificationId
EOS_CALL
EOS_UI_AddNotifyDisplaySettingsUpdated_Detour (EOS_HUI                                        Handle,
                                         const EOS_UI_AddNotifyDisplaySettingsUpdatedOptions* Options,
                                               void*                                          ClientData,
                                         const EOS_UI_OnDisplaySettingsUpdatedCallback        NotificationFn)
{
  epic_log->Log (L"EOS_UI_AddNotifyDisplaySettingsUpdated");

  return
    eos_overlay->AddNotifyDisplaySettingsUpdated (Handle, Options, ClientData, NotificationFn);
}

void
EOS_CALL
EOS_UI_RemoveNotifyDisplaySettingsUpdated_Detour (EOS_HUI            Handle,
                                                  EOS_NotificationId Id)
{
  epic_log->Log (L"EOS_UI_RemoveNotifyDisplaySettingsUpdated");

  return
    eos_overlay->RemoveNotifyDisplaySettingsUpdated (Handle, Id);
}

EOS_Initialize_pfn       EOS_Initialize_Original       = nullptr;
EOS_Shutdown_pfn         EOS_Shutdown_Original         = nullptr;
EOS_Platform_Tick_pfn    EOS_Platform_Tick_Original    = nullptr;
EOS_Platform_Create_pfn  EOS_Platform_Create_Original  = nullptr;
EOS_Platform_Release_pfn EOS_Platform_Release_Original = nullptr;

EOS_EResult
EOS_CALL
EOS_Initialize_Detour (const EOS_InitializeOptions* Options)
{
  epic_log->Log (L"EOS_Initialize");

  return
    EOS_Initialize_Original (Options);
}

EOS_EResult
EOS_CALL
EOS_Shutdown_Detour (void)
{
  epic_log->Log (L"EOS_Shutdown");

  return
    EOS_Shutdown_Original ();
}

volatile LONGLONG __SK_EOS_Ticks = 0;

LONGLONG
SK::EOS::GetTicksRetired (void)
{
  return
    ReadAcquire64 (&__SK_EOS_Ticks);
}

void
EOS_CALL
EOS_Platform_Tick_Detour (EOS_HPlatform Handle)
{
  SK_RunOnce (epic_log->Log (L"EOS_Platform_Tick"));

  if (epic_ctx.Platform () == nullptr)
      epic_ctx.InitEpicOnlineServices (nullptr, Handle);

  InterlockedIncrement64 (&__SK_EOS_Ticks);

  return
    EOS_Platform_Tick_Original (Handle);
}

EOS_HPlatform
EOS_CALL
EOS_Platform_Create_Detour (const EOS_Platform_Options* Options)
{
  epic_log->Log (L"EOS_Platform_Create");

  return
    EOS_Platform_Create_Original (Options);
}

void
EOS_CALL
EOS_Platform_Release_Detour (EOS_HPlatform Handle)
{
  epic_log->Log (L"EOS_Platform_Release");

  return
    EOS_Platform_Release_Original (Handle);
}

void
SK::EOS::Init (bool pre_load)
{
  if (config.steam.silent)
    return;

  const wchar_t*
    wszEOSDLLName =
      SK_RunLHIfBitness ( 64, L"EOSSDK-Win64-Shipping.dll",
                              L"EOSSDK-Win32-Shipping.dll" );

  if ((! pre_load) && (! SK_GetModuleHandle (wszEOSDLLName)))
    return;

  HMODULE hModEOS =
    SK_LoadLibraryW (wszEOSDLLName);

  if ((! pre_load) && hModEOS != nullptr)
  {
    epic_log->init (L"logs/eos.log", L"wt+,ccs=UTF-8");
    epic_log->silent = config.steam.silent;

    epic_ctx.PreInit (hModEOS);

    /* Since we probably missed the opportunity to catch EOS_Platform_Create,
         hook EOS_Platform_Tick and watch for the game's EOS_HPlatform */

    SK_CreateDLLHook2 ( wszEOSDLLName, "EOS_Initialize",
                                        EOS_Initialize_Detour,
               static_cast_p2p <void> (&EOS_Initialize_Original) );

    SK_CreateDLLHook2 ( wszEOSDLLName, "EOS_Shutdown",
                                        EOS_Shutdown_Detour,
               static_cast_p2p <void> (&EOS_Shutdown_Original) );

    SK_CreateDLLHook2 ( wszEOSDLLName, "EOS_Platform_Tick",
                                        EOS_Platform_Tick_Detour,
               static_cast_p2p <void> (&EOS_Platform_Tick_Original) );

    SK_CreateDLLHook2 ( wszEOSDLLName, "EOS_Platform_Create",
                                        EOS_Platform_Create_Detour,
               static_cast_p2p <void> (&EOS_Platform_Create_Original) );

    SK_CreateDLLHook2 ( wszEOSDLLName, "EOS_Platform_Release",
                                        EOS_Platform_Release_Detour,
               static_cast_p2p <void> (&EOS_Platform_Release_Original) );

    SK_CreateDLLHook2 ( wszEOSDLLName, "EOS_Platform_Release",
                                        EOS_Platform_Release_Detour,
               static_cast_p2p <void> (&EOS_Platform_Release_Original) );

    SK_CreateDLLHook2 ( wszEOSDLLName, "EOS_UI_AddNotifyDisplaySettingsUpdated",
                                        EOS_UI_AddNotifyDisplaySettingsUpdated_Detour,
         static_cast_p2p <void> (&eos_overlay->AddNotifyDisplaySettingsUpdated_Original) );

    SK_CreateDLLHook2 ( wszEOSDLLName, "EOS_UI_RemoveNotifyDisplaySettingsUpdated",
                                        EOS_UI_RemoveNotifyDisplaySettingsUpdated_Detour,
         static_cast_p2p <void> (&eos_overlay->RemoveNotifyDisplaySettingsUpdated_Original) );

    eos_achievements->GetUnlockedAchievementCount = (EOS_Achievements_GetUnlockedAchievementCount_pfn)
      SK_GetProcAddress (wszEOSDLLName, "EOS_Achievements_GetUnlockedAchievementCount");

    eos_achievements->GetPlayerAchievementCount   = (EOS_Achievements_GetPlayerAchievementCount_pfn)
      SK_GetProcAddress (wszEOSDLLName, "EOS_Achievements_GetPlayerAchievementCount");

    SK_ApplyQueuedHooks ();
  }

  // Preloading not supported, SK has no EOS Credentials
  //
  //if (epic_ctx.InitEpicOnlineServices (hModEOS))
  //{
  //  EOS_InitializeOptions
  //}
}

void
SK::EOS::Shutdown (void)
{
  epic_log->Log (L"STUB Shutdown");
  //epic_ctx.Shutdown ();
}

void
SK_EOSContext::PreInit (HMODULE hEOSDLL)
{
  sdk_dll_ = hEOSDLL;
}

bool
SK_EOSContext::InitEpicOnlineServices (HMODULE hEOSDLL, EOS_HPlatform platform)
{
  // If we were a registered EOS product, this is where we would init...
  if (platform == nullptr)
  {
    auto Initialize =
    (EOS_Initialize_pfn)SK_GetProcAddress (hEOSDLL,
    "EOS_Initialize");

    if (Initialize == nullptr)
      return false;

    // We need the game to initialize this, we don't have an actual product id :)
    if (Initialize (nullptr) != EOS_EResult::EOS_AlreadyConfigured)
      return false;

    auto Platform_Create =
    (EOS_Platform_Create_pfn)SK_GetProcAddress (hEOSDLL,
    "EOS_Platform_Create");

    if (Platform_Create == nullptr)
      return false;
  }

  // But we're not, so we will yoink the game's HPlatform instance
  platform_ = platform;

  auto  EOS_Platform_GetUIInterface =
       (EOS_Platform_GetUIInterface_pfn)SK_GetProcAddress (sdk_dll_,
       "EOS_Platform_GetUIInterface");

  if (EOS_Platform_GetUIInterface != nullptr) ui_ =
      EOS_Platform_GetUIInterface (platform_);

  if (ui_ != nullptr)
  {
    EOS_UI_AddNotifyDisplaySettingsUpdatedOptions opts =
      { EOS_UI_ADDNOTIFYDISPLAYSETTINGSUPDATED_API_LATEST };

    eos_overlay->AddNotifyDisplaySettingsUpdated_Original (
      ui_, &opts, nullptr,//eos_overlay.getPtr (),
        SK_EOS_UI_OnDisplaySettingsUpdatedCallback_Proxy  );
  }

  auto  EOS_Platform_GetAchievementsInterface =
       (EOS_Platform_GetAchievementsInterface_pfn)SK_GetProcAddress (sdk_dll_,
       "EOS_Platform_GetAchievementsInterface");

  if (EOS_Platform_GetAchievementsInterface != nullptr) achievements_ =
      EOS_Platform_GetAchievementsInterface (platform_);

  if (achievements_ != nullptr)
  {
    using EOS_Achievements_AddNotifyAchievementsUnlockedV2_pfn =
      EOS_NotificationId (EOS_CALL *)(      EOS_HAchievements                                        Handle,
                                      const EOS_Achievements_AddNotifyAchievementsUnlockedV2Options* Options,
                                            void*                                                    ClientData,
                                      const EOS_Achievements_OnAchievementsUnlockedCallbackV2        NotificationFn);

    auto EOS_Achievements_AddNotifyAchievementsUnlockedV2 =
        (EOS_Achievements_AddNotifyAchievementsUnlockedV2_pfn)SK_GetProcAddress (sdk_dll_,
        "EOS_Achievements_AddNotifyAchievementsUnlockedV2");

    if (EOS_Achievements_AddNotifyAchievementsUnlockedV2 != nullptr)
    {
      EOS_Achievements_AddNotifyAchievementsUnlockedV2Options opts =
        { EOS_ACHIEVEMENTS_ADDNOTIFYACHIEVEMENTSUNLOCKEDV2_API_LATEST };

      EOS_Achievements_AddNotifyAchievementsUnlockedV2 (
        achievements_, &opts, nullptr,//eos_achievements.getPtr (),
          SK_EOS_Achievements_OnAchievementsUnlockedCallbackV2_Proxy
      );

      eos_achievements->loadSound (config.steam.achievements.sound_file.c_str ());
    }
  }

  auto EOS_Platform_GetAuthInterface =
      (EOS_Platform_GetAuthInterface_pfn)SK_GetProcAddress     (sdk_dll_,
      "EOS_Platform_GetAuthInterface");
  auto EOS_Platform_GetFriendsInterface =
      (EOS_Platform_GetFriendsInterface_pfn)SK_GetProcAddress  (sdk_dll_,
      "EOS_Platform_GetFriendsInterface");
  auto EOS_Platform_GetStatsInterface =
      (EOS_Platform_GetStatsInterface_pfn)SK_GetProcAddress    (sdk_dll_,
      "EOS_Platform_GetStatsInterface");
  auto EOS_Platform_GetUserInfoInterface =
      (EOS_Platform_GetUserInfoInterface_pfn)SK_GetProcAddress (sdk_dll_,
      "EOS_Platform_GetUserInfoInterface");

  if (EOS_Platform_GetAuthInterface     != nullptr)     auth_      =
      EOS_Platform_GetAuthInterface       (platform_);
  if (EOS_Platform_GetFriendsInterface  != nullptr)     friends_   =
      EOS_Platform_GetFriendsInterface    (platform_);
  if (EOS_Platform_GetStatsInterface    != nullptr)     stats_     =
      EOS_Platform_GetStatsInterface      (platform_);
  if (EOS_Platform_GetUserInfoInterface != nullptr)     user_info_ =
      EOS_Platform_GetUserInfoInterface   (platform_);


  if (auth_ != nullptr)
  {
    using EOS_Auth_GetLoggedInAccountsCount_pfn = int32_t (EOS_CALL *)(EOS_HAuth Handle);
    auto  EOS_Auth_GetLoggedInAccountsCount =
         (EOS_Auth_GetLoggedInAccountsCount_pfn)SK_GetProcAddress (sdk_dll_,
         "EOS_Auth_GetLoggedInAccountsCount");

    int32_t logins = EOS_Auth_GetLoggedInAccountsCount != nullptr ?
                     EOS_Auth_GetLoggedInAccountsCount (auth_)    : 0;

    SK_ReleaseAssert (logins <= 1);

    using EOS_Auth_GetLoggedInAccountByIndex_pfn = EOS_EpicAccountId (EOS_CALL *)(EOS_HAuth Handle, int32_t Index);
    auto  EOS_Auth_GetLoggedInAccountByIndex =
         (EOS_Auth_GetLoggedInAccountByIndex_pfn)SK_GetProcAddress (sdk_dll_,
         "EOS_Auth_GetLoggedInAccountByIndex");

    SK::EOS::player =
      EOS_Auth_GetLoggedInAccountByIndex (auth_, 0);

    if (user_info_ != nullptr)
    {
      using EOS_UserInfo_QueryUserInfo_pfn = void (EOS_CALL *)(EOS_HUserInfo                        Handle,
                                                         const EOS_UserInfo_QueryUserInfoOptions*   Options,
                                                               void*                                ClientData,
                                                         const EOS_UserInfo_OnQueryUserInfoCallback CompletionDelegate);

      auto EOS_UserInfo_QueryUserInfo =
          (EOS_UserInfo_QueryUserInfo_pfn)SK_GetProcAddress (sdk_dll_,
          "EOS_UserInfo_QueryUserInfo");

      if (EOS_UserInfo_QueryUserInfo != nullptr)
      {
        EOS_UserInfo_QueryUserInfoOptions
          opts = { EOS_USERINFO_QUERYUSERINFO_API_LATEST,
                   SK::EOS::player,
                   SK::EOS::player };

        EOS_UserInfo_QueryUserInfo (user_info_, &opts, nullptr/*this*/, SK_EOS_UserInfo_QueryUserInfoCallback_Proxy);
      }
    }
  }

  return true;
}


SK_LazyGlobal <SK_EOSContext> pEOSCtx;
bool SK::EOS::overlay_state = false;

bool SK_EOSContext::OnVarChange (SK_IVariable *, void *)
{
  return true;
}



std::string_view
SK::EOS::PlayerName (void)
{
  std::string_view view =
    pEOSCtx->GetDisplayName ();

  if (view.empty ())
    return "";

  return view;
}

std::string_view
SK::EOS::PlayerNickname (void)
{
  std::string_view view =
    pEOSCtx->GetNickName ();

  if (view.empty ())
    return "";

  return view;
}

EOS_EpicAccountId
SK::EOS::UserID (void)
{
  return
    SK::EOS::player;
}

#include <filesystem>

std::string
SK::EOS::AppName (void)
{
  static std::string name = "";

  if (                                                          name.empty () &&
      app_cache_mgr->getAppNameFromPath (SK_GetFullyQualifiedApp ()).empty ())
  {
    std::filesystem::path path =
      std::move (std::wstring (SK_GetFullyQualifiedApp ()));

    char szDisplayName [65] = { };
    char szEpicApp     [65] = { };

    try
    {
      while (! std::filesystem::equivalent ( path.parent_path    (),
                                             path.root_directory () ) )
      {
        if (std::filesystem::is_directory (path / L".egstore"))
        {
          for ( const auto& file : std::filesystem::directory_iterator (path / L".egstore") )
          {
            if (! file.is_regular_file ())
              continue;

            if (file.path ().extension ().compare (L".mancpn") == 0)
            {
              CRegKey hkManifestRoot;
                      hkManifestRoot.Open (HKEY_CURRENT_USER, LR"(Software\Epic Games\EOS)");

              wchar_t wszManifestPath [MAX_PATH + 2] = { };
              ULONG   ulManifestLen =  MAX_PATH;
              hkManifestRoot.QueryStringValue (L"ModSdkMetadataDir", wszManifestPath, &ulManifestLen);

              PathAppendW (wszManifestPath, file.path ().stem ().c_str ());
              StrCatW     (wszManifestPath, L".item");

              if (! std::filesystem::exists (wszManifestPath))
                break;

              if (std::fstream mancpn (wszManifestPath, std::fstream::in);
                               mancpn.is_open ())
              {
                char                     szLine [512] = { };
                while (! mancpn.getline (szLine, 511).eof ())
                {
                  if (StrStrIA (szLine, "\"DisplayName\"") != nullptr)
                  {
                    const char      *substr =     StrStrIA (szLine, ":");
                    strncpy_s (szDisplayName, 64, StrStrIA (substr, "\"") + 1, _TRUNCATE);
                     *strrchr (szDisplayName, '"') = '\0';
                    continue;
                  }

                  else if (StrStrIA (szLine, "\"AppName\"") != nullptr)
                  {
                    const char      *substr = StrStrIA (szLine, ":");
                    strncpy_s (szEpicApp, 64, StrStrIA (substr, "\"") + 1, _TRUNCATE);
                     *strrchr (szEpicApp, '"') = '\0';
                    continue;
                  }

                  if (*szDisplayName != '\0' && *szEpicApp != '\0')
                  {
                    app_cache_mgr->addAppToCache (
                        SK_GetFullyQualifiedApp (),
                                  SK_GetHostApp (),
                              SK_UTF8ToWideChar (szDisplayName).c_str (),
                                                 szEpicApp
                    );

                    name = szDisplayName;
                    break;
                  }
                }

                path = LR"(\)";
                break;
              }
            }
          }
        }

        path =
          path.parent_path ();
      }

      app_cache_mgr->saveAppCache       ();
      app_cache_mgr->loadAppCacheForExe (SK_GetFullyQualifiedApp ());

      // Trigger profile migration if necessary
      app_cache_mgr->getConfigPathForEpicApp (szEpicApp);
    }

    catch (...)
    {

    }
  }

  return
    name;
}

EOS_EpicAccountId SK::EOS::player = 0;