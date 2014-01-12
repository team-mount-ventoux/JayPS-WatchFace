#ifndef CONFIG_H
#define CONFIG_H

#define PRODUCTION true


#define VERSION_PEBBLE 19
#define VERSION_TEXT "Version 1.3"
#define APP_COMPANY "N Jackson & JayPS"

#if PRODUCTION
  #define DEBUG false
#endif

#if !PRODUCTION
  #define DEBUG true
#endif

#define ORUXMAP true

#endif // CONFIG_H
