#ifndef COMMUNICATION_H
#define COMMUNICATION_H

void send_cmd(uint8_t cmd);
void send_version();
void communication_in_dropped_callback(AppMessageResult reason, void *context);
void communication_in_received_callback(DictionaryIterator *iter, void *context);
void communication_init();
void communication_deinit();

#endif // COMMUNICATION_H
