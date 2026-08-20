# Linux 主机单元测试（不依赖 MinGW / Wine）
CC ?= gcc
CFLAGS = -std=c11 -O0 -g -Wall -Wextra -Wno-unused-parameter -I . -I ../src

run_tests: test_logic.c win_stubs.c windows.h windowsx.h shellapi.h
	$(CC) $(CFLAGS) -o run_tests test_logic.c win_stubs.c

test: run_tests
	./run_tests

clean:
	rm -f run_tests

.PHONY: test clean
