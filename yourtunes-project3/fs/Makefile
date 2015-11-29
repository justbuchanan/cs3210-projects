FLAGS := -Wall -std=c++11 -ltag
CXX := clang++
NAME := main
SOURCE := $(NAME).cpp
LIB_DIR := $(PWD)

.PHONY: all

all: main

main: main.cpp main.hpp
	$(CXX) $(FLAGS) $(SOURCE) `pkg-config fuse --cflags --libs` -o $(NAME)

run: main
	mkdir -p /tmp/fuse
	./$(NAME) /tmp/fuse -d -o default_permissions

clean:
	$(RM) $(NAME) *.o *.so* *~
