ADSLIB_OBJ = adslib.o

SRC = $(wildcard src/*.c)
HEADER = $(wildcard include/*.h)
OBJ = $(subst .c,.o,$(subst src,obj,$(SRC)))

CC = gcc
CC_FLAGS = -c -W -Wall -pedantic

all: config $(OBJ)

./obj/%.o: ./src/%.c ./include/%.h
	$(CC) $< $(CC_FLAGS) -DADS_STRING_EXTENDED -o $@

config:
	@ mkdir -p obj

clean:
	@ rm -rf obj

# # # # # # # # # # # # # # # # # # # # 

PREFIX ?= /usr/local
LIB_PATH = $(DESTDIR)$(PREFIX)/lib
INCLUDE_PATH = $(DESTDIR)$(PREFIX)/include/adslib

install: $(OBJ)
	gcc $(OBJ) -fPIC -shared -o obj/libadslib.so
	install -d $(LIB_PATH)
	install -m 644 obj/libadslib.so $(LIB_PATH)
	install -d $(INCLUDE_PATH)
	install -m 644 $(HEADER) $(INCLUDE_PATH)
	ldconfig