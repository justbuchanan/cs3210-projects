FLAGS := -Wall
CC := gcc
NAME := main
SOURCE := $(NAME).c
LIB_DIR := $(PWD)

.PHONY: all

all:
	$(CC) $(FLAGS) $(SOURCE) -g `pkg-config fuse --cflags --libs` -o $(NAME)

load: clean all 
	mkdir -p /tmp/fuse
	./$(NAME) /tmp/fuse -d

clean:
	$(RM) $(NAME) *.o *.so* *~
