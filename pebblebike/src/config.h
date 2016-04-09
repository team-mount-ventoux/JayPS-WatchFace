#ifndef CONFIG_H
#define CONFIG_H

#define PRODUCTION true


#define VERSION_PEBBLE 287
#define VERSION_TEXT "Version 2.5.0-alpha8"
#define APP_COMPANY "N Jackson & JayPS"

#if PRODUCTION
  #define DEBUG false
  #define ROTATION false
  #define DEMO false
#else
  #define DEBUG false
  #define ROTATION false
  //#define DEBUG_COLOR true
  //#define DEBUG_FIELDS_SIZE true
  #define DEMO true
#endif

#define ORUXMAPS true
#define LOCALIZE true
#if LOCALIZE
  //#define LOCALIZE_FORCE_FR true
#endif
#if !LOCALIZE
  #define _(a) (a)
#endif


#ifdef PBL_PLATFORM_APLITE
  // save memory on APLITE (1050 bytes)
  #define FUNCTION_LIVE false
#else
  #define FUNCTION_LIVE true
#endif


#if PRODUCTION
  #define LOG_ENTER()
  #define LOG_EXIT()
  #define LOG_INFO(...)
  #define LOG_DEBUG(...)
#else
  #define LOG_ENTER() APP_LOG(APP_LOG_LEVEL_DEBUG, "Enter %s", __FUNCTION__)
  #define LOG_EXIT() APP_LOG(APP_LOG_LEVEL_DEBUG, "Exit %s", __FUNCTION__)
  #define LOG_INFO(args...) APP_LOG(APP_LOG_LEVEL_INFO, args);
  #define LOG_DEBUG(args...) APP_LOG(APP_LOG_LEVEL_DEBUG, args);
#endif
#endif // CONFIG_H
