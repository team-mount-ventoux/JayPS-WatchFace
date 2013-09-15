/*
 * RockShot
 * Copyright (C) 2013 Matthew Tole
 *
 * The functions for capturing and transmitting a screenshot from the Pebble is
 * based on httpcapture written by Edward Patel. (https://github.com/epatel)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ROCKSHOT_H
#define ROCKSHOT_H

#include "pebble_os.h"
#include "pebble_app.h"

#define MAX_SCREENSHOTS 2

void rockshot_main(PebbleAppHandlers* handlers);
void rockshot_init(AppContextRef* ctx);
void rockshot_capture_single();
/*void rockshot_capture_multiple(int count, int interval);*/
bool rockshot_capture_in_progress();
void rockshot_cancel();

#endif // ROCKSHOT_H
