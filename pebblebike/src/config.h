#ifndef CONFIG_H
#define CONFIG_H

#define PRODUCTION true


#define VERSION_PEBBLE 250
#define VERSION_TEXT "Version 2.1.0-alpha1"
#define APP_COMPANY "N Jackson & JayPS"

#if PRODUCTION
  #define DEBUG false
#endif

#if !PRODUCTION
  #define DEBUG true
#endif

#define ORUXMAP true

#endif // CONFIG_H
