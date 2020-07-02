SRC_DIR  := src
OBJ_DIR  := obj
TEST_DIR := test
BIN_DIR  := bin
DOC_DIR  := docs

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
TESTCFLAGS := -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -Wno-return-type
LIB := libmthread.a
LIBSO := libmthread.so
LDFLAGS  := -L.
LDLIBS := -lm

CC := gcc
AR := /usr/bin/ar
RANLIB := /usr/bin/ranlib
.PHONY: all clean lib exe docs

DEBUG := 0
ifeq ($(DEBUG),1)
CFLAGS += -DDEBUG
endif

all: $(LIB) $(EXE) $(LIBSO)

$(LIBSO) : $(OBJ)
	$(CC) -shared $(OBJ) -o $@

$(LIB) : $(OBJ)
	$(AR) rcs $(LIB) $(OBJ)
	$(RANLIB) $(LIB)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%: $(TEST_DIR)/%.o $(LIB)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(TEST_DIR)/%.o: $(TEST_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TESTCFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir $@

docs:
	doxygen $(DOC_DIR)/doxy-config

clean:
	$(RM) -rf $(OBJ) $(TEST_OBJ) $(LIB) $(LIBSO) $(EXE) $(DOC_DIR)/html $(DOC_DIR)/latex