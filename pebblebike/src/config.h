#ifndef CONFIG_H
#define CONFIG_H

#define PRODUCTION true


#define VERSION_PEBBLE 264
#define VERSION_TEXT "Version 2.2.0-beta4"
#define APP_COMPANY "N Jackson & JayPS"

#if PRODUCTION
  #define DEBUG false
  #define ROTATION false
#else
  #define DEBUG true
  #define ROTATION false
#endif

#define ORUXMAP true

#endif // CONFIG_H
