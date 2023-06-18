CC=gcc
BINARY=escalonador
INCDIR=./includes
CODEDIR=./src
PROCDIR=./processes
OBJDIR=./obj
DEBUG=true
CFILES=$(wildcard $(CODEDIR)/*.c)
OBJFILES=$(patsubst $(CODEDIR)/%.c,$(OBJDIR)/%.o,$(CFILES))
INCS = $(wildcard $(INCDIR)/*.h)
FLAGS= -Wall -I$(INCDIR)

$(shell mkdir -p $(OBJDIR))

ifeq ($(DEBUG),true)
	FLAGS += -g
endif

all: fast medium slow $(BINARY)

$(BINARY):$(OBJFILES)
	$(CC) $(FLAGS) -o $@ $^

obj/%.o:$(CODEDIR)/%.c $(INCS)
	$(CC) $(FLAGS) -c $< -o $@

fast: $(PROCDIR)/fast.c
	$(CC) $(FLAGS) -o $(PROCDIR)/$@ $^

medium: $(PROCDIR)/medium.c
	$(CC) $(FLAGS) -o $(PROCDIR)/$@ $^

slow: $(PROCDIR)/slow.c
	$(CC) $(FLAGS) -o $(PROCDIR)/$@ $^

run: $(BINARY)
	./escalonador inputfile.txt
clear:
	rm obj/*
