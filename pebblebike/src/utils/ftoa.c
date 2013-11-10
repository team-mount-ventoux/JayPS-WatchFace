#include "pebble.h"
#include "ftoa.h"

/* tests
  char tmp[50];
  ftoa(12.1234, tmp, 0); // ok
  ftoa(12.1234, tmp, 1); // ok
  ftoa(12.1234, tmp, 2); // ok
  ftoa(12.1234, tmp, 3); // ok
  ftoa(12.1234, tmp, 4); // 12.1233
  ftoa(12.1234, tmp, 5); // 12.12339
  ftoa(12.1234, tmp, 6); // 12.123399
  ftoa(-12.15, tmp, 1); // ok
  
  ftoa(1.1234, tmp, 3); // ok
  ftoa(1.1234, tmp, 4); // 1.1233

  ftoa(2.12345, tmp, 3); // ok
  ftoa(2.12345, tmp, 4); // ok
  ftoa(2.12345, tmp, 5); // ok
  
  ftoa(123.1234, tmp, 3); // ok
  ftoa(123.1234, tmp, 4); // 123.1233
  ftoa(123.1234, tmp, 5); // 123.12339

  ftoa(123456.1234, tmp, 3); // 12345556.125
*/

void ftoa(float value, char* buf, size_t maxlen, int decimal_digits) {
  int32_t dp = (int32_t) value;
  int32_t fp;

  if (decimal_digits < 0) {
    decimal_digits = 0;
  }
  if (decimal_digits > 8) {
    decimal_digits = 8;
  }
  
  if (decimal_digits == 0) {
    snprintf(buf, maxlen, "%ld", dp);
    return;
  }
  
  bool neg = value > 0 ? false : true;
  if (neg) {
    value = -value;
  }
  float value2 = (float)(int32_t) value;
  int coeff = 1;
  for(int i = 0; i < decimal_digits; i++) {
    coeff *= 10;
  }
  
  fp = (int32_t) ((value - value2) * coeff);
  
  if (neg) {
    dp = -dp;
  }

  /*
  APP_LOG(APP_LOG_LEVEL_DEBUG, 
          "%d dp=%ld fp=%ld coeff=%d "
          "res=%ld.%ld",
          decimal_digits, dp, fp, coeff,
          dp, fp
  );
  */
  
  
  snprintf(buf, maxlen, "%ld.%ld", dp, fp);
}