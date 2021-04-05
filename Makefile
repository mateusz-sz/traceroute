CC = gcc
COMPILER_FLAGS = -std=gnu99 -Wall -Wextra
OBJS = main.c icmp_send.c icmp_retrieve.c icmp_checksum.c error_handler.c


#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = traceroute

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(COMPILER_FLAGS) -c $(OBJS)
	$(CC) *.o -o $(OBJ_NAME)


clean:
	$(RM) *.o

distclean:
	$(RM) *.o $(OBJ_NAME)
 