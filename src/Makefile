COMPILER = gcc
COMPILER_FLAGS = -g -O0 -Wall -Werror 
LINKER_FLAGS = -lncurses -lm

PROG_NAME = main
SOURCE_FILES = $(wildcard ../src/*.c) \
							 $(wildcard $(CPATH)/*.c)
#SOURCE_FILES = main.c

prog:
	$(COMPILER) -o $(PROG_NAME) $(SOURCE_FILES) $(COMPILER_FLAGS) $(LINKER_FLAGS)
clean:
	rm main
	rm -rf main.dSYM
