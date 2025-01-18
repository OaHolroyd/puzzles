# list the separate puzzle executables
PUZZLES=2048 tileset

# compiler/linker
CC=gcc
LD=$(CC)

# flags
WARNINGS=-Wall -Wextra -pedantic -Wno-unused-parameter -Wshadow \
         -Waggregate-return -Wbad-function-cast -Wcast-align -Wcast-qual \
         -Wfloat-equal -Wformat=2 -Wlogical-op -Wmissing-include-dirs \
         -Wnested-externs -Wpointer-arith -Wconversion -Wno-sign-conversion \
         -Wredundant-decls -Wsequence-point -Wstrict-prototypes -Wswitch -Wundef \
         -Wunused-but-set-parameter -Wwrite-strings
CFLAGS=
CFLAGS_DEBUG=-O0 -g3 -DDEBUG
CFLAGS_SMALL=-Os -ffunction-sections -fdata-sections
LDFLAGS=
LDFLAGS_SMALL=-Wl,-dead_strip

# libraries
INCLUDES=-I /opt/homebrew/opt/notcurses/include
LIBS= -lNotcurses -lNotcurses-core -L/opt/homebrew/opt/notcurses/lib

# directories
SRC_DIR=./src
OBJ_DIR=./obj
TEST_DIR=./tests

# get source code
SRC_CORE=$(wildcard $(SRC_DIR)/core/*.c)
OBJ_CORE=$(addprefix $(OBJ_DIR)/, $(notdir $(SRC_CORE:.c=.o)))
DEPS_CORE=$(patsubst %.o,%.d,$(OBJ_CORE))

SRC_TEST=$(wildcard $(SRC_DIR)/tests/*.c)
OBJ_TEST=$(addprefix $(OBJ_DIR)/, $(notdir $(SRC_TEST:.c=.o)))
DEPS_TEST=$(patsubst %.o,%.d,$(OBJ_TEST))
TESTS=$(addprefix $(TEST_DIR)/, $(notdir $(SRC_TEST:.c=)))
RUN_TESTS=$(addprefix run_, $(notdir $(TESTS)))


.PHONY: all
all: $(PUZZLES)

# return the project to it's pre-build state
.PHONY: clean
clean:
	@printf "`tput bold``tput setaf 1`Cleaning`tput sgr0`\n"
	rm -rf $(OBJ_DIR) $(TEST_DIR) $(PUZZLES)

# run all tests
.PHONY: check
check: $(RUN_TESTS)

# link the core objects and the correct TUI object into a puzzle
$(PUZZLES): % : obj/%.o $(OBJ_CORE)
	@printf "`tput bold``tput setaf 2`Linking %s`tput sgr0`\n" $@
	$(LD) $(LDFLAGS) -o $@ obj/$@.o $(OBJ_CORE) $(LIBS)

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
	$(CC) $(CFLAGS) $(WARNINGS) $(INCLUDES) -I $(SRC_DIR) -MMD -MP -c -o $@ $<

# include dependency information
-include $(DEPS_CORE)
-include $(DEPS_TEST)

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
	&& printf "`tput bold``tput setaf 2`PASSED %s`tput sgr0`\n" $@ \
	|| printf "`tput bold``tput setaf 1`FAILED %s`tput sgr0`\n" $@
