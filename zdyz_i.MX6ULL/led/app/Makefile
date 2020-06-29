CROSS_COMPILE = arm-linux-gnueabihf-
CC = gcc
RM = rm

OBJ = led0_app

$(OBJ) : led0_app.c
	$(CROSS_COMPILE)$(CC) $< -o $@

.PHONY:
clean:
	$(RM) $(OBJ) -rf
