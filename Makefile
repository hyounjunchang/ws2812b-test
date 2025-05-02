all: led_blink

led_blink: ws2812b_led_control.c mailbox.c ws2811.c pwm.c pcm.c dma.c rpihw.c
	gcc ws2812b_led_control.c mailbox.c ws2811.c pwm.c pcm.c dma.c rpihw.c -lm -o led_blink

clean:
	rm -f led_blink

.PHONY: clean