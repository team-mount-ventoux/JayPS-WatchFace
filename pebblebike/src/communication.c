#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "communication.h"
#include "screen_map.h"
#include "screen_live.h"

int nb_sync_error_callback = 0;
int nb_tuple_live = 0, nb_tuple_altitude = 0, nb_tuple_state = 0;
static AppTimer *reset_data_timer;

void communication_init() {
  app_message_register_inbox_received(communication_in_received_callback);
  app_message_register_inbox_dropped(communication_in_dropped_callback);

  app_message_open(/* size_inbound */ 124, /* size_outbound */ 256);
}
void communication_deinit() {
  if (reset_data_timer) {
    app_timer_cancel(reset_data_timer);
  }
}

void send_cmd(uint8_t cmd) {
    Tuplet value = TupletInteger(CMD_BUTTON_PRESS, cmd);

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if (iter == NULL)
        return;

    dict_write_tuplet(iter, &value);
    dict_write_end(iter);

    app_message_outbox_send();
}
void send_version() {
    Tuplet value = TupletInteger(MSG_VERSION_PEBBLE, VERSION_PEBBLE);

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if (iter == NULL)
        return;

    dict_write_tuplet(iter, &value);
    dict_write_end(iter);

    app_message_outbox_send();
}

/*static void send_ask_name(int8_t live_max_name) {
  Tuplet value = TupletInteger(MSG_LIVE_ASK_NAMES, live_max_name);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL)
    return;

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}*/


void communication_in_dropped_callback(AppMessageResult reason, void *context) {
  // incoming message dropped

  #if DEBUG
    if (reason != APP_MSG_OK) {
        //vibes_short_pulse();
        nb_sync_error_callback++;
    }

    //char debug1[16];
    char debug2[16];

    switch (reason) {
      case APP_MSG_OK:
        // All good, operation was successful.
        strncpy(debug2, "OK", 16);
        break;
      case APP_MSG_SEND_TIMEOUT:
        // The other end did not confirm receiving the sent data with an (n)ack in time.
        strncpy(debug2, "NOC", 16);
        break;
      case APP_MSG_SEND_REJECTED:
        // The other end rejected the sent data, with a "nack" reply.
        strncpy(debug2, "NAC", 16);
        break;
      case APP_MSG_NOT_CONNECTED:
        // The other end was not connected.
        strncpy(debug2, "NCO", 16);
        break;
      case APP_MSG_APP_NOT_RUNNING:
        // The local application was not running.
        strncpy(debug2, "NOR", 16);
        break;
      case APP_MSG_INVALID_ARGS:
        // The function was called with invalid arguments.
        strncpy(debug2, "INV", 16);
        break;
      case APP_MSG_BUSY:
        // There are pending (in or outbound) messages that need to be processed first before new ones can be received or sent.
        strncpy(debug2, "BUS", 16);
        break;
      case APP_MSG_BUFFER_OVERFLOW:
        // The buffer was too small to contain the incoming message.
        strncpy(debug2, "OVE", 16);
        break;
      case APP_MSG_ALREADY_RELEASED:
        // The resource had already been released.
        strncpy(debug2, "ALR", 16);
        break;
      case APP_MSG_CALLBACK_ALREADY_REGISTERED:
        // The callback node was already registered, or its ListNode has not been initialized.
        strncpy(debug2, "AL2", 16);
        break;
      case APP_MSG_CALLBACK_NOT_REGISTERED:
        // The callback could not be deregistered, because it had not been registered before.
        strncpy(debug2, "NOR", 16);
        break;
      case APP_MSG_OUT_OF_MEMORY:
        // The support library did not have sufficient application memory to perform the requested operation. 
        strncpy(debug2, "OUT", 16);
        break;
      case APP_MSG_CLOSED:
        // App message was closed
        strncpy(debug2, "CLO", 16);
        break;
      case APP_MSG_INTERNAL_ERROR:
        // An internal OS error prevented APP_MSG from completing an operation.
        strncpy(debug2, "INT", 16);
        break;
    }
    snprintf(s_data.debug2, sizeof(s_data.debug2),
      "#%d\napp_msg_err:\n%d - %s\ntpl_live:%d\ntpl_altitude:%d\ntpl_state:%d",
      nb_sync_error_callback,
      reason, debug2,
      nb_tuple_live, nb_tuple_altitude, nb_tuple_state
    );
    layer_mark_dirty(s_data.page_debug2);
  #endif
}

