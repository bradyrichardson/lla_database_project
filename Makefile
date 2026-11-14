# declare variables here
CC = gcc
CFLAGS = -Wall -g -I./inc
TARGET = ./bin/out
SRC := $(wildcard ./src/*.c)
OBJS := $(patsubst %.c, %.o, $(SRC))

# final target
default: $(TARGET)

# intermediate targets, $^ is all dependencies of the current rule
$(TARGET): $(OBJS)
		$(CC) $(CFLAGS) -o $(TARGET) $^

# -c flag means compile-only, we link in next step; $< is first dependency of the current rule
./src/%.o: ./src/%.c $(wildcard ./inc/*.h)
		$(CC) $(CFLAGS) -c $< -o $@

# cleanup target
clean:
		rm -f ./src/*.o
		rm -rf ./bin/*