#ifndef CONFIG_H
#define CONFIG_H

#define PRODUCTION false


#define VERSION_PEBBLE 17
#define VERSION_TEXT "Version 1.3 beta6"
#define APP_COMPANY "N Jackson & JayPS"

#if PRODUCTION
  #define DEBUG false
  #define ROCKSHOT false
#endif

#if !PRODUCTION
  #define DEBUG true
  #define ROCKSHOT false
#endif

#define ORUXMAP true

#endif // CONFIG_H
