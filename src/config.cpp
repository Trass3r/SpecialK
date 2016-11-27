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
#include <SpecialK/config.h>
#include <SpecialK/core.h>
#include <SpecialK/parameter.h>
#include <SpecialK/import.h>
#include <SpecialK/utility.h>
#include <SpecialK/ini.h>
#include <SpecialK/log.h>
#include <SpecialK/steam_api.h>

#include <SpecialK/DLL_VERSION.H>

const wchar_t*       SK_VER_STR = SK_VERSION_STR_W;

iSK_INI*             dll_ini         = nullptr;
iSK_INI*             osd_ini         = nullptr;
iSK_INI*             achievement_ini = nullptr;
sk_config_t          config;
sk::ParameterFactory g_ParameterFactory;

extern
bool
__stdcall
SK_IsInjected (void);

struct {
  struct {
    sk::ParameterBool*    show;
  } time;

  struct {
    sk::ParameterBool*    show;
    sk::ParameterFloat*   interval;
  } io;

  struct {
    sk::ParameterBool*    show;
  } fps;

  struct {
    sk::ParameterBool*    show;
  } memory;

  struct {
    sk::ParameterBool*    show;
  } SLI;

  struct {
    sk::ParameterBool*    show;
    sk::ParameterFloat*   interval;
    sk::ParameterBool*    simple;
  } cpu;

  struct {
    sk::ParameterBool*    show;
    sk::ParameterBool*    print_slowdown;
    sk::ParameterFloat*   interval;
  } gpu;

  struct {
    sk::ParameterBool*    show;
    sk::ParameterFloat*   interval;
    sk::ParameterInt*     type;
  } disk;

  struct {
    sk::ParameterBool*    show;
    sk::ParameterFloat*   interval;
  } pagefile;
} monitoring;

struct {
  sk::ParameterBool*      show;

  struct {
    sk::ParameterBool*    pump;
    sk::ParameterFloat*   pump_interval;
  } update_method;

  struct {
    sk::ParameterInt*     red;
    sk::ParameterInt*     green;
    sk::ParameterInt*     blue;
  } text;

  struct {
    sk::ParameterFloat*   scale;
    sk::ParameterInt*     pos_x;
    sk::ParameterInt*     pos_y;
  } viewport;
} osd;

struct {
  struct {
    sk::ParameterStringW*   sound_file;
    sk::ParameterBool*      play_sound;
    sk::ParameterBool*      take_screenshot;

    struct {
      sk::ParameterBool*    show;
      sk::ParameterBool*    show_title;
      sk::ParameterBool*    animate;
      sk::ParameterStringW* origin;
      sk::ParameterFloat*   inset;
      sk::ParameterInt*     duration;
    } popup;
  } achievements;

  struct {
    sk::ParameterInt*     appid;
    sk::ParameterInt*     init_delay;
    sk::ParameterBool*    auto_pump;
    sk::ParameterStringW* notify_corner;
  } system;

  struct {
    sk::ParameterBool*    silent;
  } log;
} steam;

struct {
  struct {
    sk::ParameterBool*    override;
    sk::ParameterStringW* compatibility;
    sk::ParameterStringW* num_gpus;
    sk::ParameterStringW* mode;
  } sli;

  struct {
    sk::ParameterBool*    disable;
  } api;
} nvidia;

sk::ParameterFloat*       mem_reserve;
sk::ParameterBool*        debug_output;
sk::ParameterBool*        game_output;
sk::ParameterBool*        handle_crashes;
sk::ParameterBool*        prefer_fahrenheit;
sk::ParameterBool*        ignore_rtss_delay;
sk::ParameterInt*         init_delay;
sk::ParameterBool*        silent;
sk::ParameterStringW*     version;

struct {
  struct {
    sk::ParameterFloat*   target_fps;
    sk::ParameterInt*     prerender_limit;
    sk::ParameterInt*     present_interval;
    sk::ParameterInt*     buffer_count;
    sk::ParameterInt*     max_delta_time;
    sk::ParameterBool*    flip_discard;
  } framerate;
  struct {
    sk::ParameterInt*     adapter_override;
    sk::ParameterStringW* max_res;
    sk::ParameterStringW* min_res;
    sk::ParameterInt*     swapchain_wait;
    sk::ParameterStringW* scaling_mode;
  } dxgi;
  struct {
    sk::ParameterBool*    force_d3d9ex;
    sk::ParameterBool*    force_fullscreen;
    sk::ParameterInt*     hook_type;
    sk::ParameterInt*     refresh_rate;
  } d3d9;
} render;

struct {
  struct {
    sk::ParameterBool*    precise_hash;
    sk::ParameterBool*    dump;
    sk::ParameterBool*    inject;
    sk::ParameterBool*    cache;
    sk::ParameterStringW* res_root;
  } d3d11;
  struct {
    sk::ParameterInt*     min_evict;
    sk::ParameterInt*     max_evict;
    sk::ParameterInt*     min_size;
    sk::ParameterInt*     max_size;
    sk::ParameterInt*     min_entries;
    sk::ParameterInt*     max_entries;
    sk::ParameterBool*    ignore_non_mipped;
  } cache;
} texture;

struct {
  struct {
    sk::ParameterBool*    manage;
    sk::ParameterBool*    keys_activate;
    sk::ParameterFloat*   timeout;
  } cursor;
} input;

struct {
  sk::ParameterBool*      borderless;
  sk::ParameterBool*      center;
  sk::ParameterInt*       x_off;
  sk::ParameterInt*       y_off;
  sk::ParameterBool*      background_render;
  sk::ParameterBool*      background_mute;
  sk::ParameterBool*      confine_cursor;
  sk::ParameterBool*      fullscreen;
  sk::ParameterStringW*   override;
} window;

struct {
  sk::ParameterBool*      ignore_raptr;
  sk::ParameterBool*      disable_raptr;
  sk::ParameterBool*      rehook_loadlibrary;

  struct {
    sk::ParameterBool*    rehook_reset;
    sk::ParameterBool*    rehook_present;
    sk::ParameterBool*    hook_reset_vtable;
    sk::ParameterBool*    hook_present_vtable;
  } d3d9;
} compatibility;


extern const wchar_t*
SK_Steam_PopupOriginToWStr (int origin);
extern int
SK_Steam_PopupOriginWStrToEnum (const wchar_t* str);

