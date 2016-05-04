CODE= conway.c
OBJ= conway
LIB= -lmthread
FLAGS= -o
CC= clang
DIR= /usr/games
USR= /home/conway

install: 

	$(CC) $(CODE) $(LIB) $(FLAGS) $(OBJ)

	cp $(OBJ) $(DIR)

	ln -s $(DIR)/$(OBJ) $(USR)

	rm $(OBJ)


