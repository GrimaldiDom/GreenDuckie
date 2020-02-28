# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS2  = -g -Wall -lsqlite3

# the build target executable:
TARGET2 = recieve

$(TARGET2): $(TARGET2).c
	$(CC) $(CFLAGS2) -o $(TARGET2) $(TARGET2).c

clean:
	$(RM) $(TARGET2)

