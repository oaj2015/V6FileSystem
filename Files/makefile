NAME = fsaccess
CC = g++
CFLAGS := 
SOURCE := src
BUILD := build

CPPFILES := $(foreach dir, $(SOURCE)/, $(notdir $(wildcard $(SOURCE)/*.cpp)))
OBJFILES := $(addprefix $(BUILD)/, $(CPPFILES:.cpp=.o))

all: fs

debug: CFLAGS += -g -DDEBUG
debug: CFLAGS += -Wall
debug: fs

fs: $(OBJFILES)
	$(CC) $^ -o $(NAME)

$(BUILD)/%.o: $(SOURCE)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)/*
	rm -f fs*
