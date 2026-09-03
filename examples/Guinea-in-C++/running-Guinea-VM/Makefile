main:
	clang++ -c test.cc -o test.o
	clang -c ../../../src/guinea.c -Wpedantic -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -O2 -fomit-frame-pointer -Wshadow -o guinea.o
	clang++ test.o guinea.o -o test.out
	./test.out