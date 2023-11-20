# Create a makefile for the project
# commandline: gcc main.c libs/gfx.c libs/circular_linked_list.c libs/pdi_img.c games/snakegame.c -lX11 -lm -o main

# Variables
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -g
LDFLAGS = -lX11 -lm
SRC = main.c libs/gfx.c libs/circular_linked_list.c libs/pdi_img.c games/snakegame.c games/circulargame.c
OBJ = $(SRC:.c=.o)
EXEC = visual_data_structure.app

# Rules
all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

