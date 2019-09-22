all: main.c
	gcc main.c DEV_Config.c OLED_Driver.c -o expono -Wno-pointer-to-int-cast -lm -lcjson -lwiringPi

clean:
	$(RM) main
