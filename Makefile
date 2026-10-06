CC = gcc
STD ?= c23
CFLAGS ?= -O2 -Wall -Wextra -Wpedantic -Wconversion -Wshadow
CPPFLAGS ?=
LDFLAGS ?=
BASE = -std=$(STD) $(CFLAGS) $(CPPFLAGS) -Isrc
ALG_OBJECTS = build/algoritmos.o build/algoritmos_counted.o

.PHONY: all test clean
all: benchmark.exe

build:
	mkdir -p build

build/algoritmos.o: src/algoritmos.c src/algoritmos.h | build
	$(CC) $(BASE) -c $< -o $@

build/algoritmos_counted.o: src/algoritmos.c src/algoritmos.h | build
	$(CC) $(BASE) -DINSTRUMENTED -c $< -o $@

build/dados.o: src/dados.c src/dados.h | build
	$(CC) $(BASE) -c $< -o $@

build/tempo.o: src/tempo.c src/tempo.h | build
	$(CC) $(BASE) -c $< -o $@

build/benchmark.o: src/benchmark.c src/algoritmos.h src/dados.h src/tempo.h | build
	$(CC) $(BASE) -c $< -o $@

benchmark.exe: build/benchmark.o build/tempo.o build/dados.o $(ALG_OBJECTS)
	$(CC) $(BASE) $^ $(LDFLAGS) -o $@

test_algoritmos.exe: tests/test_algoritmos.c src/algoritmos.h src/dados.h build/dados.o $(ALG_OBJECTS)
	$(CC) $(BASE) tests/test_algoritmos.c build/dados.o $(ALG_OBJECTS) $(LDFLAGS) -o $@

test: test_algoritmos.exe
	./test_algoritmos.exe

clean:
	rm -rf build benchmark.exe test_algoritmos.exe
