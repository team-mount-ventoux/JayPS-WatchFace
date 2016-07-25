#include <pebble.h>
#include "config.h"
  
DictionaryIterator s_locale_dict;


void locale_init(void) {
#ifdef ENABLE_LOCALIZE_FORCE
  //hard-coded for testing
  const char* locale_str = ENABLE_LOCALIZE_FORCE;
#else
  // Detect system locale
  const char* locale_str = i18n_get_system_locale();
#endif
  ResHandle locale_handle = NULL;
  int locale_size = 0;

  if (strncmp(locale_str, "fr", 2) == 0) {
    locale_handle = resource_get_handle(RESOURCE_ID_LOCALE_FRENCH);
    locale_size = resource_size(locale_handle);
  } else if (strncmp(locale_str, "es", 2) == 0) {
    locale_handle = resource_get_handle(RESOURCE_ID_LOCALE_SPANISH);
    locale_size = resource_size(locale_handle);
  } else if (strncmp(locale_str, "de", 2) == 0) {
    locale_handle = resource_get_handle(RESOURCE_ID_LOCALE_GERMAN);
    locale_size = resource_size(locale_handle);
  } else if (strncmp(locale_str, "it", 2) == 0) {
    locale_handle = resource_get_handle(RESOURCE_ID_LOCALE_ITALIAN);
    locale_size = resource_size(locale_handle);
  }

  // Fallback to English for unlocalized languages (0 byte files)
  if (locale_size == 0) {
    locale_handle = resource_get_handle(RESOURCE_ID_LOCALE_ENGLISH);
    locale_size = resource_size(locale_handle);
  }

  int resource_offset = 0;
  int locale_entries = 0;
  resource_offset += resource_load_byte_range(locale_handle, resource_offset, 
      (uint8_t*)&locale_entries, sizeof(locale_entries));

  struct locale {
    int32_t hashval;
    int32_t strlen;
  } locale_info;

  int dict_buffer_size = locale_size + 7 * locale_entries; //7 byte header per item
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "dict_buffer_size=%d", dict_buffer_size);
  char *dict_buffer = malloc(dict_buffer_size);
  dict_write_begin(&s_locale_dict, (uint8_t*)dict_buffer, dict_buffer_size);

  for (int i = 0; i < locale_entries; i++) {
    resource_offset += resource_load_byte_range(locale_handle,
                                                resource_offset,
                                                (uint8_t*)&locale_info,
                                                sizeof(struct locale));
    char *buffer = malloc(locale_info.strlen);
    resource_offset += resource_load_byte_range(locale_handle,
                                                resource_offset,
                                                (uint8_t*)buffer,
                                                locale_info.strlen);
    dict_write_cstring(&s_locale_dict, locale_info.hashval, buffer);
    free(buffer);
  }

  dict_write_end(&s_locale_dict);
}

char *locale_str(int hashval) { 
  Tuple *tupl = dict_find(&s_locale_dict, hashval);

  if (tupl && tupl->value->cstring) {
    return tupl->value->cstring;
  }
  return "\7"; //return blank character
}