static void reset_data_timer_callback(void *data) {
  reset_data_timer = NULL;

  s_gpsdata.speed100 = 0;
  if (s_gpsdata.heartrate != 255) {
    s_gpsdata.heartrate = 0;
  }
  s_gpsdata.ascentrate = 0;
  strcpy(s_data.speed, "0");
  strcpy(s_data.ascentrate, "0");

  if (s_data.page_number == PAGE_SPEED || s_data.page_number == PAGE_HEARTRATE) {
    layer_mark_dirty(s_data.page_speed);
  }
  if (s_data.page_number == PAGE_ALTITUDE) {
    layer_mark_dirty(s_data.page_altitude);
  }
}

void communication_in_received_callback(DictionaryIterator *iter, void *context) {
    Tuple *tuple = dict_read_first(iter);
#define SIZE_OF_A_FRIEND 9
    //char friend[100];
    //int8_t live_max_name = -1;
    uint16_t time0;
    int16_t xpos = 0, ypos = 0;

    while (tuple) {
        switch (tuple->key) {
        case MSG_LIVE_NAME0:
            //vibes_short_pulse();
            strncpy(s_live.friends[0].name, tuple->value->cstring, 10);
            break;
        case MSG_LIVE_NAME1:
            strncpy(s_live.friends[1].name, tuple->value->cstring, 10);
            break;
        case MSG_LIVE_NAME2:
            strncpy(s_live.friends[2].name, tuple->value->cstring, 10);
            break;
        case MSG_LIVE_NAME3:
            strncpy(s_live.friends[3].name, tuple->value->cstring, 10);
            break;
        case MSG_LIVE_NAME4:
            strncpy(s_live.friends[4].name, tuple->value->cstring, 10);
            break;

        case MSG_LIVE_SHORT:
            nb_tuple_live++;
            //strcpy(s_data.friends, "");

            s_live.nb = tuple->value->data[0];
            if (s_live.nb > NUM_LIVE_FRIENDS) {
                s_live.nb = NUM_LIVE_FRIENDS;
            }

            for (int i = 0; i < s_live.nb; i++) {
                /*if (strcmp(s_live.friends[i].name, "") != 0) {
                  // we already know the name
                  live_max_name = i;
                }*/


                if (tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 1] >= 128) {
                    s_live.friends[i].xpos = -1 * (tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 0] + 256 * (tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 1] - 128));
                } else {
                    s_live.friends[i].xpos = tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 0] + 256 * tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 1];
                }
                if (tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 3] >= 128) {
                    s_live.friends[i].ypos = -1 * (tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 2] + 256 * (tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 3] - 128));
                } else {
                    s_live.friends[i].ypos = tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 2] + 256 * tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 3];
                }
                s_live.friends[i].distance = (tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 4] + 256 * tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 5]) * 10; // in m
                s_live.friends[i].bearing = 360 * tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 6] / 256;
                s_live.friends[i].lastviewed = tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 7] + 256 * tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 8]; // in seconds

                /*
                if (i < 3) {
                  snprintf(friend, sizeof(friend),
                    "%d/%d %s %.0f(m)\n"
                    //"%d|%d\n"
                    "b:%u lv:%d(s)\n",
                    i, s_live.nb, s_live.friends[i].name, s_live.friends[i].distance,
                    //s_live.friends[i].xpos, s_live.friends[i].ypos,
                    s_live.friends[i].bearing, s_live.friends[i].lastviewed
                  );

                  strcat(s_data.friends, friend);
                }
                */

            }
            screen_live_menu_update();
            if (s_data.page_number == PAGE_MAP) {
                layer_mark_dirty(s_data.page_map);
            }

            //if (live_max_name != s_live.nb) {
            //    send_ask_name(live_max_name);
            //}

            break;

        case ALTITUDE_DATA:
            nb_tuple_altitude++;
            change_units((tuple->value->data[0] & 0b00000001) >> 0, false);
            change_state((tuple->value->data[0] & 0b00000010) >> 1);
            s_data.debug = (tuple->value->data[0] & 0b00000100) >> 2;
            s_data.live = (tuple->value->data[0] & 0b00001000) >> 3;
            s_data.refresh_code = (tuple->value->data[0] & 0b00110000) >> 4;

            s_gpsdata.accuracy = tuple->value->data[1];
            s_gpsdata.distance100 = (tuple->value->data[2] + 256 * tuple->value->data[3]); // in 0.01km or 0.01miles
            time0 = s_gpsdata.time;
            s_gpsdata.time = tuple->value->data[4] + 256 * tuple->value->data[5];
            if (s_gpsdata.time != 0) {
                s_gpsdata.avgspeed100 = 3600 * s_gpsdata.distance100 / s_gpsdata.time; // 0.01km/h or 0.01mph
            } else {
                s_gpsdata.avgspeed100 = 0;
            }
            s_gpsdata.speed100 = ((tuple->value->data[17] + 256 * tuple->value->data[18])) * 10;
            s_gpsdata.altitude = tuple->value->data[6] + 256 * tuple->value->data[7];
            if (tuple->value->data[9] >= 128) {
                s_gpsdata.ascent = -1 * (tuple->value->data[8] + 256 * (tuple->value->data[9] - 128));
            } else {
                s_gpsdata.ascent = tuple->value->data[8] + 256 * tuple->value->data[9];
            }

            if (tuple->value->data[11] >= 128) {
                s_gpsdata.ascentrate = -1 * (tuple->value->data[10] + 256 * (tuple->value->data[11] - 128));
            } else {
                s_gpsdata.ascentrate = tuple->value->data[10] + 256 * tuple->value->data[11];
            }
            if (tuple->value->data[12] >= 128) {
                s_gpsdata.slope = -1 * (tuple->value->data[12] - 128);
            } else {
                s_gpsdata.slope = tuple->value->data[12];
            }



            if (tuple->value->data[14] >= 128) {
                xpos = -1 * (tuple->value->data[13] + 256 * (tuple->value->data[14] - 128));
            } else {
                xpos = tuple->value->data[13] + 256 * tuple->value->data[14];
            }
            if (tuple->value->data[16] >= 128) {
                ypos = -1 * (tuple->value->data[15] + 256 * (tuple->value->data[16] - 128));
            } else {
                ypos = tuple->value->data[15] + 256 * tuple->value->data[16];
            }

            if ((xpos == 0 && ypos == 0) || (time0 > s_gpsdata.time)) {
                // ignore old values (can happen if gps is stopped/restarted)
                if (s_data.debug) {
                    #if DEBUG
                        APP_LOG(APP_LOG_LEVEL_DEBUG, "==> time0=%d t=%d xpos=%d ypos=%d", time0, s_gpsdata.time, xpos, ypos);      
                        //vibes_short_pulse();
                    #endif
                }
                xpos = s_gpsdata.xpos;
                ypos = s_gpsdata.ypos;
            }
            s_gpsdata.xpos = xpos;
            s_gpsdata.ypos = ypos;

            s_gpsdata.bearing = 360 * tuple->value->data[19] / 256;
            s_gpsdata.heartrate = tuple->value->data[20];

            snprintf(s_data.accuracy,   sizeof(s_data.accuracy),   "%d",   s_gpsdata.accuracy);
            snprintf(s_data.distance,   sizeof(s_data.distance),   "%ld.%ld", s_gpsdata.distance100 / 100, s_gpsdata.distance100 % 100 / 10);
            // + 5: round instead of trunc
            snprintf(s_data.avgspeed,   sizeof(s_data.avgspeed),   "%ld.%ld", (s_gpsdata.avgspeed100 + 5) / 100, ((s_gpsdata.avgspeed100 + 5) % 100) / 10);

            if (s_data.page_number == PAGE_HEARTRATE) {
              snprintf(s_data.speed, sizeof(s_data.speed), "%d", s_gpsdata.heartrate);
            } else {
              // + 5: round instead of trunc
              snprintf(s_data.speed, sizeof(s_data.speed), "%ld.%ld", (s_gpsdata.speed100 + 5) / 100, ((s_gpsdata.speed100 + 5) % 100) / 10);
            }


            snprintf(s_data.altitude,   sizeof(s_data.altitude),   "%u",   s_gpsdata.altitude);
            snprintf(s_data.ascent,     sizeof(s_data.ascent),     "%d",   s_gpsdata.ascent);
            snprintf(s_data.ascentrate, sizeof(s_data.ascentrate), "%d",   s_gpsdata.ascentrate);
            snprintf(s_data.slope,      sizeof(s_data.slope),      "%d",   s_gpsdata.slope);

            // reset data (instant speed...) after X if no data is received
            if (reset_data_timer) {
              //APP_LOG(APP_LOG_LEVEL_DEBUG, "app_timer_cancel()");
              app_timer_cancel(reset_data_timer);
            }
            // s_data.refresh_code == 3 => _refresh_interval [5;+inf
            reset_data_timer = app_timer_register(s_data.refresh_code == 3 ? 60000 : 20000, reset_data_timer_callback, NULL);

