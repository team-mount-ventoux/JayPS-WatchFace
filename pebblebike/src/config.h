#ifndef CONFIG_H
#define CONFIG_H

#define PRODUCTION true


#define VERSION_PEBBLE 281
#define VERSION_TEXT "Version 2.5.0-alpha2"
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

#define ORUXMAP true

#ifdef PBL_PLATFORM_APLITE
  // save memory on APLITE (1050 bytes)
  #define FUNCTION_LIVE false
#else
  #define FUNCTION_LIVE true
#endif
#endif // CONFIG_H
