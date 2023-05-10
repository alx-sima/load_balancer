# Copyright 2023 Sima Alexandru (312CA)
CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -g

TARGET=tema2

HEADERS=$(wildcard *.h)
SRC=$(wildcard *.c)
OBJ=$(SRC:%.c=%.o)
DEP=$(OBJ:%.o=%.d)

.PHONY: all build doc format pack clean

build: $(TARGET)

all: build doc tags format

doc: Doxyfile $(SRC) $(HEADERS)
	doxygen

format: $(SRC) $(HEADERS)
	clang-format -i $?

tags: $(SRC) $(HEADERS)
	ctags $^

pack: format $(TARGET).zip

$(TARGET).zip: README.md Makefile $(SRC) $(HEADERS)
	zip -FSr $@ $^

clean:
	rm -f $(TARGET) $(TARGET).zip tags vgcore.* *.o *.d *.h.gch

$(TARGET): $(OBJ)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $^ -c -MMD -MP -MF $(@:.o=.d)

-include $(DEP)
