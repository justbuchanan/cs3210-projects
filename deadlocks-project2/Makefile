FLAGS := -fPIC -O3 -g -Wall -Werror
CC := gcc
MAJOR := 0
MINOR := 1
NAME := class_thread
VERSION := $(MAJOR).$(MINOR)
SOURCE := $(NAME).c
LIB_DIR := $(PWD)

.PHONY: all

all: proc.ko lib

lib: lib$(NAME).so

lib$(NAME).so: lib$(NAME).so.$(VERSION)
	ldconfig -v -n .
	ln -s lib$(NAME).so.$(MAJOR) lib$(NAME).so

lib$(NAME).so.$(VERSION): $(NAME).o
	$(CC) -shared -g -Wl,-soname,lib$(NAME).so.$(MAJOR) $^ -lpthread -o $@

$(NAME).o: 
	$(CC) -fPIC -g -c $(SOURCE)

clean: proc.ko_clean
	$(RM) *.o *.so*

proc.ko: 
	cd proc && $(MAKE) all && cd ../

proc.ko_clean: 
	cd proc && $(MAKE) clean && cd ../

