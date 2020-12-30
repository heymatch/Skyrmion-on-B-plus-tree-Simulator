# set C++ compiler
CC := g++
CFLAGS := -std=c++11

# initial


# source
ifeq ($(OS), Windows_NT)
SRC_LIST := $(shell dir /b .\src\*.hpp)
else
SRC_LIST := $(shell ls ./src/*.hpp)
endif
SRC_PATH := ./src/
SRC := $(SRC_LIST)

# object
OBJ_PATH := ./obj/
ifeq ($(OS), Windows_NT)
OBJ := $(patsubst %.hpp, $(OBJ_PATH)%.o, $(SRC_LIST)) ./obj/main.o
else
OBJ := $(patsubst $(SRC_PATH)%.hpp, $(OBJ_PATH)%.o, $(SRC_LIST)) ./obj/main.o
endif

# part compile
fast: compile

# whole compile
all: clear compile

# compile only
compile: $(OBJ)
ifeq ($(OS), Windows_NT)
	$(CC) $(CFLAGS) -o Skyrmion.exe $(OBJ)
else
	$(CC) $(CFLAGS) -o Skyrmion $(OBJ)
endif

# compile main
$(OBJ_PATH)main.o: $(SRC_PATH)main.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

# compile pair of .cpp and .hpp 	
$(OBJ_PATH)%.o: $(SRC_PATH)%.cpp $(SRC_PATH)%.hpp
	$(CC) $(CFLAGS) -o $@ -c $<

# clear .exe/.out and .o
.PHONY: clear
clear:
ifeq ($(OS), Windows_NT)
	del .\obj\*.o
	del Skyrmion.exe
else
	rm ./obj/*.o
	rm ./Skyrmion
endif