#if DEBUG
            ftoa(s_gpsdata.distance, tmp, 10, 1);
            snprintf(s_data.debug1, sizeof(s_data.debug1),
                     "#%d us:%d|%d A:%u\n"
                     "alt:%u asc:%d\n"
                     "pos:%d|%d #%u\n"
                     //"%d|%d|%d\n"
                     "s:%d b:%u\n"
                     "D:%s km T:%u\n"
                     "%d avg:%d\n",
                     s_gpsdata.nb_received++, s_gpsdata.units, s_data.state, s_gpsdata.accuracy,
                     s_gpsdata.altitude, s_gpsdata.ascent,
                     //s_gpsdata.ascentrate, s_gpsdata.slope,
                     s_gpsdata.xpos, s_gpsdata.ypos, nb_points,
                     //s_data.debug, s_data.live, s_data.refresh_code,
                     map_scale,s_gpsdata.bearing,
                     tmp, s_gpsdata.time,
                     (int) s_gpsdata.speed, (int) s_gpsdata.avgspeed
                    );
#endif

            screen_map_update_location();

            if (s_data.page_number == PAGE_SPEED || s_data.page_number == PAGE_HEARTRATE) {
                layer_mark_dirty(s_data.page_speed);
            }
            if (s_data.page_number == PAGE_ALTITUDE) {
                layer_mark_dirty(s_data.page_altitude);
            }
            if (s_data.page_number == PAGE_LIVE_TRACKING) {
                layer_mark_dirty((Layer *) s_data.page_live_tracking);
            }
#if DEBUG
            if (s_data.page_number == PAGE_DEBUG1) {
                layer_mark_dirty(s_data.page_debug1);
            }
            if (s_data.page_number == PAGE_DEBUG2) {
                layer_mark_dirty(s_data.page_debug2);
            }
#endif
            break;

        case STATE_CHANGED:
            nb_tuple_state++;
            //vibes_short_pulse();
            change_state(tuple->value->uint8);
            break;

        case MSG_VERSION_ANDROID:
            //vibes_short_pulse();
            s_data.android_version = tuple->value->int32;
            break;

        case MSG_BATTERY_LEVEL:
          //APP_LOG(APP_LOG_LEVEL_DEBUG, "MSG_BATTERY_LEVEL:%ld", tuple->value->int32);
          s_data.phone_battery_level = tuple->value->int32;
          break;
        }
        tuple = dict_read_next(iter);
    }
/*
#if DEBUG
    snprintf(s_data.debug2, sizeof(s_data.debug2),
             "sync_error:%d\ntpl_live:%d\ntpl_altitude:%d\ntpl_state:%d",
             nb_sync_error_callback,
             nb_tuple_live, nb_tuple_altitude, nb_tuple_state
            );
#endif
*/
}
