/*
 * newtest.c
 *
 * Copyright (c) 2014 Jeremy Garff <jer @ jers.net>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     1.  Redistributions of source code must retain the above copyright notice, this list of
 *         conditions and the following disclaimer.
 *     2.  Redistributions in binary form must reproduce the above copyright notice, this list
 *         of conditions and the following disclaimer in the documentation and/or other materials
 *         provided with the distribution.
 *     3.  Neither the name of the owner nor the names of its contributors may be used to endorse
 *         or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


static char VERSION[] = "XX.YY.ZZ";

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
#include "version.h"

#include "ws2811.h"

// user created
#include "colorspace.h"


#define ARRAY_SIZE(stuff)       (sizeof(stuff) / sizeof(stuff[0]))

// defaults for cmdline options
#define TARGET_FREQ             WS2811_TARGET_FREQ
#define GPIO_PIN                10 // gpio 10 for SPI MOSI pin 19
#define DMA                     10
#define STRIP_TYPE            	WS2811_STRIP_GRB // LED strip we have is GRB

#define WIDTH                   12
#define HEIGHT                  1
#define LED_COUNT               (WIDTH * HEIGHT)

int width = WIDTH;
int height = HEIGHT;
int led_count = LED_COUNT;

int clear_on_exit = 0;

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

static uint8_t running = 1;


// shifts color of matrix;
void matrix_shift_right(ws2811_led_t *matrix, uint32_t matrix_size){
	ws2811_led_t temp = matrix[matrix_size - 1];
	for (int i = 1; i < matrix_size; i++){
		matrix[i] = matrix[i-1];
	}
	matrix[0] = temp;
}

void matrix_clear(ws2811_led_t *matrix, uint32_t matrix_size)
{
    for (int i = 0; i < matrix_size; i++){
		matrix[i] = 0;
	}
}

// sets LED colors for ledstrip truncated if matrix size < led strip size, with rendering
void update_led_color(ws2811_t *ledstring, ws2811_led_t *matrix, uint32_t matrix_size){
	int led_channel_count = ledstring->channel[0].count;
	if (led_channel_count < matrix_size){
		matrix_size = led_channel_count;
	}
	for (int i = 0; i < matrix_size; i++){
		ledstring->channel[0].leds[i] = matrix[i];
	}

	// render ws2811
	ws2811_return_t ret;
	if ((ret = ws2811_render(ledstring)) != WS2811_SUCCESS)
	{
		fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
	}
}

void matrix_change_intensity(ws2811_led_t *matrix, uint32_t matrix_size){
	for (int i = 0; i < matrix_size; i++){

		uint32_t r = (matrix[i] & 0xFF0000) >> 16;
		uint32_t g = (matrix[i] & 0x00FF00) >> 8;
		uint32_t b = matrix[i] & 0x0000FF;

		if (r == 0x80){
			r = 0x1;
		}
		else{
			r = r << 1;
		}
		if (g == 0x80){
			g = 0x1;
		}
		else{
			g = g << 1;
		}
		if (b == 0x80){
			b = 0x1;
		}
		else{
			b = b << 1;
		}

		/*
		uint32_t r = (matrix[i] & 0xFF0000) >> 16;
		uint32_t g = (matrix[i] & 0x00FF00) >> 8;
		uint32_t b = matrix[i] & 0x0000FF;
		printf ("Initial rgb values: %x, %x, %x\r\n", r, g, b);

		RGB startRGB;
		startRGB.r = r;
		startRGB.g = g;
		startRGB.b = b;

		HSI startHSI = RGBtoHSI(startRGB);

		printf("Initial hsi values: %d, %d, %d\r\n", startHSI.h, startHSI.s, startHSI.i);

		HSI endHSI = startHSI;
		endHSI.i = endHSI.i - 0.1;
		if (endHSI.i < 0.25){
			endHSI.i = 0.85;
		}
		
		printf("Final hsi values: %d, %d, %d\r\n", endHSI.h, endHSI.s, endHSI.i);

		RGB endRGB = HSItoRGB(endHSI);
		r =  endRGB.r << 16;
		g =  endRGB.g << 8;
		b =  endRGB.b;
		*/

		// update matrix value
		matrix[i] = 0x0 | r << 16 | g << 8 | b;
		printf ("Final Matrix Value: %x\r\n", matrix[i]);
	}
}


ws2811_led_t dotcolors[] =
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

ws2811_led_t dotcolors_rgbw[] =
{
    0x00200000,  // red
    0x10200000,  // red + W
    0x00002000,  // green
    0x10002000,  // green + W
    0x00000020,  // blue
    0x10000020,  // blue + W
    0x00101010,  // white
    0x10101010,  // white + W

};

static void ctrl_c_handler(int signum)
{
	(void)(signum);
    running = 0;
}

static void setup_handlers(void)
{
    struct sigaction sa =
    {
        .sa_handler = ctrl_c_handler,
    };

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

int main(int argc, char *argv[])
{
	ws2811_return_t ret;
    matrix = malloc(sizeof(ws2811_led_t) * LED_COUNT);

	// set up matrix
	for (int i = 0; i < LED_COUNT; i++){
		matrix[i] = dotcolors[i%8];
	}

    if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
        return ret;
    }

    while (running)
    {
        matrix_change_intensity(matrix, LED_COUNT);
		//matrix_shift_right(matrix, LED_COUNT);
		update_led_color(&ledstring, matrix, LED_COUNT);

        // 15 frames /sec
        usleep(1000000 / 15);
    }

    if (clear_on_exit) {
	matrix_clear(matrix, LED_COUNT);
	ws2811_render(&ledstring);
    }

    ws2811_fini(&ledstring);

    printf ("\n");
    return ret;
}
