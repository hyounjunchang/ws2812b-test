CC=gcc
CFLAGS=

all: led_blink # Uncomment this line if you want to compile the main.c file separately

led_blink: main.c ws2812b_led_control.c mailbox.c ws2811.c pwm.c pcm.c dma.c rpihw.c
	gcc main.c ws2812b_led_control.c mailbox.c ws2811.c pwm.c pcm.c dma.c rpihw.c -lm -o led_blink

clean:
	rm -f led_blink

.PHONY: clean
