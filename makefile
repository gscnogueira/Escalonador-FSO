CC=gcc
BINARY=escalonador
INCDIR=./includes
CODEDIR=./src
OBJDIR=./obj
DEBUG=false
CFILES=$(wildcard $(CODEDIR)/*.c)
OBJFILES=$(patsubst $(CODEDIR)/%.c,$(OBJDIR)/%.o,$(CFILES))
INCS = $(wildcard $(INCDIR)/*.h)
FLAGS= -Wall -I$(INCDIR)

$(shell mkdir -p $(OBJDIR))

ifeq ($(DEBUG),true)
	FLAGS += -g
endif

all: $(BINARY)

$(BINARY):$(OBJFILES)
	$(CC) $(FLAGS) -o $@ $^

obj/%.o:$(CODEDIR)/%.c $(INCS)
	$(CC) $(FLAGS) -c $< -o $@

clear:
	rm $(OBJFILES)

