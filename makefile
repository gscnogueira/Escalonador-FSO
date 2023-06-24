CC=gcc
BINARY=escalonador
DEBUG=true
FLAGS= -Wall

ifeq ($(DEBUG),true)
	FLAGS += -g
endif

all: fast medium slow $(BINARY)

$(BINARY): main.c
	$(CC) $(FLAGS) -o $@ $^

fast: fast.c
	$(CC) $(FLAGS) -o $@ $^

medium: medium.c
	$(CC) $(FLAGS) -o $@ $^

slow: slow.c
	$(CC) $(FLAGS) -o $@ $^

run: $(BINARY)
	./escalonador -ws inputfile.txt