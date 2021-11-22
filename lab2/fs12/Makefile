WORK_DIR  := $(shell pwd)
ifeq ($(WORK_DIR),)
$(error Where are you?)
endif

# fat12
IMG_DIR   := fat12
IMG_NAME  := a.img
MOUNT_DIR := data

# project
LIB_DIR   := src
INC_DIR   := include
BUILD_DIR := build
TEST_DIR  := test
TEST_IN   := in
TEST_OUT  := out

# main.c fs.c
SRCS_C    := main.c
SRCS_ASM  :=
# all src makefile
include $(WORK_DIR)/$(LIB_DIR)/Makefile

# objects
OBJS_C    := $(patsubst %.c, $(BUILD_DIR)/%.c.o, $(SRCS_C))
OBJS_ASM  := $(patsubst %.asm, $(BUILD_DIR)/%.asm.o, $(SRCS_ASM))

NASM      := nasm
NASMFLAGS := -f elf32 -g -I$(INC_DIR) 
LD        := gcc
LDFLAGS   := -m32 -g -O2 -lreadline 
CC        := gcc
CFLAGS    := -m32 -g -c -I$(INC_DIR) -O2 -MMD -Wall -Werror

TARGET    := $(BUILD_DIR)/main

.PHONY: gdb run rerun clean test count submit

$(BUILD_DIR)/%.asm.o: %.asm
	@mkdir -p $(dir $@)
	@$(NASM) $(NASMFLAGS) -o $@ $<

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -o $@ $<

$(TARGET): $(OBJS_C) $(OBJS_ASM)
	@$(LD) $(LDFLAGS) -o $@ $^

gdb: $(TARGET)
	@gdb --args $(TARGET) $(WORK_DIR)/$(IMG_DIR)/$(IMG_NAME)

run: $(TARGET)
	@$(TARGET) $(WORK_DIR)/$(IMG_DIR)/$(IMG_NAME)

clean:
	@rm -rf $(WORK_DIR)/$(BUILD_DIR)

test: $(TARGET)
	@cat $(TEST_DIR)/$(TEST_IN) | $(TARGET) $(WORK_DIR)/$(IMG_DIR)/$(IMG_NAME) > $(TEST_DIR)/$(TEST_OUT)

count:
	@find . -name "*.[ch]" | xargs cat | grep -v "^$$" | wc -l

submit:
	@git add .
	@git commit -m "submit"
	@git push origin main

