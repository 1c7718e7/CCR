CC := clang
CFLAGS      ?= -DNDEBUG
CFLAGS      += -std=c99 -Wall -pedantic -lm
SDL_CFLAGS  += $(shell pkg-config --cflags sdl)
SDL_LDFLAGS += $(shell pkg-config --libs sdl)
