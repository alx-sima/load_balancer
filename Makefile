# Copyright 2023 Sima Alexandru (312CA)
CC=gcc
CFLAGS=-std=c99 -Wall -Wextra

TARGET=tema2
SRC=$(wildcard *.c)
OBJ=$(SRC:%.c=%.o)
DEP=$(OBJ:%.o=%.d)

.PHONY: build format pack clean

build: $(TARGET)

tags: $(SRC)
	ctags $?

format: $(SRC)
	clang-format -i $?

$(TARGET): $(OBJ)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $^ -c -MMD -MP -MF $(@:.o=.d)

-include $(DEP)

pack:

clean:
	rm -f $(TARGET) tags *.o *.d *.h.gch

