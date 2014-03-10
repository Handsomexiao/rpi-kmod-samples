DEBUG	= -O2
CC	= gcc
INCLUDE	= -I. -I/home/pi/linux/include
DEFS	= -D_GNU_SOURCE

CFLAGS	= $(DEBUG) $(DEFS) -Wformat=2 -Wall -Winline $(INCLUDE) -pipe -fPIC

SRC	= ultrasonic_c_int.c ultrasonic_cloop.c

OBJ	=	$(SRC:.c=.o)

all: $(OBJ)
	@echo [Compile] $<
	@$(CC) -c $(CFLAGS) $< -o $@
