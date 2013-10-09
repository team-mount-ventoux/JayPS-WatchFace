#ifndef CONFIG_H
#define CONFIG_H

// 5DD35873-3BB6-44D6-8255-0E61BC3B97F5
#define MY_UUID { 0x5D, 0xD3, 0x58, 0x73, 0x3B, 0xB6, 0x44, 0xD6, 0x82, 0x55, 0x0E, 0x61, 0xBC, 0x3B, 0x97, 0xF5 }

#define VERSION_PEBBLE 17
#define VERSION_MAJOR 1
#define VERSION_MINOR 3
#define VERSION_TEXT "Version 1.3 beta6"
#define APP_COMPANY "N Jackson & JayPS"

#define PRODUCTION true

#if PRODUCTION
  #define DEBUG false
  #define ROCKSHOT false
  #define APP_NAME "Pebble Bike 1.3.0-beta6"
#endif

#if !PRODUCTION
  #define DEBUG true
  #define ROCKSHOT true
  #define APP_NAME "PB 1.3.0-beta6"
#endif

#define ORUXMAP true

#endif // CONFIG_H