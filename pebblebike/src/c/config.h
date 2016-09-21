#ifndef CONFIG_H
#define CONFIG_H

////////////////////////////////////////////////////////////////////////////////

#define PRODUCTION
#define VERSION_PEBBLE 331
#define VERSION_TEXT "Version 2.9.0-beta1"
#define APP_COMPANY "N Jackson & JayPS"

////////////////////////////////////////////////////////////////////////////////
// uncomment to enable specific functions (note: will be disable for PRODUCTION)
////////////////////////////////////////////////////////////////////////////////
#define ENABLE_LOGS
#define ENABLE_LOGS_DEBUG
//#define ENABLE_DEBUG
//#define ENABLE_DEBUG_COLOR
//#define ENABLE_DEBUG_FIELDS_SIZE
//#define ENABLE_LOCALIZE_FORCE "it"
//#define ENABLE_DEMO
//#define ENABLE_SCREENB_NAVIGATION

////////////////////////////////////////////////////////////////////////////////
#ifdef PRODUCTION
  // never use them in PRODUCTION, undef them if necessary
  #undef ENABLE_LOGS
  #undef ENABLE_LOGS_DEBUG
  #undef ENABLE_DEBUG
  #undef ENABLE_DEBUG_COLOR
  #undef ENABLE_DEBUG_FIELDS_SIZE
  #undef ENABLE_LOCALIZE_FORCE
  #undef ENABLE_DEMO
#endif

////////////////////////////////////////////////////////////////////////////////
// force functions, even in PRODUCTION
#define ENABLE_ORUXMAPS
#define ENABLE_LOCALIZE
#define ENABLE_NAVIGATION
#define ENABLE_MAP_SKIP_POINT_OUTSIDE

#define ENABLE_OVERLAYS

//#ifndef PBL_SDK_2
#ifdef _PBL_API_EXISTS_app_glance_reload
//#if PBL_API_EXISTS(app_glance_reload)
#define ENABLE_GLANCE
#endif
//#endif

////////////////////////////////////////////////////////////////////////////////
#ifndef ENABLE_LOCALIZE
  #define _(a) (a)
#endif

#ifndef PBL_PLATFORM_APLITE
  // save memory on APLITE
  // function live 1050 bytes
  #define ENABLE_FUNCTION_LIVE
 // Heart zones in menu > 740 bytes
  #define ENABLE_MENU_HEART_ZONES
  // don't deinit objects in deinit -- save at least 700 bytes
  #define ENABLE_APP_DEINIT
  #define ENABLE_ROTATION
  #define ENABLE_NAVIGATION_FULL
  //#define ENABLE_MAP_SKIP_POINT_OUTSIDE
#endif


#ifdef PBL_PLATFORM_DIORITE
  #define HAS_TWO_HRM
  #define ENABLE_INTERNAL_HRM
  //#define DEBUG_REPLACE_CADENCE_BY_HEARTRATE
#endif

#ifdef ENABLE_OVERLAYS
  //#define ENABLE_FUNCTION_LIVE
 // Heart zones in menu > 740 bytes
  #define ENABLE_MENU_HEART_ZONES
  // don't deinit objects in deinit -- save at least 700 bytes
 // #define ENABLE_APP_DEINIT
  #define ENABLE_ROTATION
  #define ENABLE_NAVIGATION_FULL
#endif


#define NAV_NB_POINTS 20
#ifdef ENABLE_OVERLAYS
  #define ENABLE_NAVIGATION_FULL
  #if PBL_PLATFORM_APLITE
    #define MAP_NUM_POINTS 200
    #define NAV_NB_POINTS_STORAGE 200
  #else
    #define MAP_NUM_POINTS 1500
    #define NAV_NB_POINTS_STORAGE 1280
  #endif
#else
  #ifdef ENABLE_NAVIGATION_FULL
    // 5 * 256
    #define NAV_NB_POINTS_STORAGE 1280
  #else
    #define NAV_NB_POINTS_STORAGE NAV_NB_POINTS
  #endif

  // 4 Bytes/point
  // to compute correct values, use DEMO mode, cycle through all screens including menu
  #if PBL_PLATFORM_APLITE
    #define MAP_NUM_POINTS 100
  #else
    #define MAP_NUM_POINTS 1500
  #endif
#endif

#ifdef ENABLE_LOGS
  #define LOG_ENTER() APP_LOG(APP_LOG_LEVEL_DEBUG, "Enter %s", __FUNCTION__)
  #define LOG_EXIT() APP_LOG(APP_LOG_LEVEL_DEBUG, "Exit %s", __FUNCTION__)
  #define LOG_ERROR(args...) APP_LOG(APP_LOG_LEVEL_ERROR, args);
  #define LOG_INFO(args...) APP_LOG(APP_LOG_LEVEL_INFO, args);
  #ifdef ENABLE_LOGS_DEBUG
    #define LOG_DEBUG(args...) APP_LOG(APP_LOG_LEVEL_DEBUG, args);
  #else
    #define LOG_DEBUG(...)
  #endif
#else
  #define LOG_ENTER()
  #define LOG_EXIT()
  #define LOG_ERROR(...)
  #define LOG_INFO(...)
  #define LOG_DEBUG(...)
#endif

#ifdef ENABLE_DEMO
  #define PB_IF_DEMO_ELSE(if_true, if_false) (if_true)
#else
  #define PB_IF_DEMO_ELSE(if_true, if_false) (if_false)
#endif

#endif // CONFIG_H
