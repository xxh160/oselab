WORK_DIR  := $(shell pwd)
LIB_DIR   := src
INC_DIR   := include
BUILD_DIR := build
SRCS      := $(wildcard $(LIB_DIR)/*.asm) main.asm
OBJS      := $(patsubst %.asm, $(BUILD_DIR)/%.o, $(SRCS))
NASM      := nasm
NASMFLAGS := -f elf32 -g -I$(WORK_DIR)/$(INC_DIR)
LD        := ld
LDFLAGS   := -m elf_i386 -g -e _start
TARGET    := $(BUILD_DIR)/main

.PHONY: gdb run clean test

$(BUILD_DIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	@$(NASM) $(NASMFLAGS) -o $@ $<

$(TARGET): $(OBJS)
	@$(LD) $(LDFLAGS) -o $@ $^

gdb: $(TARGET)
	@gdb $(TARGET)

run: $(TARGET)
	@$(TARGET)

# 因为大量使用宏定义 所以可能更新会有问题 还是勤用 clean 吧
clean:
	@rm -rf $(WORK_DIR)/$(BUILD_DIR)

test:
	@echo $(SRCS)
	@echo $(OBJS)

