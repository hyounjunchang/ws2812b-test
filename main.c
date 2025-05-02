/**
 * moved main program
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <stdarg.h>
#include <getopt.h>

#include "clk.h"
#include "gpio.h"
#include "dma.h"
#include "pwm.h"

#include "ws2811.h"

#include "ws2812b_led_control.h"

#define ARRAY_SIZE(stuff)       (sizeof(stuff) / sizeof(stuff[0]))

// defaults for cmdline options
#define TARGET_FREQ             WS2811_TARGET_FREQ
#define GPIO_PIN                10 // gpio 10 for SPI MOSI pin 19
#define DMA                     10
#define STRIP_TYPE            	WS2811_STRIP_GRB // LED strip we have is GRB

#define WIDTH                   8
#define HEIGHT                  1
#define LED_COUNT               (WIDTH * HEIGHT)

int width = WIDTH;
int height = HEIGHT;
int led_count = LED_COUNT;

// variable specific to interrupt handling, not need if not using this main() function
static uint8_t running = 1;

ws2811_t ledstring =
{
    .freq = TARGET_FREQ,
    .dmanum = DMA,
    .channel =
    {
        [0] =
        {
            .gpionum = GPIO_PIN,
            .invert = 0,
            .count = LED_COUNT,
            .strip_type = STRIP_TYPE,
            .brightness = 25,
        },
        [1] =
        {
            .gpionum = 0,
            .invert = 0,
            .count = 0,
            .brightness = 0,
        },
    },
};

ws2811_led_t *matrix;

// Example dot colors
ws2811_led_t dotcolors2[] =
{
    0x00200000,  // red
    0x00201000,  // orange
    0x00202000,  // yellow
    0x00002000,  // green
    0x00002020,  // lightblue
    0x00000020,  // blue
    0x00100010,  // purple
    0x00200010,  // pink
};

int main(int argc, char *argv[])
{
    ws2811_return_t ret;

    // initialize matrix
    matrix = malloc(sizeof(ws2811_led_t) * LED_COUNT);
	// set up test matrix
	for (int i = 0; i < LED_COUNT; i++){
		matrix[i] = dotcolors2[i%8];
	}

    // initialize LED string
    if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
        return ret;
    }

    uint32_t loop_count = 0;
    uint32_t aud_matrix[LED_COUNT] = {0};

    while (running)
    {
        if (loop_count % 2 == 1){
            aud_matrix[0] = 0x3<<29;
            aud_matrix[1] = 0x1<<29;
            aud_matrix[2] = 0x0<<29;
            aud_matrix[3] = 0x0<<29;
            aud_matrix[4] = 0x7<<29;
            aud_matrix[5] = 0x6<<29;
            aud_matrix[6] = 0x0<<29;
            aud_matrix[7] = 0x1<<29;
        }
        else{
            aud_matrix[0] = 0x0<<29;
            aud_matrix[1] = 0x1<<29;
            aud_matrix[2] = 0x0<<29;
            aud_matrix[3] = 0x0<<29;
            aud_matrix[4] = 0x2<<29;
            aud_matrix[5] = 0x5<<29;
            aud_matrix[6] = 0x7<<29;
            aud_matrix[7] = 0x0<<29;
        }

        update_led_matrix_from_sound(matrix, &aud_matrix[0],LED_COUNT);
		update_led_color(&ledstring, matrix, LED_COUNT);

        loop_count++;

        // 2 frames /sec
        usleep(1000000 / 2);
    }

    // clear values on exit
	matrix_clear(matrix, LED_COUNT);
	update_led_color(&ledstring, matrix, LED_COUNT);
    usleep(1000000 / 2);

    // to stop ws2811
    ws2811_fini(&ledstring);

    printf ("\n");
    return ret;
}
