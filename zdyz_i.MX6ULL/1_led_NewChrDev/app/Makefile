CROSS_COMPILE = arm-linux-gnueabihf-
CC = gcc
RM = rm

OBJ = led0_newchrdev_app

$(OBJ) : led0_newchrdev_app.c
	$(CROSS_COMPILE)$(CC) $< -o $@

.PHONY:
clean:
	$(RM) $(OBJ) -rf
