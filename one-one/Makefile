SRC_DIR  := src
OBJ_DIR  := obj
TEST_DIR := test
BIN_DIR  := bin

SRC := $(wildcard $(SRC_DIR)/*.c)
# $(info SRC is $(SRC))

OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
# $(info OBJ is $(OBJ))

TEST_SRC := $(wildcard $(TEST_DIR)/*.c)
# $(info TEST_SRC is $(TEST_SRC))

TEST_OBJ := $(TEST_SRC:$(TEST_DIR)/%.c=$(TEST_DIR)/%.o)
# $(info TEST_OBJ is $(TEST_OBJ))

EXE := $(addprefix $(BIN_DIR)/, $(notdir $(basename $(TEST_SRC))))
# $(info EXE is $(EXE))

CPPFLAGS := -I include
CFLAGS   :=  -g -Wall -fpic
# -g	: For debugging purposes
# -Wall : Print all warnings
# -fpic : Use Position Independent Code. It means that the generated machine 			  code is not dependent on being located at a specific address in order 		  to work Ex. Jumps would be generated as relative rather than absolute
LIB := libmthread.a
LDFLAGS  := -L.

CC := gcc
AR := /usr/bin/ar
.PHONY: all clean lib exe

DEBUG := 1
ifeq ($(DEBUG),1)
CFLAGS += -DDEBUG
endif

all: $(LIB) $(EXE)

$(LIB) : $(OBJ)
	$(AR) rcs $(LIB) $(OBJ)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%: $(TEST_DIR)/%.o $(LIB)
	$(CC) $(LDFLAGS) $^ -o $@

$(TEST_DIR)/%.o: $(TEST_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir $@

clean:
	$(RM) $(OBJ) $(TEST_OBJ) $(LIB) $(EXE)