bool
SK_LoadConfig (std::wstring name) {
  // Load INI File
  std::wstring full_name;
  std::wstring osd_config;
  std::wstring achievement_config;

  full_name = SK_GetConfigPath () +
                name              +
                  L".ini";

  SK_CreateDirectories (full_name.c_str ());

  dll_ini =
    new iSK_INI (full_name.c_str ());

  bool empty = dll_ini->get_sections ().empty ();

  osd_config =
    SK_EvalEnvironmentVars (
      L"%USERPROFILE%\\Documents\\My Mods\\SpecialK\\Global\\osd.ini"
    );

  achievement_config =
    SK_EvalEnvironmentVars (
      L"%USERPROFILE%\\Documents\\My Mods\\SpecialK\\Global\\achievements.ini"
    );

  SK_CreateDirectories (osd_config.c_str ());

  osd_ini =
    new iSK_INI (osd_config.c_str ());

  achievement_ini =
    new iSK_INI (achievement_config.c_str ());

  //
  // Create Parameters
  //
  monitoring.io.show =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (L"Show IO Monitoring"));
  monitoring.io.show->register_to_ini (osd_ini, L"Monitor.IO", L"Show");

  monitoring.io.interval =
    static_cast <sk::ParameterFloat *>
     (g_ParameterFactory.create_parameter <float> (L"IO Monitoring Interval"));
  monitoring.io.interval->register_to_ini(osd_ini, L"Monitor.IO", L"Interval");

  monitoring.disk.show =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (L"Show Disk Monitoring"));
  monitoring.disk.show->register_to_ini(osd_ini, L"Monitor.Disk", L"Show");

  monitoring.disk.interval =
    static_cast <sk::ParameterFloat *>
     (g_ParameterFactory.create_parameter <float> (
       L"Disk Monitoring Interval")
     );
  monitoring.disk.interval->register_to_ini (
    osd_ini,
      L"Monitor.Disk",
        L"Interval" );

  monitoring.disk.type =
    static_cast <sk::ParameterInt *>
     (g_ParameterFactory.create_parameter <int> (
       L"Disk Monitoring Type (0 = Physical, 1 = Logical)")
     );
  monitoring.disk.type->register_to_ini (
    osd_ini,
      L"Monitor.Disk",
        L"Type" );


  monitoring.cpu.show =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (L"Show CPU Monitoring"));
  monitoring.cpu.show->register_to_ini (osd_ini, L"Monitor.CPU", L"Show");

  monitoring.cpu.interval =
    static_cast <sk::ParameterFloat *>
     (g_ParameterFactory.create_parameter <float> (
       L"CPU Monitoring Interval (seconds)")
     );
  monitoring.cpu.interval->register_to_ini (
    osd_ini,
      L"Monitor.CPU",
        L"Interval" );

  monitoring.cpu.simple =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (L"Minimal CPU info"));
  monitoring.cpu.simple->register_to_ini (osd_ini, L"Monitor.CPU", L"Simple");

  monitoring.gpu.show =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (L"Show GPU Monitoring"));
  monitoring.gpu.show->register_to_ini (osd_ini, L"Monitor.GPU", L"Show");

  monitoring.gpu.print_slowdown =
    static_cast <sk::ParameterBool *>
    (g_ParameterFactory.create_parameter <bool>(L"Print GPU Slowdown Reason"));
  monitoring.gpu.print_slowdown->register_to_ini (
    osd_ini,
      L"Monitor.GPU",
        L"PrintSlowdown" );

  monitoring.gpu.interval =
    static_cast <sk::ParameterFloat *>
     (g_ParameterFactory.create_parameter <float> (
       L"GPU Monitoring Interval (seconds)")
     );
  monitoring.gpu.interval->register_to_ini (
    osd_ini,
      L"Monitor.GPU",
        L"Interval" );


  monitoring.pagefile.show =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Show Pagefile Monitoring")
      );
  monitoring.pagefile.show->register_to_ini (
    osd_ini,
      L"Monitor.Pagefile",
        L"Show" );

  monitoring.pagefile.interval =
    static_cast <sk::ParameterFloat *>
     (g_ParameterFactory.create_parameter <float> (
       L"Pagefile Monitoring Interval (seconds)")
     );
  monitoring.pagefile.interval->register_to_ini (
    osd_ini,
      L"Monitor.Pagefile",
        L"Interval" );


  monitoring.memory.show =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Show Memory Monitoring")
      );
  monitoring.memory.show->register_to_ini (
    osd_ini,
      L"Monitor.Memory",
        L"Show" );


  monitoring.fps.show =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Show Framerate Monitoring")
      );
  monitoring.fps.show->register_to_ini (
    osd_ini,
      L"Monitor.FPS",
        L"Show" );


  monitoring.time.show =
    static_cast <sk::ParameterBool *>
    (g_ParameterFactory.create_parameter <bool> (
      L"Show Time")
    );
  monitoring.time.show->register_to_ini (
    osd_ini,
      L"Monitor.Time",
        L"Show" );


  input.cursor.manage =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Manage Cursor Visibility")
      );
  input.cursor.manage->register_to_ini (
    dll_ini,
      L"Input.Cursor",
        L"Manage" );

  input.cursor.keys_activate =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Keyboard Input Activates Cursor")
      );
  input.cursor.keys_activate->register_to_ini (
    dll_ini,
      L"Input.Cursor",
        L"KeyboardActivates" );

  input.cursor.timeout =
    static_cast <sk::ParameterFloat *>
      (g_ParameterFactory.create_parameter <float> (
        L"Hide Delay")
      );
  input.cursor.timeout->register_to_ini (
    dll_ini,
      L"Input.Cursor",
        L"Timeout" );


  window.borderless =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Borderless Window Mode")
      );
  window.borderless->register_to_ini (
    dll_ini,
      L"Window.System",
        L"Borderless" );

  window.center =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Center the Window")
      );
  window.center->register_to_ini (
    dll_ini,
      L"Window.System",
        L"Center" );

  window.background_render =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Render While Window is in Background")
      );
  window.background_render->register_to_ini (
    dll_ini,
      L"Window.System",
        L"RenderInBackground" );

  window.background_mute =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Mute While Window is in Background")
      );
  window.background_mute->register_to_ini (
    dll_ini,
      L"Window.System",
        L"MuteInBackground" );

  window.x_off =
    static_cast <sk::ParameterInt *>
      (g_ParameterFactory.create_parameter <int> (
        L"X Offset")
      );
  window.x_off->register_to_ini (
    dll_ini,
      L"Window.System",
        L"XOffset" );

  window.y_off =
    static_cast <sk::ParameterInt *>
      (g_ParameterFactory.create_parameter <int> (
        L"Y Offset")
      );
  window.y_off->register_to_ini (
    dll_ini,
      L"Window.System",
        L"YOffset" );

  window.confine_cursor =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Confine the Mouse Cursor to the Game Window.")
      );
  window.confine_cursor->register_to_ini (
    dll_ini,
      L"Window.System",
        L"ConfineCursor" );

  window.fullscreen =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Make the Game Window Fill the Screen (scale to fit)")
      );
  window.fullscreen->register_to_ini (
    dll_ini,
      L"Window.System",
        L"Fullscreen" );

  window.override =
    static_cast <sk::ParameterStringW *>
      (g_ParameterFactory.create_parameter <std::wstring> (
        L"Force the Client Region to this Size in Windowed Mode")
      );
  window.override->register_to_ini (
    dll_ini,
      L"Window.System",
        L"OverrideRes" );



  compatibility.ignore_raptr =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Ignore Raptr Warning")
      );
  compatibility.ignore_raptr->register_to_ini (
    dll_ini,
      L"Compatibility.General",
        L"IgnoreRaptr" );

  compatibility.disable_raptr =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Forcefully Disable Raptr")
      );
  compatibility.disable_raptr->register_to_ini (
    dll_ini,
      L"Compatibility.General",
        L"DisableRaptr" );

  compatibility.rehook_loadlibrary =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Rehook LoadLibrary When RTSS/Steam/GeDoSaTo hook it")
      );
  compatibility.rehook_loadlibrary->register_to_ini (
    dll_ini,
      L"Compatibility.General",
        L"RehookLoadLibrary" );


  mem_reserve =
    static_cast <sk::ParameterFloat *>
      (g_ParameterFactory.create_parameter <float> (
        L"Memory Reserve Percentage")
      );
  mem_reserve->register_to_ini (
    dll_ini,
      L"Manage.Memory",
        L"ReservePercent" );


  init_delay =
    static_cast <sk::ParameterInt *>
      (g_ParameterFactory.create_parameter <int> (
        L"Initialization Delay (msecs)")
      );
  init_delay->register_to_ini (
    dll_ini,
      L"SpecialK.System",
        L"InitDelay" );

  silent =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Log Silence")
      );
  silent->register_to_ini (
    dll_ini,
      L"SpecialK.System",
        L"Silent" );

  prefer_fahrenheit =
    static_cast <sk::ParameterBool *>
    (g_ParameterFactory.create_parameter <bool> (
      L"Prefer Fahrenheit Units")
      );
  prefer_fahrenheit->register_to_ini (
    osd_ini,
      L"SpecialK.OSD",
        L"PreferFahrenheit" );

  handle_crashes =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Use Custom Crash Handler")
      );
  handle_crashes->register_to_ini (
    dll_ini,
      L"SpecialK.System",
        L"UseCrashHandler" );

  debug_output =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Print Application's Debug Output in real-time")
      );
  debug_output->register_to_ini (
    dll_ini,
      L"SpecialK.System",
        L"DebugOutput" );

  game_output =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Log Application's Debug Output")
      );
  game_output->register_to_ini (
    dll_ini,
      L"SpecialK.System",
        L"GameOutput" );


  ignore_rtss_delay =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Ignore RTSS Delay Incompatibilities")
      );
  ignore_rtss_delay->register_to_ini (
    dll_ini,
      L"SpecialK.System",
        L"IgnoreRTSSHookDelay" );

  version =
    static_cast <sk::ParameterStringW *>
      (g_ParameterFactory.create_parameter <std::wstring> (
        L"Software Version")
      );
  version->register_to_ini (
    dll_ini,
      L"SpecialK.System",
        L"Version" );


  render.framerate.target_fps =
    static_cast <sk::ParameterFloat *>
      (g_ParameterFactory.create_parameter <float> (
        L"Framerate Target")
      );
  render.framerate.target_fps->register_to_ini (
    dll_ini,
      L"Render.FrameRate",
        L"TargetFPS" );

  render.framerate.buffer_count =
    static_cast <sk::ParameterInt *>
      (g_ParameterFactory.create_parameter <int> (
        L"Number of BackBuffers in the SwapChain")
      );
  render.framerate.buffer_count->register_to_ini (
    dll_ini,
      L"Render.FrameRate",
        L"BackBufferCount" );

  render.framerate.present_interval =
    static_cast <sk::ParameterInt *>
      (g_ParameterFactory.create_parameter <int> (
        L"Presentation Interval")
      );
  render.framerate.present_interval->register_to_ini (
    dll_ini,
      L"Render.FrameRate",
        L"PresentationInterval" );

  render.framerate.prerender_limit =
    static_cast <sk::ParameterInt *>
      (g_ParameterFactory.create_parameter <int> (
        L"Maximum Frames to Render-Ahead")
      );
  render.framerate.prerender_limit->register_to_ini (
    dll_ini,
      L"Render.FrameRate",
        L"PreRenderLimit" );


  if (SK_IsInjected () || (SK_GetDLLRole () & DLL_ROLE::D3D9)) {
    compatibility.d3d9.rehook_present =
      static_cast <sk::ParameterBool *>
        (g_ParameterFactory.create_parameter <bool> (
          L"Rehook D3D9 Present On Device Reset")
        );
    compatibility.d3d9.rehook_present->register_to_ini (
      dll_ini,
        L"Compatibility.D3D9",
          L"RehookPresent" );
    compatibility.d3d9.rehook_reset =
      static_cast <sk::ParameterBool *>
        (g_ParameterFactory.create_parameter <bool> (
          L"Rehook D3D9 Reset On Device Reset")
        );
    compatibility.d3d9.rehook_reset->register_to_ini (
      dll_ini,
        L"Compatibility.D3D9",
          L"RehookReset" );

    compatibility.d3d9.hook_present_vtable =
      static_cast <sk::ParameterBool *>
        (g_ParameterFactory.create_parameter <bool> (
          L"Use VFtable Override for Present")
        );
    compatibility.d3d9.hook_present_vtable->register_to_ini (
      dll_ini,
        L"Compatibility.D3D9",
          L"UseVFTableForPresent" );
    compatibility.d3d9.hook_reset_vtable =
      static_cast <sk::ParameterBool *>
        (g_ParameterFactory.create_parameter <bool> (
          L"Use VFtable Override for Reset")
        );
    compatibility.d3d9.hook_reset_vtable->register_to_ini (
      dll_ini,
        L"Compatibility.D3D9",
          L"UseVFTableForReset" );

    render.d3d9.force_d3d9ex =
      static_cast <sk::ParameterBool *>
        (g_ParameterFactory.create_parameter <bool> (
          L"Force D3D9Ex Context")
        );
    render.d3d9.force_d3d9ex->register_to_ini (
      dll_ini,
        L"Render.D3D9",
          L"ForceD3D9Ex" );
    render.d3d9.hook_type =
      static_cast <sk::ParameterInt *>
        (g_ParameterFactory.create_parameter <int> (
          L"Hook Technique")
        );
    render.d3d9.hook_type->register_to_ini (
      dll_ini,
        L"Render.D3D9",
          L"HookType" );
    render.d3d9.force_fullscreen =
      static_cast <sk::ParameterBool *>
        (g_ParameterFactory.create_parameter <bool> (
          L"Force Fullscreen Mode")
        );
    render.d3d9.force_fullscreen->register_to_ini (
      dll_ini,
        L"Render.D3D9",
          L"ForceFullscreen" );
    render.d3d9.refresh_rate =
      static_cast <sk::ParameterInt *>
        (g_ParameterFactory.create_parameter <int> (
          L"Fullscreen Refresh Rate")
        );
    render.d3d9.refresh_rate->register_to_ini (
      dll_ini,
        L"Render.D3D9",
          L"RefreshRate" );
  }

  if (SK_IsInjected () || (SK_GetDLLRole () & (DLL_ROLE::DXGI))) {
    render.framerate.max_delta_time =
      static_cast <sk::ParameterInt *>
        (g_ParameterFactory.create_parameter <int> (
          L"Maximum Frame Delta Time")
        );
    render.framerate.max_delta_time->register_to_ini (
      dll_ini,
        L"Render.DXGI",
          L"MaxDeltaTime" );

    render.framerate.flip_discard =
      static_cast <sk::ParameterBool *>
        (g_ParameterFactory.create_parameter <bool> (
          L"Use Flip Discard - Windows 10+")
        );
    render.framerate.flip_discard->register_to_ini (
      dll_ini,
        L"Render.DXGI",
          L"UseFlipDiscard" );

    render.dxgi.adapter_override =
      static_cast <sk::ParameterInt *>
        (g_ParameterFactory.create_parameter <int> (
          L"Override DXGI Adapter")
        );
    render.dxgi.adapter_override->register_to_ini (
      dll_ini,
        L"Render.DXGI",
          L"AdapterOverride" );

    render.dxgi.max_res =
      static_cast <sk::ParameterStringW *>
        (g_ParameterFactory.create_parameter <std::wstring> (
          L"Maximum Resolution To Report")
        );
    render.dxgi.max_res->register_to_ini (
      dll_ini,
        L"Render.DXGI",
          L"MaxRes" );

    render.dxgi.min_res =
      static_cast <sk::ParameterStringW *>
        (g_ParameterFactory.create_parameter <std::wstring> (
          L"Minimum Resolution To Report")
        );
    render.dxgi.min_res->register_to_ini (
      dll_ini,
        L"Render.DXGI",
          L"MinRes" );

    render.dxgi.swapchain_wait =
      static_cast <sk::ParameterInt *>
        (g_ParameterFactory.create_parameter <int> (
          L"Time to wait in msec. for SwapChain")
        );
    render.dxgi.swapchain_wait->register_to_ini (
      dll_ini,
        L"Render.DXGI",
          L"SwapChainWait" );

    render.dxgi.scaling_mode =
      static_cast <sk::ParameterStringW *>
        (g_ParameterFactory.create_parameter <std::wstring> (
          L"Scaling Preference (DontCare | Centered | Stretched)")
        );
    render.dxgi.scaling_mode->register_to_ini (
      dll_ini,
        L"Render.DXGI",
          L"Scaling" );


    texture.d3d11.cache =
      static_cast <sk::ParameterBool *>
        (g_ParameterFactory.create_parameter <bool> (
          L"Cache Textures")
        );
    texture.d3d11.cache->register_to_ini (
      dll_ini,
        L"Textures.D3D11",
          L"Cache" );

    texture.d3d11.precise_hash =
      static_cast <sk::ParameterBool *>
        (g_ParameterFactory.create_parameter <bool> (
          L"Precise Hash Generation")
        );
    texture.d3d11.precise_hash->register_to_ini (
      dll_ini,
        L"Textures.D3D11",
          L"PreciseHash" );

    texture.d3d11.dump =
      static_cast <sk::ParameterBool *>
        (g_ParameterFactory.create_parameter <bool> (
          L"Dump Textures")
        );
    texture.d3d11.dump->register_to_ini (
      dll_ini,
        L"Textures.D3D11",
          L"Dump" );

    texture.d3d11.inject =
      static_cast <sk::ParameterBool *>
        (g_ParameterFactory.create_parameter <bool> (
          L"Inject Textures")
        );
    texture.d3d11.inject->register_to_ini (
      dll_ini,
        L"Textures.D3D11",
          L"Inject" );

    texture.d3d11.res_root =
      static_cast <sk::ParameterStringW *>
        (g_ParameterFactory.create_parameter <std::wstring> (
          L"Resource Root")
        );
    texture.d3d11.res_root->register_to_ini (
      dll_ini,
        L"Textures.D3D11",
          L"ResourceRoot" );

    texture.cache.min_entries =
      static_cast <sk::ParameterInt *>
        (g_ParameterFactory.create_parameter <int> (
          L"Minimum Cached Textures")
        );
    texture.cache.min_entries->register_to_ini (
      dll_ini,
        L"Textures.Cache",
          L"MinEntries" );

    texture.cache.max_entries =
      static_cast <sk::ParameterInt *>
        (g_ParameterFactory.create_parameter <int> (
          L"Maximum Cached Textures")
        );
    texture.cache.max_entries->register_to_ini (
      dll_ini,
        L"Textures.Cache",
          L"MaxEntries" );

    texture.cache.min_evict =
      static_cast <sk::ParameterInt *>
        (g_ParameterFactory.create_parameter <int> (
          L"Minimum Textures to Evict")
        );
    texture.cache.min_evict->register_to_ini (
      dll_ini,
        L"Textures.Cache",
          L"MinEvict" );

    texture.cache.max_evict =
      static_cast <sk::ParameterInt *>
        (g_ParameterFactory.create_parameter <int> (
          L"Maximum Textures to Evict")
        );
    texture.cache.max_evict->register_to_ini (
      dll_ini,
        L"Textures.Cache",
          L"MaxEvict" );

    texture.cache.min_size =
      static_cast <sk::ParameterInt *>
        (g_ParameterFactory.create_parameter <int> (
          L"Minimum Textures to Evict")
        );
    texture.cache.min_size->register_to_ini (
      dll_ini,
        L"Textures.Cache",
          L"MinSizeInMiB" );

    texture.cache.max_size =
      static_cast <sk::ParameterInt *>
        (g_ParameterFactory.create_parameter <int> (
          L"Maximum Textures to Evict")
        );
    texture.cache.max_size->register_to_ini (
      dll_ini,
        L"Textures.Cache",
          L"MaxSizeInMiB" );

    texture.cache.ignore_non_mipped =
      static_cast <sk::ParameterBool *>
        (g_ParameterFactory.create_parameter <bool> (
          L"Ignore textures without mipmaps?")
        );
    texture.cache.ignore_non_mipped->register_to_ini (
      dll_ini,
        L"Textures.Cache",
          L"IgnoreNonMipmapped" );
  }


  nvidia.api.disable =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Disable NvAPI")
      );
  nvidia.api.disable->register_to_ini (
    dll_ini,
      L"NVIDIA.API",
        L"Disable" );


  nvidia.sli.compatibility =
    static_cast <sk::ParameterStringW *>
      (g_ParameterFactory.create_parameter <std::wstring> (
        L"SLI Compatibility Bits")
      );
  nvidia.sli.compatibility->register_to_ini (
    dll_ini,
      L"NVIDIA.SLI",
        L"CompatibilityBits" );

  nvidia.sli.num_gpus =
    static_cast <sk::ParameterStringW *>
      (g_ParameterFactory.create_parameter <std::wstring> (
        L"SLI GPU Count")
      );
  nvidia.sli.num_gpus->register_to_ini (
    dll_ini,
      L"NVIDIA.SLI",
        L"NumberOfGPUs" );

  nvidia.sli.mode =
    static_cast <sk::ParameterStringW *>
      (g_ParameterFactory.create_parameter <std::wstring> (
        L"SLI Mode")
      );
  nvidia.sli.mode->register_to_ini (
    dll_ini,
      L"NVIDIA.SLI",
        L"Mode" );

  nvidia.sli.override =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Override Driver Defaults")
      );
  nvidia.sli.override->register_to_ini (
    dll_ini,
      L"NVIDIA.SLI",
        L"Override" );


  osd.show =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"OSD Visibility")
      );
  osd.show->register_to_ini (
    osd_ini,
      L"SpecialK.OSD",
        L"Show" );

  osd.update_method.pump =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Refresh the OSD irrespective of frame completion")
      );
  osd.update_method.pump->register_to_ini (
    osd_ini,
      L"SpecialK.OSD",
        L"AutoPump" );

  osd.update_method.pump_interval =
    static_cast <sk::ParameterFloat *>
    (g_ParameterFactory.create_parameter <float> (
      L"Time in seconds between OSD updates")
    );
  osd.update_method.pump_interval->register_to_ini (
    osd_ini,
      L"SpecialK.OSD",
        L"PumpInterval" );

  osd.text.red =
    static_cast <sk::ParameterInt *>
      (g_ParameterFactory.create_parameter <int> (
        L"OSD Color (Red)")
      );
  osd.text.red->register_to_ini (
    osd_ini,
      L"SpecialK.OSD",
        L"TextColorRed" );

  osd.text.green =
    static_cast <sk::ParameterInt *>
      (g_ParameterFactory.create_parameter <int> (
        L"OSD Color (Green)")
      );
  osd.text.green->register_to_ini (
    osd_ini,
      L"SpecialK.OSD",
        L"TextColorGreen" );

  osd.text.blue =
    static_cast <sk::ParameterInt *>
      (g_ParameterFactory.create_parameter <int> (
        L"OSD Color (Blue)")
      );
  osd.text.blue->register_to_ini (
    osd_ini,
      L"SpecialK.OSD",
        L"TextColorBlue" );

  osd.viewport.pos_x =
    static_cast <sk::ParameterInt *>
      (g_ParameterFactory.create_parameter <int> (
        L"OSD Position (X)")
      );
  osd.viewport.pos_x->register_to_ini (
    osd_ini,
      L"SpecialK.OSD",
        L"PositionX" );

  osd.viewport.pos_y =
    static_cast <sk::ParameterInt *>
      (g_ParameterFactory.create_parameter <int> (
        L"OSD Position (Y)")
      );
  osd.viewport.pos_y->register_to_ini (
    osd_ini,
      L"SpecialK.OSD",
        L"PositionY" );

  osd.viewport.scale =
    static_cast <sk::ParameterFloat *>
      (g_ParameterFactory.create_parameter <float> (
        L"OSD Scale")
      );
  osd.viewport.scale->register_to_ini (
    osd_ini,
      L"SpecialK.OSD",
        L"Scale" );


  monitoring.SLI.show =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Show SLI Monitoring")
      );
  monitoring.SLI.show->register_to_ini (
    osd_ini,
      L"Monitor.SLI",
        L"Show" );


  steam.achievements.sound_file =
    static_cast <sk::ParameterStringW *>
      (g_ParameterFactory.create_parameter <std::wstring> (
        L"Achievement Sound File")
      );
  steam.achievements.sound_file->register_to_ini (
    dll_ini,
      L"Steam.Achievements",
        L"SoundFile" );

  steam.achievements.play_sound =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Silence is Bliss?")
      );
  steam.achievements.play_sound->register_to_ini(
    achievement_ini,
      L"Steam.Achievements",
        L"PlaySound" );

  steam.achievements.take_screenshot =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Precious Memories")
      );
  steam.achievements.take_screenshot->register_to_ini(
    achievement_ini,
      L"Steam.Achievements",
        L"TakeScreenshot" );

  steam.system.notify_corner =
    static_cast <sk::ParameterStringW *>
      (g_ParameterFactory.create_parameter <std::wstring> (
        L"Overlay Notification Position")
      );
  steam.system.notify_corner->register_to_ini (
    achievement_ini,
      L"Steam.System",
        L"NotifyCorner" );

  steam.achievements.popup.origin =
    static_cast <sk::ParameterStringW *>
      (g_ParameterFactory.create_parameter <std::wstring> (
        L"Achievement Popup Position")
      );
  steam.achievements.popup.origin->register_to_ini (
    achievement_ini,
      L"Steam.Achievements",
        L"PopupOrigin" );

  steam.achievements.popup.animate =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Achievement Notification Animation")
      );
  steam.achievements.popup.animate->register_to_ini (
    achievement_ini,
      L"Steam.Achievements",
        L"AnimatePopup" );

  steam.achievements.popup.show_title =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Achievement Popup Includes Game Title?")
      );
  steam.achievements.popup.show_title->register_to_ini (
    achievement_ini,
      L"Steam.Achievements",
        L"ShowPopupTitle" );

  steam.achievements.popup.inset =
    static_cast <sk::ParameterFloat *>
    (g_ParameterFactory.create_parameter <float> (
      L"Achievement Notification Inset X")
    );
  steam.achievements.popup.inset->register_to_ini (
    achievement_ini,
      L"Steam.Achievements",
        L"PopupInset" );

  steam.achievements.popup.duration =
    static_cast <sk::ParameterInt *>
    (g_ParameterFactory.create_parameter <int> (
      L"Achievement Popup Duration (in ms)")
    );
  steam.achievements.popup.duration->register_to_ini (
    achievement_ini,
      L"Steam.Achievements",
        L"PopupDuration" );

  steam.system.appid = 
    static_cast <sk::ParameterInt *>
    (g_ParameterFactory.create_parameter <int> (
      L"Steam AppID")
    );
  steam.system.appid->register_to_ini (
    dll_ini,
      L"Steam.System",
        L"AppID" );

  steam.system.init_delay =
    static_cast <sk::ParameterInt *>
    (g_ParameterFactory.create_parameter <int> (
      L"How long to delay SteamAPI initialization if the game doesn't do it")
    );
  steam.system.init_delay->register_to_ini (
    dll_ini,
      L"Steam.System",
        L"AutoInitDelay" );

  steam.system.auto_pump =
    static_cast <sk::ParameterBool *>
    (g_ParameterFactory.create_parameter <bool> (
      L"Should we force the game to run Steam callbacks?")
    );
  steam.system.auto_pump->register_to_ini (
    dll_ini,
      L"Steam.System",
        L"AutoPumpCallbacks" );

  steam.log.silent =
    static_cast <sk::ParameterBool *>
      (g_ParameterFactory.create_parameter <bool> (
        L"Makes steam_api.log go away")
      );
  steam.log.silent->register_to_ini(
    dll_ini,
      L"Steam.Log",
        L"Silent" );

  iSK_INI::_TSectionMap& sections =
    dll_ini->get_sections ();

  iSK_INI::_TSectionMap::const_iterator sec =
    sections.begin ();

  int import = 0;

  while (sec != sections.end ()) {
    if (wcsstr ((*sec).first.c_str (), L"Import.")) {
      imports [import].filename = 
         static_cast <sk::ParameterStringW *>
             (g_ParameterFactory.create_parameter <std::wstring> (
                L"Import Filename")
             );
      imports [import].filename->register_to_ini (
        dll_ini,
          (*sec).first.c_str (),
            L"Filename" );

      imports [import].when = 
         static_cast <sk::ParameterStringW *>
             (g_ParameterFactory.create_parameter <std::wstring> (
                L"Import Timeframe")
             );
      imports [import].when->register_to_ini (
        dll_ini,
          (*sec).first.c_str (),
            L"When" );

      imports [import].role = 
         static_cast <sk::ParameterStringW *>
             (g_ParameterFactory.create_parameter <std::wstring> (
                L"Import Role")
             );
      imports [import].role->register_to_ini (
        dll_ini,
          (*sec).first.c_str (),
            L"Role" );

      imports [import].architecture = 
         static_cast <sk::ParameterStringW *>
             (g_ParameterFactory.create_parameter <std::wstring> (
                L"Import Architecture")
             );
      imports [import].architecture->register_to_ini (
        dll_ini,
          (*sec).first.c_str (),
            L"Architecture" );

      imports [import].blacklist = 
         static_cast <sk::ParameterStringW *>
             (g_ParameterFactory.create_parameter <std::wstring> (
                L"Blakclisted Executables")
             );
      imports [import].blacklist->register_to_ini (
        dll_ini,
          (*sec).first.c_str (),
            L"Blacklist" );

      imports [import].filename->load     ();
      imports [import].when->load         ();
      imports [import].role->load         ();
      imports [import].architecture->load ();
      imports [import].blacklist->load    ();

      imports [import].hLibrary = NULL;

      ++import;

      if (import > SK_MAX_IMPORTS)
        break;
    }

    ++sec;
  }

  //
  // Load Parameters
  //
  if (compatibility.ignore_raptr->load ())
    config.compatibility.ignore_raptr = compatibility.ignore_raptr->get_value ();
  if (compatibility.disable_raptr->load ())
    config.compatibility.disable_raptr = compatibility.disable_raptr->get_value ();
  if (compatibility.rehook_loadlibrary->load ())
    config.compatibility.rehook_loadlibrary = compatibility.rehook_loadlibrary->get_value ();


  if (monitoring.io.show->load ())
    config.io.show = monitoring.io.show->get_value ();
  if (monitoring.io.interval->load ())
    config.io.interval = monitoring.io.interval->get_value ();

  if (monitoring.fps.show->load ())
    config.fps.show = monitoring.fps.show->get_value ();

  if (monitoring.memory.show->load ())
    config.mem.show = monitoring.memory.show->get_value ();
  if (mem_reserve->load ())
    config.mem.reserve = mem_reserve->get_value ();

  if (monitoring.cpu.show->load ())
    config.cpu.show = monitoring.cpu.show->get_value ();
  if (monitoring.cpu.interval->load ())
    config.cpu.interval = monitoring.cpu.interval->get_value ();
  if (monitoring.cpu.simple->load ())
    config.cpu.simple = monitoring.cpu.simple->get_value ();

  if (monitoring.gpu.show->load ())
    config.gpu.show = monitoring.gpu.show->get_value ();
  if (monitoring.gpu.print_slowdown->load ())
    config.gpu.print_slowdown = monitoring.gpu.print_slowdown->get_value ();
  if (monitoring.gpu.interval->load ())
    config.gpu.interval = monitoring.gpu.interval->get_value ();

  if (monitoring.disk.show->load ())
    config.disk.show = monitoring.disk.show->get_value ();
  if (monitoring.disk.interval->load ())
    config.disk.interval = monitoring.disk.interval->get_value ();
  if (monitoring.disk.type->load ())
    config.disk.type = monitoring.disk.type->get_value ();

  if (monitoring.pagefile.show->load ())
    config.pagefile.show = monitoring.pagefile.show->get_value ();
  if (monitoring.pagefile.interval->load ())
    config.pagefile.interval = monitoring.pagefile.interval->get_value ();

  if (monitoring.time.show->load ())
    config.time.show = monitoring.time.show->get_value ();

  if (monitoring.SLI.show->load ())
    config.sli.show = monitoring.SLI.show->get_value ();

  if (nvidia.api.disable->load ())
    config.nvidia.api.disable = nvidia.api.disable->get_value ();

  if ( SK_IsInjected () ||
         ( SK_GetDLLRole () & DLL_ROLE::D3D9 ||
           SK_GetDLLRole () & DLL_ROLE::DXGI ) ) {
    // SLI only works in Direct3D
    if (nvidia.sli.compatibility->load ())
      config.nvidia.sli.compatibility =
        nvidia.sli.compatibility->get_value ();
    if (nvidia.sli.mode->load ())
      config.nvidia.sli.mode =
        nvidia.sli.mode->get_value ();
    if (nvidia.sli.num_gpus->load ())
      config.nvidia.sli.num_gpus =
        nvidia.sli.num_gpus->get_value ();
    if (nvidia.sli.override->load ())
      config.nvidia.sli.override =
        nvidia.sli.override->get_value ();

    if (render.framerate.target_fps->load ())
      config.render.framerate.target_fps =
        render.framerate.target_fps->get_value ();
    if (render.framerate.buffer_count->load ())
      config.render.framerate.buffer_count =
        render.framerate.buffer_count->get_value ();
    if (render.framerate.prerender_limit->load ())
      config.render.framerate.pre_render_limit =
        render.framerate.prerender_limit->get_value ();
    if (render.framerate.present_interval->load ())
      config.render.framerate.present_interval =
        render.framerate.present_interval->get_value ();

    if (SK_IsInjected () || SK_GetDLLRole () & DLL_ROLE::D3D9) {
      if (compatibility.d3d9.rehook_present->load ())
        config.compatibility.d3d9.rehook_present =
          compatibility.d3d9.rehook_present->get_value ();
      if (compatibility.d3d9.rehook_reset->load ())
        config.compatibility.d3d9.rehook_reset =
          compatibility.d3d9.rehook_reset->get_value ();

      if (compatibility.d3d9.hook_present_vtable->load ())
        config.compatibility.d3d9.hook_present_vftbl =
          compatibility.d3d9.hook_present_vtable->get_value ();
      if (compatibility.d3d9.hook_reset_vtable->load ())
        config.compatibility.d3d9.hook_reset_vftbl =
          compatibility.d3d9.hook_reset_vtable->get_value ();

      if (render.d3d9.force_d3d9ex->load ())
        config.render.d3d9.force_d3d9ex =
          render.d3d9.force_d3d9ex->get_value ();
      if (render.d3d9.force_fullscreen->load ())
        config.render.d3d9.force_fullscreen =
          render.d3d9.force_fullscreen->get_value ();
      if (render.d3d9.hook_type->load ())
        config.render.d3d9.hook_type =
          render.d3d9.hook_type->get_value ();
      if (render.d3d9.refresh_rate->load ())
        config.render.d3d9.refresh_rate =
          render.d3d9.refresh_rate->get_value ();
    }

    if (SK_IsInjected () || SK_GetDLLRole () & DLL_ROLE::DXGI) {
      if (render.framerate.max_delta_time->load ())
        config.render.framerate.max_delta_time =
          render.framerate.max_delta_time->get_value ();
      if (render.framerate.flip_discard->load ()) {
        config.render.framerate.flip_discard =
          render.framerate.flip_discard->get_value ();

        extern bool SK_DXGI_use_factory1;
        if (config.render.framerate.flip_discard)
          SK_DXGI_use_factory1 = true;
      }

      if (render.dxgi.adapter_override->load ())
        config.render.dxgi.adapter_override =
          render.dxgi.adapter_override->get_value ();

      if (render.dxgi.max_res->load ()) {
        swscanf ( render.dxgi.max_res->get_value_str ().c_str (),
                    L"%lux%lu",
                    &config.render.dxgi.res.max.x,
                      &config.render.dxgi.res.max.y );
      }
      if (render.dxgi.min_res->load ()) {
        swscanf ( render.dxgi.min_res->get_value_str ().c_str (),
                    L"%lux%lu",
                    &config.render.dxgi.res.min.x,
                      &config.render.dxgi.res.min.y );
      }

      // Default = Don't Care
      config.render.dxgi.scaling_mode = -1;

      if (render.dxgi.scaling_mode->load ()) {
        if (! _wcsicmp (
                render.dxgi.scaling_mode->get_value_str ().c_str (),
                L"Unspecified"
              )
           )
        {
          config.render.dxgi.scaling_mode = DXGI_MODE_SCALING_UNSPECIFIED;
        }

        else if (! _wcsicmp (
                     render.dxgi.scaling_mode->get_value_str ().c_str (),
                     L"Centered"
                   )
                )
        {
          config.render.dxgi.scaling_mode = DXGI_MODE_SCALING_CENTERED;
        }

        else if (! _wcsicmp (
                     render.dxgi.scaling_mode->get_value_str ().c_str (),
                     L"Stretched"
                   )
                )
        {
          config.render.dxgi.scaling_mode = DXGI_MODE_SCALING_STRETCHED;
        }
      }

      if (render.dxgi.swapchain_wait->load ())
        config.render.framerate.swapchain_wait = render.dxgi.swapchain_wait->get_value ();

      if (texture.d3d11.cache->load ())
        config.textures.d3d11.cache = texture.d3d11.cache->get_value ();
      if (texture.d3d11.precise_hash->load ())
        config.textures.d3d11.precise_hash = texture.d3d11.precise_hash->get_value ();
      if (texture.d3d11.dump->load ())
        config.textures.d3d11.dump = texture.d3d11.dump->get_value ();
      if (texture.d3d11.inject->load ())
        config.textures.d3d11.inject = texture.d3d11.inject->get_value ();
      if (texture.d3d11.res_root->load ())
        config.textures.d3d11.res_root = texture.d3d11.res_root->get_value ();

      if (texture.cache.max_entries->load ())
        config.textures.cache.max_entries = texture.cache.max_entries->get_value ();
      if (texture.cache.min_entries->load ())
        config.textures.cache.min_entries = texture.cache.min_entries->get_value ();
      if (texture.cache.max_evict->load ())
        config.textures.cache.max_evict = texture.cache.max_evict->get_value ();
      if (texture.cache.min_evict->load ())
        config.textures.cache.min_evict = texture.cache.min_evict->get_value ();
      if (texture.cache.max_size->load ())
        config.textures.cache.max_size = texture.cache.max_size->get_value ();
      if (texture.cache.min_size->load ())
        config.textures.cache.min_size = texture.cache.min_size->get_value ();
      if (texture.cache.ignore_non_mipped->load ())
        config.textures.cache.ignore_nonmipped = texture.cache.ignore_non_mipped->get_value ();

      extern void WINAPI SK_DXGI_SetPreferredAdapter (int override_id);

      if (config.render.dxgi.adapter_override != -1)
        SK_DXGI_SetPreferredAdapter (config.render.dxgi.adapter_override);
    }
  }

  if (input.cursor.manage->load ())
    config.input.cursor.manage = input.cursor.manage->get_value ();
  if (input.cursor.keys_activate->load ())
    config.input.cursor.keys_activate = input.cursor.keys_activate->get_value ();
  if (input.cursor.timeout->load ())
    config.input.cursor.timeout = (int)(1000.0 * input.cursor.timeout->get_value ());

  if (window.borderless->load ())
    config.window.borderless = window.borderless->get_value ();
  if (window.center->load ())
    config.window.center = window.center->get_value ();
  if (window.background_render->load ())
    config.window.background_render = window.background_render->get_value ();
  if (window.background_mute->load ())
    config.window.background_mute = window.background_mute->get_value ();
  if (window.x_off->load ())
    config.window.x_offset = window.x_off->get_value ();
  if (window.y_off->load ())
    config.window.y_offset = window.y_off->get_value ();
  if (window.confine_cursor->load ())
    config.window.confine_cursor = window.confine_cursor->get_value ();
  if (window.fullscreen->load ())
    config.window.fullscreen = window.fullscreen->get_value ();
  if (window.override->load ()) {
    swscanf ( window.override->get_value_str ().c_str (),
                L"%lux%lu",
                &config.window.res.override.x,
                  &config.window.res.override.y );
  }

  if (steam.achievements.play_sound->load ())
    config.steam.achievements.play_sound =
    steam.achievements.play_sound->get_value ();
  if (steam.achievements.sound_file->load ())
    config.steam.achievements.sound_file =
      steam.achievements.sound_file->get_value ();
  if (steam.achievements.take_screenshot->load ())
    config.steam.achievements.take_screenshot =
      steam.achievements.take_screenshot->get_value ();
  if (steam.achievements.popup.animate->load ())
    config.steam.achievements.popup.animate =
      steam.achievements.popup.animate->get_value ();
  if (steam.achievements.popup.show_title->load ())
    config.steam.achievements.popup.show_title =
      steam.achievements.popup.show_title->get_value ();
  if (steam.achievements.popup.origin->load ()) {
    config.steam.achievements.popup.origin =
      SK_Steam_PopupOriginWStrToEnum (
        steam.achievements.popup.origin->get_value ().c_str ()
      );
  } else {
    config.steam.achievements.popup.origin = 3;
  }
  if (steam.achievements.popup.inset->load ())
    config.steam.achievements.popup.inset =
      steam.achievements.popup.inset->get_value ();
  if (steam.achievements.popup.duration->load ())
    config.steam.achievements.popup.duration =
      steam.achievements.popup.duration->get_value ();

  if (steam.log.silent->load ())
    config.steam.silent = steam.log.silent->get_value ();

  if (steam.system.appid->load ())
    config.steam.appid = steam.system.appid->get_value ();
  if (steam.system.init_delay->load ())
    config.steam.init_delay = steam.system.init_delay->get_value ();
  if (steam.system.auto_pump->load ())
    config.steam.auto_pump_callbacks = steam.system.auto_pump->get_value ();
  if (steam.system.notify_corner->load ())
    config.steam.notify_corner =
      SK_Steam_PopupOriginWStrToEnum (
        steam.system.notify_corner->get_value ().c_str ()
    );


  if (osd.show->load ())
    config.osd.show = osd.show->get_value ();

  if (osd.update_method.pump->load ())
    config.osd.pump = osd.update_method.pump->get_value ();

  if (osd.update_method.pump_interval->load ())
    config.osd.pump_interval = osd.update_method.pump_interval->get_value ();

  if (osd.text.red->load ())
    config.osd.red = osd.text.red->get_value ();
  if (osd.text.green->load ())
    config.osd.green = osd.text.green->get_value ();
  if (osd.text.blue->load ())
    config.osd.blue = osd.text.blue->get_value ();

  if (osd.viewport.pos_x->load ())
    config.osd.pos_x = osd.viewport.pos_x->get_value ();
  if (osd.viewport.pos_y->load ())
    config.osd.pos_y = osd.viewport.pos_y->get_value ();
  if (osd.viewport.scale->load ())
    config.osd.scale = osd.viewport.scale->get_value ();


  if (init_delay->load ())
    config.system.init_delay = init_delay->get_value ();
  if (silent->load ())
    config.system.silent = silent->get_value ();
  if (prefer_fahrenheit->load ())
    config.system.prefer_fahrenheit = prefer_fahrenheit->get_value ();

  if (ignore_rtss_delay->load ())
    config.system.ignore_rtss_delay = ignore_rtss_delay->get_value ();

  if (handle_crashes->load ())
    config.system.handle_crashes = handle_crashes->get_value ();

  if (debug_output->load ())
    config.system.display_debug_out = debug_output->get_value ();

  if (game_output->load ())
    config.system.game_output = game_output->get_value ();

  if (version->load ())
    config.system.version = version->get_value ();

  if (empty)
    return false;

  return true;
}

