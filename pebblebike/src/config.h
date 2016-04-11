#ifndef CONFIG_H
#define CONFIG_H

////////////////////////////////////////////////////////////////////////////////

#define PRODUCTION
#define VERSION_PEBBLE 287
#define VERSION_TEXT "Version 2.5.0-alpha8"
#define APP_COMPANY "N Jackson & JayPS"

////////////////////////////////////////////////////////////////////////////////
// uncomment to enable specific functions (note: will be disable for PRODUCTION)
////////////////////////////////////////////////////////////////////////////////
#define ENABLE_LOGS
//#define ENABLE_LOGS_DEBUG
//#define ENABLE_DEBUG
//#define ENABLE_ROTATION
//#define ENABLE_DEBUG_COLOR
//#define ENABLE_DEBUG_FIELDS_SIZE
#define ENABLE_DEMO

////////////////////////////////////////////////////////////////////////////////
#ifdef PRODUCTION
  // never use them in PRODUCTION, undef them if necessary
  #undef ENABLE_LOGS
  #undef ENABLE_LOGS_DEBUG
  #undef ENABLE_DEBUG
  #undef ENABLE_ROTATION
  #undef ENABLE_DEBUG_COLOR
  #undef ENABLE_DEBUG_FIELDS_SIZE
  #undef ENABLE_DEMO
#endif

////////////////////////////////////////////////////////////////////////////////
// force functions, even in PRODUCTION
#define ENABLE_ORUXMAPS
#define ENABLE_LOCALIZE


////////////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_LOCALIZE
  //#define LOCALIZE_FORCE_FR true
#else
  #define _(a) (a)
#endif

#ifndef PBL_PLATFORM_APLITE
  // save memory on APLITE (1050 bytes)
  #define ENABLE_FUNCTION_LIVE
#endif

#ifdef ENABLE_LOGS
  #define LOG_ENTER() APP_LOG(APP_LOG_LEVEL_DEBUG, "Enter %s", __FUNCTION__)
  #define LOG_EXIT() APP_LOG(APP_LOG_LEVEL_DEBUG, "Exit %s", __FUNCTION__)
  #define LOG_INFO(args...) APP_LOG(APP_LOG_LEVEL_INFO, args);
  #ifdef ENABLE_LOGS_DEBUG
    #define LOG_DEBUG(args...) APP_LOG(APP_LOG_LEVEL_DEBUG, args);
  #else
    #define LOG_DEBUG(...)
  #endif
#else
  #define LOG_ENTER()
  #define LOG_EXIT()
  #define LOG_INFO(...)
  #define LOG_DEBUG(...)
#endif

#endif // CONFIG_H
