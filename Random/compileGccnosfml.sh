#!/bin/bash

g++ -std=c++1y -O0 -pthread -Wall -Wextra -Wpedantic -Wundef -Wshadow -Wno-missing-field-initializers \
	-Wpointer-arith -Wcast-align -Wwrite-strings -Wno-unreachable-code -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC \
	"${@:2}" ./$1 -o /tmp/$1.temp && /tmp/$1.temp