void
SK_SaveConfig (std::wstring name, bool close_config) {
  compatibility.ignore_raptr->set_value       (config.compatibility.ignore_raptr);
  compatibility.disable_raptr->set_value      (config.compatibility.disable_raptr);
  compatibility.rehook_loadlibrary->set_value (config.compatibility.rehook_loadlibrary);

  monitoring.memory.show->set_value           (config.mem.show);
  mem_reserve->set_value                      (config.mem.reserve);

  monitoring.fps.show->set_value              (config.fps.show);

  monitoring.io.show->set_value               (config.io.show);
  monitoring.io.interval->set_value           (config.io.interval);

  monitoring.cpu.show->set_value              (config.cpu.show);
  monitoring.cpu.interval->set_value          (config.cpu.interval);
  monitoring.cpu.simple->set_value            (config.cpu.simple);

  monitoring.gpu.show->set_value              (config.gpu.show);
  monitoring.gpu.print_slowdown->set_value    (config.gpu.print_slowdown);
  monitoring.gpu.interval->set_value          (config.gpu.interval);

  monitoring.disk.show->set_value             (config.disk.show);
  monitoring.disk.interval->set_value         (config.disk.interval);
  monitoring.disk.type->set_value             (config.disk.type);

  monitoring.pagefile.show->set_value         (config.pagefile.show);
  monitoring.pagefile.interval->set_value     (config.pagefile.interval);

  monitoring.SLI.show->set_value              (config.sli.show);
  monitoring.time.show->set_value             (config.time.show);

  osd.show->set_value                         (config.osd.show);
  osd.update_method.pump->set_value           (config.osd.pump);
  osd.update_method.pump_interval->set_value  (config.osd.pump_interval);
  osd.text.red->set_value                     (config.osd.red);
  osd.text.green->set_value                   (config.osd.green);
  osd.text.blue->set_value                    (config.osd.blue);
  osd.viewport.pos_x->set_value               (config.osd.pos_x);
  osd.viewport.pos_y->set_value               (config.osd.pos_y);
  osd.viewport.scale->set_value               (config.osd.scale);

  input.cursor.manage->set_value              (config.input.cursor.manage);
  input.cursor.keys_activate->set_value       (config.input.cursor.keys_activate);
  input.cursor.timeout->set_value             ((float)config.input.cursor.timeout / 1000.0f);

  window.borderless->set_value                (config.window.borderless);
  window.center->set_value                    (config.window.center);
  window.background_render->set_value         (config.window.background_render);
  window.background_mute->set_value           (config.window.background_mute);
  window.x_off->set_value                     (config.window.x_offset);
  window.y_off->set_value                     (config.window.y_offset);
  window.confine_cursor->set_value            (config.window.confine_cursor);
  window.fullscreen->set_value                (config.window.fullscreen);

  wchar_t wszFormattedRes [64] = { L'\0' };

  wsprintf ( wszFormattedRes, L"%lux%lu",
               config.window.res.override.x,
                 config.window.res.override.y );

  window.override->set_value (wszFormattedRes);

  if ( SK_IsInjected () ||
      (SK_GetDLLRole () & DLL_ROLE::D3D9 || SK_GetDLLRole () & DLL_ROLE::DXGI) ) {
    render.framerate.target_fps->set_value       (config.render.framerate.target_fps);
    render.framerate.prerender_limit->set_value  (config.render.framerate.pre_render_limit);
    render.framerate.buffer_count->set_value     (config.render.framerate.buffer_count);
    render.framerate.present_interval->set_value (config.render.framerate.present_interval);

    // SLI only works in Direct3D
    nvidia.sli.compatibility->set_value          (config.nvidia.sli.compatibility);
    nvidia.sli.mode->set_value                   (config.nvidia.sli.mode);
    nvidia.sli.num_gpus->set_value               (config.nvidia.sli.num_gpus);
    nvidia.sli.override->set_value               (config.nvidia.sli.override);

    if (  SK_IsInjected () ||
        ( SK_GetDLLRole () & DLL_ROLE::DXGI ) ) {
      render.framerate.max_delta_time->set_value (config.render.framerate.max_delta_time);
      render.framerate.flip_discard->set_value   (config.render.framerate.flip_discard);

      texture.d3d11.cache->set_value        (config.textures.d3d11.cache);
      texture.d3d11.precise_hash->set_value (config.textures.d3d11.precise_hash);
      texture.d3d11.dump->set_value         (config.textures.d3d11.dump);
      texture.d3d11.inject->set_value       (config.textures.d3d11.inject);
      texture.d3d11.res_root->set_value     (config.textures.d3d11.res_root);

      texture.cache.max_entries->set_value (config.textures.cache.max_entries);
      texture.cache.min_entries->set_value (config.textures.cache.min_entries);
      texture.cache.max_evict->set_value   (config.textures.cache.max_evict);
      texture.cache.min_evict->set_value   (config.textures.cache.min_evict);
      texture.cache.max_size->set_value    (config.textures.cache.max_size);
      texture.cache.min_size->set_value    (config.textures.cache.min_size);

      texture.cache.ignore_non_mipped->set_value (config.textures.cache.ignore_nonmipped);

      wsprintf ( wszFormattedRes, L"%lux%lu",
                   config.render.dxgi.res.max.x,
                     config.render.dxgi.res.max.y );

      render.dxgi.max_res->set_value (wszFormattedRes);

      wsprintf ( wszFormattedRes, L"%lux%lu",
                   config.render.dxgi.res.min.x,
                     config.render.dxgi.res.min.y );

      render.dxgi.min_res->set_value (wszFormattedRes);

      render.dxgi.swapchain_wait->set_value (config.render.framerate.swapchain_wait);

      switch (config.render.dxgi.scaling_mode) {
        case DXGI_MODE_SCALING_UNSPECIFIED:
          render.dxgi.scaling_mode->set_value (L"Unspecified");
          break;
        case DXGI_MODE_SCALING_CENTERED:
          render.dxgi.scaling_mode->set_value (L"Centered");
          break;
        case DXGI_MODE_SCALING_STRETCHED:
          render.dxgi.scaling_mode->set_value (L"Stretched");
          break;
        default:
          render.dxgi.scaling_mode->set_value (L"DontCare");
          break;
      }
    }

    if (SK_IsInjected () || SK_GetDLLRole () & DLL_ROLE::D3D9) {
      render.d3d9.force_d3d9ex->set_value     (config.render.d3d9.force_d3d9ex);
      render.d3d9.force_fullscreen->set_value (config.render.d3d9.force_fullscreen);
      render.d3d9.hook_type->set_value        (config.render.d3d9.hook_type);
      render.d3d9.refresh_rate->set_value     (config.render.d3d9.refresh_rate);
    }
  }

  steam.achievements.sound_file->set_value      (config.steam.achievements.sound_file);
  steam.achievements.play_sound->set_value      (config.steam.achievements.play_sound);
  steam.achievements.take_screenshot->set_value (config.steam.achievements.take_screenshot);
  steam.achievements.popup.origin->set_value    (
    SK_Steam_PopupOriginToWStr (config.steam.achievements.popup.origin)
  );
  steam.achievements.popup.inset->set_value      (config.steam.achievements.popup.inset);
  steam.achievements.popup.duration->set_value   (config.steam.achievements.popup.duration);
  steam.achievements.popup.animate->set_value    (config.steam.achievements.popup.animate);
  steam.achievements.popup.show_title->set_value (config.steam.achievements.popup.show_title);

  if (config.steam.appid == 0) {
    if (SK::SteamAPI::AppID () != 0 &&
        SK::SteamAPI::AppID () != 1)
      config.steam.appid = SK::SteamAPI::AppID ();
  }

  steam.system.appid->set_value              (config.steam.appid);
  steam.system.init_delay->set_value         (config.steam.init_delay);
  steam.system.auto_pump->set_value          (config.steam.auto_pump_callbacks);
  steam.system.notify_corner->set_value      (
    SK_Steam_PopupOriginToWStr (config.steam.notify_corner)
  );

  steam.log.silent->set_value                (config.steam.silent);

  init_delay->set_value                      (config.system.init_delay);
  silent->set_value                          (config.system.silent);
  prefer_fahrenheit->set_value               (config.system.prefer_fahrenheit);

  compatibility.ignore_raptr->store       ();
  compatibility.disable_raptr->store      ();
  compatibility.rehook_loadlibrary->store ();

  monitoring.memory.show->store           ();
  mem_reserve->store                      ();

  monitoring.SLI.show->store              ();
  monitoring.time.show->store             ();

  monitoring.fps.show->store              ();

  monitoring.io.show->store               ();
  monitoring.io.interval->store           ();

  monitoring.cpu.show->store              ();
  monitoring.cpu.interval->store          ();
  monitoring.cpu.simple->store            ();

  monitoring.gpu.show->store              ();
  monitoring.gpu.print_slowdown->store    ();
  monitoring.gpu.interval->store          ();

  monitoring.disk.show->store             ();
  monitoring.disk.interval->store         ();
  monitoring.disk.type->store             ();

  monitoring.pagefile.show->store         ();
  monitoring.pagefile.interval->store     ();

  input.cursor.manage->store              ();
  input.cursor.keys_activate->store       ();
  input.cursor.timeout->store             ();

  window.borderless->store                ();
  window.center->store                    ();
  window.background_render->store         ();
  window.background_mute->store           ();
  window.x_off->store                     ();
  window.y_off->store                     ();
  window.confine_cursor->store            ();
  window.fullscreen->store                ();
  window.override->store                  ();

  nvidia.api.disable->store               ();

  if (  SK_IsInjected ()                  || 
      ( SK_GetDLLRole () & DLL_ROLE::DXGI ||
        SK_GetDLLRole () & DLL_ROLE::D3D9 ) ) {
    render.framerate.target_fps->store       ();
    render.framerate.buffer_count->store     ();
    render.framerate.prerender_limit->store  ();
    render.framerate.present_interval->store ();

    if (sk::NVAPI::nv_hardware) {
      nvidia.sli.compatibility->store        ();
      nvidia.sli.mode->store                 ();
      nvidia.sli.num_gpus->store             ();
      nvidia.sli.override->store             ();
    }

    if (  SK_IsInjected () ||
        ( SK_GetDLLRole () & DLL_ROLE::DXGI ) ) {
      render.framerate.max_delta_time->store ();
      render.framerate.flip_discard->store   ();

      texture.d3d11.cache->store        ();
      texture.d3d11.precise_hash->store ();
      texture.d3d11.dump->store         ();
      texture.d3d11.inject->store       ();
      texture.d3d11.res_root->store     ();

      texture.cache.max_entries->store ();
      texture.cache.min_entries->store ();
      texture.cache.max_evict->store   ();
      texture.cache.min_evict->store   ();
      texture.cache.max_size->store    ();
      texture.cache.min_size->store    ();

      texture.cache.ignore_non_mipped->store ();

      render.dxgi.max_res->store      ();
      render.dxgi.min_res->store      ();
      render.dxgi.scaling_mode->store ();

      render.dxgi.swapchain_wait->store ();
    }

    if (  SK_IsInjected () ||
        ( SK_GetDLLRole () & DLL_ROLE::D3D9 ) ) {
      render.d3d9.force_d3d9ex->store     ();
      render.d3d9.force_fullscreen->store ();
      render.d3d9.hook_type->store        ();
      render.d3d9.refresh_rate->store     ();
    }
  }

  osd.show->store                        ();
  osd.update_method.pump->store          ();
  osd.update_method.pump_interval->store ();
  osd.text.red->store                    ();
  osd.text.green->store                  ();
  osd.text.blue->store                   ();
  osd.viewport.pos_x->store              ();
  osd.viewport.pos_y->store              ();
  osd.viewport.scale->store              ();

  steam.achievements.sound_file->store       ();
  steam.achievements.play_sound->store       ();
  steam.achievements.take_screenshot->store  ();
  steam.achievements.popup.show_title->store ();
  steam.achievements.popup.animate->store    ();
  steam.achievements.popup.duration->store   ();
  steam.achievements.popup.inset->store      ();
  steam.achievements.popup.origin->store     ();
  steam.system.notify_corner->store          ();
  steam.system.appid->store                  ();
  steam.system.init_delay->store             ();
  steam.system.auto_pump->store              ();
  steam.log.silent->store                    ();

  init_delay->store                      ();
  silent->store                          ();
  prefer_fahrenheit->store               ();

  ignore_rtss_delay->set_value           (config.system.ignore_rtss_delay);
  ignore_rtss_delay->store               ();

  handle_crashes->set_value              (config.system.handle_crashes);
  handle_crashes->store                  ();

  game_output->set_value                 (config.system.game_output);
  game_output->store                     ();

  version->set_value                     (SK_VER_STR);
  version->store                         ();

  wchar_t wszFullName [ MAX_PATH + 2 ] = { L'\0' };

  lstrcatW (wszFullName, SK_GetConfigPath ());
  lstrcatW (wszFullName,       name.c_str ());
  lstrcatW (wszFullName,             L".ini");

  dll_ini->write ( wszFullName );
  osd_ini->write ( SK_EvalEnvironmentVars (
                     L"%USERPROFILE%\\Documents\\My Mods\\SpecialK\\"
                     L"Global\\osd.ini"
                   ).c_str () );
  achievement_ini->write ( SK_EvalEnvironmentVars (
                     L"%USERPROFILE%\\Documents\\My Mods\\SpecialK\\"
                     L"Global\\achievements.ini"
                   ).c_str () );

  if (close_config) {
    if (dll_ini != nullptr) {
      delete dll_ini;
      dll_ini = nullptr;
    }

    if (osd_ini != nullptr) {
      delete osd_ini;
      osd_ini = nullptr;
    }

    if (achievement_ini != nullptr) {
      delete achievement_ini;
      achievement_ini = nullptr;
    }
  }
}

const wchar_t*
__stdcall
SK_GetVersionStr (void)
{
  return SK_VER_STR;
}