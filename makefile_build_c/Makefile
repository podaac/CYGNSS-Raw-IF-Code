#-------------------------------------------------
# CYGNSS Raw IF Data Processor Makefile Version
#-------------------------------------------------

LIBS = -lm -lfftw3

SRC = main.c \
	mainwindow.c \
	CYGNSS_DDMP_Processing.c \
	CYGNSS_DDMP_Main.c \
	fftw_acquire.c \
	code_table.c

# object files
OBJ = $(SRC:.c=.o)

# name of executable
OUT = CYGNSS_DDM_Processor

# gcc c++ compiler
CC=gcc -std=c99

all: $(SRC)
	$(CC) -g -c $(SRC) 
	$(CC) -o $(OUT) $(OBJ) $(LIBS)

clean: 
	rm *.o
