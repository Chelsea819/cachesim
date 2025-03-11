WORK_DIR  = $(shell pwd)
BUILD_DIR = $(WORK_DIR)/build
OBJ_DIR  = $(BUILD_DIR)/obj
BINARY   = $(BUILD_DIR)/cachesim

# 使用相对路径匹配避免绝对路径问题
CSRC = $(wildcard *.c)
# CSRC += $(wildcard */*.c)  # 如果存在子目录

INC_PATH = $(WORK_DIR)
INCLUDES = $(addprefix -I, $(INC_PATH))

CFLAGS = $(INCLUDES) -Wall -Werror -g

# 修正对象文件路径生成规则
OBJS = $(addprefix $(OBJ_DIR)/,$(notdir $(CSRC:.c=.o)))

$(OBJ_DIR)/%.o: %.c
	@echo + CC $<
	@mkdir -p $(dir $@)
	gcc $(CFLAGS) -c -o $@ $<
$(BINARY): $(OBJS)
	@echo + LD $@
	@gcc -o $@ $(OBJS) -lm

LOG ?= $(WORK_DIR)/nemu-log.txt
mainargs ?= -b
ARGS = -f $(LOG) $(mainargs)
NEMU_EXEC := $(BINARY) $(ARGS)
run: $(BINARY)
	@echo "Running cache simulator..."
	@$(BINARY) $(ARGS)

gdb: $(BINARY)
	@echo "Running cache simulator..."
	gdb -s $(BINARY) --args $(NEMU_EXEC)


clean: 
	rm -rf $(BUILD_DIR)

.PHONY: run clean