SNET = -w

ifdef win
CROSS = x86_64-w64-mingw32.static-
SNET = -w -lws2_32 -liphlpapi
endif

CC=$(CROSS)g++
LD=$(CROSS)ld
AR=$(CROSS)ar
PKG_CONFIG=$(CROSS)pkg-config
SDL2_CONFIG=$(CROSS)sdl2-config

all : main.cc
	$(CC) -Wall -Wno-write-strings -std=c++11 `$(SDL2_CONFIG) --cflags` `$(PKG_CONFIG) --cflags glew` `$(PKG_CONFIG) --cflags SDL2_image` `$(PKG_CONFIG) --cflags SDL2_mixer` `$(PKG_CONFIG) --cflags SDL2_ttf` main.cc `$(SDL2_CONFIG) --libs` `$(PKG_CONFIG) --libs SDL2_image` `$(PKG_CONFIG) --libs SDL2_mixer` `$(PKG_CONFIG) --libs glew` `$(PKG_CONFIG) --libs SDL2_ttf` -o shipacade.exe
