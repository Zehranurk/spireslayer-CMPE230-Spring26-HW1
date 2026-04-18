CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -pedantic -D_POSIX_C_SOURCE=200809L

.PHONY: default grade clean

default: spireslayer

spireslayer: src/main.c src/state.c src/helpers.c src/interpreter.c src/queries.c
	$(CC) $(CFLAGS) -o $@ $^

grade:
	python3 test/grader.py ./spireslayer test-cases

clean:
	rm -f spireslayer