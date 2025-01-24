# list the separate puzzle executables
PUZZLES=2048 tileset

# compiler/linker
CC=gcc
LD=$(CC)

# flags
WARNINGS=-Wall -Wextra -pedantic -Wno-unused-parameter -Wshadow \
         -Waggregate-return -Wbad-function-cast -Wcast-align -Wcast-qual \
         -Wfloat-equal -Wformat=2 -Wmissing-include-dirs \
         -Wnested-externs -Wpointer-arith -Wconversion -Wno-sign-conversion \
         -Wredundant-decls -Wsequence-point -Wstrict-prototypes -Wswitch -Wundef \
         -Wunused-but-set-parameter -Wwrite-strings -Wvla -Wno-gnu-zero-variadic-macro-arguments
CFLAGS=-O0 -g3 -DDEBUG
CFLAGS_DEBUG=-O0 -g3 -DDEBUG
CFLAGS_SMALL=-Os -ffunction-sections -fdata-sections
LDFLAGS=
LDFLAGS_SMALL=-Wl,-dead_strip

# libraries
INCLUDES=-I.
LIBS=$(shell pkg-config ncursesw --libs)

# directories
SRC_DIR=./src
OBJ_DIR=./obj
TEST_DIR=./tests
BIN_DIR=./puzzles

# files relating to core code
SRC_CORE=$(wildcard $(SRC_DIR)/core/*.c)
OBJ_CORE=$(addprefix $(OBJ_DIR)/, $(notdir $(SRC_CORE:.c=.o)))
DEPS_CORE=$(patsubst %.o,%.d,$(OBJ_CORE))

# files relating to test code
SRC_TEST=$(wildcard $(SRC_DIR)/tests/*.c)
OBJ_TEST=$(addprefix $(OBJ_DIR)/, $(notdir $(SRC_TEST:.c=.o)))
DEPS_TEST=$(patsubst %.o,%.d,$(OBJ_TEST))
TESTS=$(addprefix $(TEST_DIR)/, $(notdir $(SRC_TEST:.c=)))
RUN_TESTS=$(addprefix run_, $(notdir $(TESTS)))

# puzzles will eventually be put in $(BIN_DIR)
BIN_PUZZLES=$(addprefix $(BIN_DIR)/, $(PUZZLES))


# build all the puzzles
.PHONY: all
all: $(PUZZLES)

# build each puzzle (alias for BIN_DIR/puzzle_name)
.PNONY: $(PUZZLES)
$(PUZZLES): % : $(BIN_DIR)/%

# rebuild the project
.PHONY: rebuild
rebuild:
	$(MAKE) clean
	$(MAKE) all

# forces a debug build
.PHONY: debug
debug: CFLAGS=$(CFLAGS_DEBUG)
debug:
	$(MAKE) clean
	$(MAKE) all

# forces a size-optimised build
.PHONY: small
small: CFLAGS=$(CFLAGS_SMALL)
small: LDLAGS=$(LDLAGS_SMALL)
small:
	$(MAKE) clean
	$(MAKE) all

# return the project to it's pre-build state
.PHONY: clean
clean:
	@printf "`tput bold``tput setaf 1`Cleaning`tput sgr0`\n"
	rm -rf $(OBJ_DIR) $(TEST_DIR) $(BIN_DIR)

# build then run all tests
.PHONY: check
check: $(RUN_TESTS)

# link the core objects and the correct TUI object into a puzzle
$(BIN_PUZZLES): $(BIN_DIR)/% : obj/%.o $(OBJ_CORE) | $(BIN_DIR)
	@printf "`tput bold``tput setaf 2`Linking %s`tput sgr0`\n" $@
	$(LD) $(LDFLAGS) -o $@ obj/$*.o $(OBJ_CORE) $(LIBS)

# link the core objects and the correct test object to make a test
$(TESTS): tests/% : obj/%.o $(OBJ_CORE) | $(TEST_DIR)
	@printf "`tput bold``tput setaf 2`Linking %s`tput sgr0`\n" $@
	$(LD) $(LDFLAGS) -o $@ obj/$*.o $(OBJ_CORE) $(LIBS)

# compile TUI code
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@printf "`tput bold``tput setaf 6`Building %s`tput sgr0`\n" $@
	$(CC) $(CFLAGS) $(WARNINGS) $(INCLUDES) -MMD -MP -c -o $@ $<

# compile core code
$(OBJ_DIR)/%.o: $(SRC_DIR)/core/%.c | $(OBJ_DIR)
	@printf "`tput bold``tput setaf 6`Building %s`tput sgr0`\n" $@
	$(CC) $(CFLAGS) $(WARNINGS) $(INCLUDES) -MMD -MP -c -o $@ $<

# compile test code
$(OBJ_DIR)/%.o: $(SRC_DIR)/tests/%.c | $(OBJ_DIR)
	@printf "`tput bold``tput setaf 6`Building %s`tput sgr0`\n" $@
	$(CC) $(CFLAGS) $(WARNINGS) $(INCLUDES) -MMD -MP -c -o $@ $<

# include dependency information
-include $(DEPS_CORE)
-include $(DEPS_TEST)

# create directory for puzzle executables
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# create directory for .o files
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# create directory for test executables
$(TEST_DIR):
	mkdir -p $(TEST_DIR)

# run each test executable
.PHONY: $(RUN_TESTS)
$(RUN_TESTS): run_% : $(TESTS)
	@$(TEST_DIR)/$* \
	&& printf "`tput bold``tput setaf 2`PASSED %s`tput sgr0`\n" $* \
	|| printf "`tput bold``tput setaf 1`FAILED %s`tput sgr0`\n" $*
