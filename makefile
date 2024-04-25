TARGET_SRV = bin/server
TARGET_CLI = bin/client

SRC_SRV = $(wildcard src/srv/*.c)
OBJ_SRV = $(SRC_SRV:src/srv/%.c=obj/srv/%.o)

SRC_CLI = $(wildcard src/cli/*.c)
OBJ_CLI = $(SRC_CLI:src/cli/%.c=obj/cli/%.o)

default: $(TARGET_SRV) $(TARGET_CLI)

run: clean default

clean:
	rm -f obj/srv/*.o
	rm -f bin/*

$(TARGET_SRV): $(OBJ_SRV)
	gcc -o $@ $?

$(OBJ_SRV): obj/srv/%.o: src/srv/%.c
	mkdir -p $(@D)
	gcc -c $< -o $@ -Iinclude

$(TARGET_CLI): $(OBJ_CLI)
	gcc -o $@ $?

$(OBJ_CLI): obj/cli/%.o: src/cli/%.c
	mkdir -p $(@D)
	gcc -c $< -o $@ -Iinclude
