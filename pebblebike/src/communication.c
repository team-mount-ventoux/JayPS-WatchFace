#include "pebble_os.h"
#include "config.h"
#include "pebblebike.h"
#include "communication.h"
#include "screen_map.h"

int nb_sync_error_callback = 0;
int nb_tuple_live = 0, nb_tuple_altitude = 0, nb_tuple_state = 0;

void send_cmd(uint8_t cmd) {
  Tuplet value = TupletInteger(CMD_BUTTON_PRESS, cmd);
  
  DictionaryIterator *iter;
  app_message_out_get(&iter);
  
  if (iter == NULL)
    return;
  
  dict_write_tuplet(iter, &value);
  dict_write_end(iter);
  
  app_message_out_send();
  app_message_out_release();
}
void send_version() {
  Tuplet value = TupletInteger(MSG_VERSION_PEBBLE, VERSION_PEBBLE);
  
  DictionaryIterator *iter;
  app_message_out_get(&iter);
  
  if (iter == NULL)
    return;
  
  dict_write_tuplet(iter, &value);
  dict_write_end(iter);
  
  app_message_out_send();
  app_message_out_release();
}

/*static void send_ask_name(int8_t live_max_name) {
  Tuplet value = TupletInteger(MSG_LIVE_ASK_NAMES, live_max_name);
  
  DictionaryIterator *iter;
  app_message_out_get(&iter);
  
  if (iter == NULL)
    return;
  
  dict_write_tuplet(iter, &value);
  dict_write_end(iter);
  
  app_message_out_send();
  app_message_out_release();
}*/



void communication_in_dropped_callback(void *context, AppMessageResult app_message_error) {
  // incoming message dropped
  (void) app_message_error;
  (void) context;

  #if DEBUG
  if (app_message_error != APP_MSG_OK) {
    //vibes_short_pulse();
    nb_sync_error_callback++;
  }
  
  //char debug1[16];
  char debug2[16];

  switch (app_message_error) {
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
  }
  snprintf(s_data.debug2, sizeof(s_data.debug2),
    "#%d\napp_msg_err:\n%d - %s\ntpl_live:%d\ntpl_altitude:%d\ntpl_state:%d",
    nb_sync_error_callback,
    app_message_error, debug2,
    nb_tuple_live, nb_tuple_altitude, nb_tuple_state

  );
  layer_mark_dirty(&s_data.page_debug2);
  #endif

}

void communication_in_received_callback(DictionaryIterator *iter, void *context) {
  Tuple *tuple = dict_read_first(iter);
  #define SIZE_OF_A_FRIEND 9
  char friend[100];
  //int8_t live_max_name = -1;
  
  
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
        strcpy(s_data.friends, "");

        s_live.nb = tuple->value->data[0];
        
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
          s_live.friends[i].distance = (float) (tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 4] + 256 * tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 5]) * 10; // in km or miles
          s_live.friends[i].bearing = 360 * tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 6] / 256;
          s_live.friends[i].lastviewed = tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 7] + 256 * tuple->value->data[1 + i * SIZE_OF_A_FRIEND + 8]; // in seconds

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

        }
        if (s_data.page_number == PAGE_MAP) {
          layer_mark_dirty(&s_data.page_map);
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
        s_gpsdata.distance = (float) (tuple->value->data[2] + 256 * tuple->value->data[3]) / 100; // in km or miles
        s_gpsdata.time = tuple->value->data[4] + 256 * tuple->value->data[5];
        if (s_gpsdata.time != 0) {
          s_gpsdata.avgspeed = s_gpsdata.distance / (float) s_gpsdata.time * 3600; // km/h or mph
        } else {
          s_gpsdata.avgspeed = 0;
        }
        s_gpsdata.speed = ((float) (tuple->value->data[17] + 256 * tuple->value->data[18])) / 10;
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
            s_gpsdata.xpos = -1 * (tuple->value->data[13] + 256 * (tuple->value->data[14] - 128));
        } else {
            s_gpsdata.xpos = tuple->value->data[13] + 256 * tuple->value->data[14];
        }
        if (tuple->value->data[16] >= 128) { 
            s_gpsdata.ypos = -1 * (tuple->value->data[15] + 256 * (tuple->value->data[16] - 128));
        } else {
            s_gpsdata.ypos = tuple->value->data[15] + 256 * tuple->value->data[16];
        }
        s_gpsdata.bearing = 360 * tuple->value->data[19] / 256;

        snprintf(s_data.accuracy,   sizeof(s_data.accuracy),   "%d",   s_gpsdata.accuracy);
        snprintf(s_data.distance,   sizeof(s_data.distance),   "%.1f", s_gpsdata.distance);
        snprintf(s_data.avgspeed,   sizeof(s_data.avgspeed),   "%.1f", s_gpsdata.avgspeed);
        snprintf(s_data.speed,      sizeof(s_data.speed),      "%.1f", s_gpsdata.speed);
        
        snprintf(s_data.altitude,   sizeof(s_data.altitude),   "%u",   s_gpsdata.altitude);
        snprintf(s_data.ascent,     sizeof(s_data.ascent),     "%d",   s_gpsdata.ascent);
        snprintf(s_data.ascentrate, sizeof(s_data.ascentrate), "%d",   s_gpsdata.ascentrate);
        snprintf(s_data.slope,      sizeof(s_data.slope),      "%d",   s_gpsdata.slope);

        #if DEBUG
        snprintf(s_data.debug1, sizeof(s_data.debug1),
          "#%d us:%d|%d A:%u\n"
          "alt:%u asc:%d\n"
          "pos:%d|%d #%u\n"
          //"%d|%d|%d\n"
          "s:%d b:%u\n"
          "D:%.1f km T:%u\n"
          "%.1f avg:%.1f\n",
          s_gpsdata.nb_received++, s_gpsdata.units, s_data.state, s_gpsdata.accuracy,
          s_gpsdata.altitude, s_gpsdata.ascent,
          //s_gpsdata.ascentrate, s_gpsdata.slope,
          s_gpsdata.xpos, s_gpsdata.ypos, nb_points,
          //s_data.debug, s_data.live, s_data.refresh_code,
          map_scale,s_gpsdata.bearing,
          s_gpsdata.distance, s_gpsdata.time,
          s_gpsdata.speed, s_gpsdata.avgspeed
        );
        #endif

        screen_map_update_location();
        
        if (s_data.page_number == PAGE_SPEED) {
          layer_mark_dirty(&s_data.page_speed);
        }
        if (s_data.page_number == PAGE_ALTITUDE) {
          layer_mark_dirty(&s_data.page_altitude);
        }
        if (s_data.page_number == PAGE_LIVE_TRACKING) {
          layer_mark_dirty(&s_data.page_live_tracking);
        }
        #if DEBUG    
        if (s_data.page_number == PAGE_DEBUG1) {
          layer_mark_dirty(&s_data.page_debug1);
        }
        if (s_data.page_number == PAGE_DEBUG2) {
          layer_mark_dirty(&s_data.page_debug2);
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
    }
    tuple = dict_read_next(iter);
  }

  #if DEBUG
  snprintf(s_data.debug2, sizeof(s_data.debug2),
    "sync_error:%d\ntpl_live:%d\ntpl_altitude:%d\ntpl_state:%d",
    nb_sync_error_callback,
    nb_tuple_live, nb_tuple_altitude, nb_tuple_state
  );
  #endif 

}