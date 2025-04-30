all: led_blink

led_blink: main.c mailbox.c ws2811.c pwm.c pcm.c dma.c rpihw.c
	gcc main.c mailbox.c ws2811.c pwm.c pcm.c dma.c rpihw.c -lm -o led_blink

clean:
	rm -f led_blink

.PHONY: clean