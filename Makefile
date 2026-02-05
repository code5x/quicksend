# ================= PROJECT =================

PROJECT := quicksend

CC      := gcc
CFLAGS  := -Wall -Wextra -O2
LDFLAGS := -pthread

# ================= PLATFORM DETECT =================
# Detect Termux
ifdef PREFIX
    INSTALL_DIR := $(PREFIX)/bin
else
    INSTALL_DIR := /usr/local/bin
endif

# ================= DIRECTORIES =================

SRC_DIR    := src
COMMON_DIR := $(SRC_DIR)/common
SEND_DIR   := $(SRC_DIR)/send
RECV_DIR   := $(SRC_DIR)/recv

BUILD_DIR  := build
BIN_DIR    := bin

# ================= SOURCE FILES =================

COMMON_SRC := \
    $(COMMON_DIR)/utils.c \
    $(COMMON_DIR)/log.c \
    $(COMMON_DIR)/net.c

SEND_SRC := \
    $(SEND_DIR)/send_main.c \
    $(SEND_DIR)/send_args.c \
    $(SEND_DIR)/send_menu.c \
    $(SEND_DIR)/send_transfer.c

RECV_SRC := \
    $(RECV_DIR)/recv_main.c \
    $(RECV_DIR)/recv_server.c \
    $(RECV_DIR)/recv_transfer.c

COMMON_OBJ := $(COMMON_SRC:%.c=$(BUILD_DIR)/%.o)
SEND_OBJ   := $(SEND_SRC:%.c=$(BUILD_DIR)/%.o)
RECV_OBJ   := $(RECV_SRC:%.c=$(BUILD_DIR)/%.o)

SEND_BIN := $(BIN_DIR)/send
RECV_BIN := $(BIN_DIR)/recv

# ================= DEFAULT TARGET =================

all: dirs $(SEND_BIN) $(RECV_BIN)

# ================= CREATE DIRECTORIES =================

dirs:
	@mkdir -p $(BUILD_DIR)/$(COMMON_DIR)
	@mkdir -p $(BUILD_DIR)/$(SEND_DIR)
	@mkdir -p $(BUILD_DIR)/$(RECV_DIR)
	@mkdir -p $(BIN_DIR)

# ================= COMPILE RULE =================

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ================= LINK =================

$(SEND_BIN): $(COMMON_OBJ) $(SEND_OBJ)
	$(CC) $(COMMON_OBJ) $(SEND_OBJ) -o $@ $(LDFLAGS)

$(RECV_BIN): $(COMMON_OBJ) $(RECV_OBJ)
	$(CC) $(COMMON_OBJ) $(RECV_OBJ) -o $@ $(LDFLAGS)

# ================= INSTALL =================

install: all
	@echo "Installing to $(INSTALL_DIR)"
	@install -Dm755 $(SEND_BIN) $(INSTALL_DIR)/send
	@install -Dm755 $(RECV_BIN) $(INSTALL_DIR)/recv
	@echo "Install complete."

# ================= UNINSTALL =================

uninstall:
	@echo "Removing from $(INSTALL_DIR)"
	@rm -f $(INSTALL_DIR)/send
	@rm -f $(INSTALL_DIR)/recv
	@echo "Uninstall complete."

# ================= CLEAN =================

clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(BIN_DIR)
	@echo "Clean complete."

# ================= PHONY =================

.PHONY: all install uninstall clean dirs
