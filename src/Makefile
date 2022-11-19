CC = gcc
CFLAGS = -std=c11 -Werror -Wall -Wconversion
LDLIBS = -lm
OUTPUT = ps4

bmp.o: bmp.c
	$(CC) $(CFLAGS) -c $^ $(LDLIBS)

transformations.o: transformations.c
	$(CC) $(CFLAGS) -c $^ $(LDLIBS)

main.o: main.c
	$(CC) $(CFLAGS) -c $^ $(LDLIBS)

all: bmp.o transformations.o main.o
	cppcheck --enable=performance,unusedFunction --error-exitcode=1 *.c
	$(CC) $(CFLAGS) $^ -o $(OUTPUT) $(LDLIBS)

clean:
	rm *.o $(OUTPUT)

push:
	git commit -am "$(MSG)"
	git push