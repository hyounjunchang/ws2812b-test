#ifndef __WS2812B_CONTROL_H__
#define __WS2812B_CONTROL_H__

#include "clk.h"
#include "gpio.h"
#include "dma.h"
#include "pwm.h"

#include "ws2811.h"

// update RGB matrix values from sound, using wrong size will cause segmentation fault
void update_led_matrix_from_sound(ws2811_led_t *matrix,  uint32_t* sound_level, uint32_t size);

// shifts led color matrix rightward
void matrix_shift_right(ws2811_led_t *matrix, uint32_t matrix_size);

// sets RGB values to #000000 (black) for all matrix
void matrix_clear(ws2811_led_t *matrix, uint32_t matrix_size);

// sets LED colors for ledstrip: truncated if matrix size < led strip size
// renders output to LED strip via SPIs
void update_led_color(ws2811_t *ledstring, ws2811_led_t *matrix, uint32_t matrix_size);

// shifts value of r/g/b channels by << 1, sets each r/g/b channel to 0x1 if channel >= 0x80
void matrix_change_intensity(ws2811_led_t *matrix, uint32_t matrix_size);




// interrupt handlers are not listed here.


#endif