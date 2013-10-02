#ifndef COMMUNICATION_H
#define COMMUNICATION_H

void send_cmd(uint8_t cmd);
void send_version();
void communication_in_dropped_callback(void *context, AppMessageResult app_message_error);
void communication_in_received_callback(DictionaryIterator *iter, void *context);

#endif // COMMUNICATION_H