default: run

WORK_DIR  = $(shell pwd)
BUILD_DIR = $(WORK_DIR)/build
OBJ_DIR  = $(BUILD_DIR)/obj
BINARY   = $(BUILD_DIR)/cachesim

CSRC = cachesim.c

INC_PATH = $(WORK_DIR)/
INCLUDES = $(addprefix -I, $(INC_PATH))

CFLAGS = $(INCLUDES) -Wall -Werror
# OBJS = $(CSRC:%.c=$(OBJ_DIR)/%.o)

# $(OBJ_DIR)/%.o: %.c
# 	@echo + CC $<
# 	@mkdir -p $(dir $@)
# 	gcc $(CFLAGS) -c -o $@ $<

$(BINARY): $(CSRC)
	@mkdir -p $(dir $@)
	gcc $(CFLAGS) -o $@ $< -lm
# @echo + LD $@
# @ld -o $@ $(OBJS) 

LOG ?= $(NEMU_HOME)/../cachesim/nemu-log-train.txt

mainargs ?= -b

ARGS = -f $(LOG) $(mainargs)

run: $(BINARY)
	$(BINARY) $(ARGS)


clean: 
	rm -r $(BUILD_DIR)

