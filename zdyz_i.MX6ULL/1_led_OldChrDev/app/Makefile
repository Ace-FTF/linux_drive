CROSS_COMPILE = arm-linux-gnueabihf-
CC = gcc
RM = rm

OBJ = led0_oldchrdev_app

$(OBJ) : led0_oldchrdev_app.c
	$(CROSS_COMPILE)$(CC) $< -o $@

.PHONY:
clean:
	$(RM) $(OBJ) -rf
