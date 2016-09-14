#include <pebble.h>
#include "overlays.h"
#include "config.h"

#include "pebble_process_info.h"
extern const PebbleProcessInfo __pbl_app_info;

extern uint32_t _ovly_table[][3]; // values set in the ldscript
extern uint32_t _novlys __attribute__ ((section (".data")));

static uint16_t s_current_ovl = 0xFFFF;

uint16_t overlay_load(uint16_t overlay_id)
{	
	// If the overlay is valid and not already loaded
  LOG_INFO("overlay_id:%d _novlys:%ld s_current_ovl:%d", overlay_id, _novlys, s_current_ovl);
	if (overlay_id < _novlys && s_current_ovl != overlay_id){

		// Load the overlay
		size_t size = resource_load_byte_range(
			resource_get_handle(RESOURCE_ID_OVL_FULL), 					
			_ovly_table[overlay_id][2] - _ovly_table[0][2], 			// offset in resource
			(uint8_t *)(&__pbl_app_info) + _ovly_table[overlay_id][0], 	// load address, it should be &__pbl_app_info)+0x82+0x26
			_ovly_table[overlay_id][1]);								// size of the overlay

		// Check if the loaded size is correct
		if(size == _ovly_table[overlay_id][1]) {
		  LOG_INFO("OK, size:%d", size);
		  memory_cache_flush((uint8_t *)(&__pbl_app_info) + _ovly_table[overlay_id][0], _ovly_table[overlay_id][1]);
			s_current_ovl = overlay_id;
    } else {
      LOG_INFO("error overlay_id:%d", overlay_id);
		}
	}
	return s_current_ovl;
}
