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

#include "rockshot.h"
#include "pebble_os.h"
#include "pebble_app.h"

#define ROCKSHOT_CAPTURE_SIZE 3024
#define ROCKSHOT_COOKIE_MULTIPLE 7625001
#define ROCKSHOT_COOKIE_SEND 7625002
#define ROCKSHOT_MAX_BYTES 128

#define ROCKSHOT_KEY_OFFSET 76250
#define ROCKSHOT_KEY_DATA 76251
#define ROCKSHOT_KEY_CHECK 76252

typedef struct {
  unsigned char data[ROCKSHOT_CAPTURE_SIZE];
  int bytes_sent;
} Screenshot;

struct GContext {
  void **ptr;
};

void do_multiple_capture(void);
void rockshot_timer_handler(AppContextRef app_ctx, AppTimerHandle handle, uint32_t cookie);
void get_screenshot(Screenshot* shot);
void send_screenshot(Screenshot* shot);
void send_bytes(Screenshot* shot);
void send_more_bytes();
void data_received(DictionaryIterator *received, void *context);

static bool in_progress;
static Screenshot screenshots[MAX_SCREENSHOTS];
static AppTimerHandle timer_multiple;
static AppTimerHandle timer_sending;
static int multiple_index;
static int multiple_interval;
static int multiple_count;

static void (*timer_handler_fwd) (AppContextRef app_ctx, AppTimerHandle handle, uint32_t cookie);
static void (*in_received_fwd) (DictionaryIterator *received, void *context);
static void *ctx_ref;

/**
 PUBLIC FUNCTIONS
 **/

void rockshot_main(PebbleAppHandlers* handlers) {
  in_progress = false;
  timer_handler_fwd = handlers->timer_handler;
  handlers->timer_handler = rockshot_timer_handler;

  in_received_fwd = handlers->messaging_info.default_callbacks.callbacks.in_received;
  handlers->messaging_info.default_callbacks.callbacks.in_received = data_received;
}

void rockshot_init(AppContextRef* ctx) {
  ctx_ref = ctx;
}


void rockshot_capture_single() {
  if (in_progress) { return; }
  in_progress = true;

  get_screenshot(&screenshots[0]);
  send_screenshot(&screenshots[0]);
}

void rockshot_capture_multiple(int count, int interval) {
  if (in_progress) { return; }
  in_progress = true;

  multiple_index = 0;
  multiple_count = count;
  multiple_interval = interval;
  do_multiple_capture();
}

bool rockshot_capture_in_progress() {
  return in_progress;
}

void rockshot_cancel() {
  app_timer_cancel_event(ctx_ref, timer_multiple);
  in_progress = false;
}

/**
 PRIVATE FUNCTIONS
 **/

void do_multiple_capture() {
  get_screenshot(&screenshots[multiple_index]);
  timer_multiple = app_timer_send_event(ctx_ref, multiple_interval, ROCKSHOT_COOKIE_MULTIPLE);
}

void rockshot_timer_handler(AppContextRef app_ctx, AppTimerHandle handle, uint32_t cookie) {
  switch (cookie) {
    case ROCKSHOT_COOKIE_MULTIPLE:
      do_multiple_capture();
    break;
    case ROCKSHOT_COOKIE_SEND:
      send_bytes(&screenshots[0]);
    break;
    default:
      if (timer_handler_fwd) {
        timer_handler_fwd(app_ctx, handle, cookie);
      }
  }
}

void data_received(DictionaryIterator *received, void *context) {
  Tuple* tup_check = dict_find(received, ROCKSHOT_KEY_CHECK);
  if (tup_check == NULL) {
    if (in_received_fwd) {
      in_received_fwd(received, context);
    }
    return;
  }

  if (strcmp(tup_check->value->cstring, "rockshot") != 0) {
    if (in_received_fwd) {
      in_received_fwd(received, context);
    }
    return;
  }

  if (in_progress) {
    return;
  }

  rockshot_capture_single();
}

void get_screenshot(Screenshot* shot) {
  struct GContext *gctx = app_get_current_graphics_context();
  unsigned char *ptr = (unsigned char *)(*gctx->ptr);

  int len = 0;
  for (int y = 0; y < 168; y += 1) {
    for (int x=0; x < 18; x += 1) {
      shot->data[len] = *ptr++;
      len += 1;
    }
    ptr++;
    ptr++;
  }
}

void send_screenshot(Screenshot* shot) {
  shot->bytes_sent = 0;
  send_more_bytes();
}

void send_bytes(Screenshot* shot) {
  int bytes_to_send = ROCKSHOT_CAPTURE_SIZE - shot->bytes_sent;
  if (bytes_to_send > ROCKSHOT_MAX_BYTES) {
    bytes_to_send = ROCKSHOT_MAX_BYTES;
  }

  if (bytes_to_send <= 0) {
    in_progress = false;
    return;
  }

  Tuplet tup_offset = TupletInteger(ROCKSHOT_KEY_OFFSET, shot->bytes_sent);
  Tuplet tup_data = TupletBytes(ROCKSHOT_KEY_DATA, &shot->data[shot->bytes_sent], bytes_to_send);

  DictionaryIterator* dict;
  AppMessageResult result = app_message_out_get(&dict);
  if (result == APP_MSG_BUSY) {
    send_more_bytes();
    return;
  }

  dict_write_tuplet(dict, &tup_offset);
  dict_write_tuplet(dict, &tup_data);
  dict_write_end(dict);

  result = app_message_out_send();
  if (result == APP_MSG_BUSY) {
    send_more_bytes();
    return;
  }
  app_message_out_release();

  shot->bytes_sent += bytes_to_send;
  send_more_bytes();
}

void send_more_bytes() {
  timer_sending = app_timer_send_event(ctx_ref, 50, ROCKSHOT_COOKIE_SEND);
}