CC     = gcc


all: tests

main.c:
	sh make-tests.sh > main.c

tests: main.c cbuffer.o test_cbuffer.c CuTest.c main.c
	$(CC) -o $@ $^
	./tests

cbuffer.o: cbuffer.c
	$(CC) -c -o $@ $^

clean:
	rm -f main.c cbuffer.o tests
