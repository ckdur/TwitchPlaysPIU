CC?=gcc
OBJ=keyboards.o joysticks.o autoplay.o KeyHandlerTwitch.o graphics.o piuinput_twitch.o piulxio_twitch.o

all: piuinput.so

piuinput.so: $(OBJ)
	$(CC) -m32 -g -shared $(OBJ) -o piuinput.so -ldl -l:libusb-0.1.so.4 -lpthread -lX11

%.o: %.c
	$(CC) -m32 -g -c -o $@ $<

.PHONY: clean

clean:
	rm -rfv $(OBJ) piuinput.so

