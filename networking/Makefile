# Makefile for S.H.A.M. Protocol Implementation

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LIBS = -lcrypto

# Detect operating system
UNAME_S := $(shell uname -s)

# macOS specific settings
ifeq ($(UNAME_S),Darwin)
    # Try to find OpenSSL via Homebrew
    OPENSSL_PREFIX := $(shell brew --prefix openssl@3 2>/dev/null || brew --prefix openssl 2>/dev/null || echo "/opt/homebrew")
    
    # Fallback paths for different macOS/Homebrew configurations
    ifeq ($(wildcard $(OPENSSL_PREFIX)/include/openssl),)
        OPENSSL_PREFIX := /opt/homebrew
        ifeq ($(wildcard $(OPENSSL_PREFIX)/include/openssl),)
            OPENSSL_PREFIX := /usr/local
            ifeq ($(wildcard $(OPENSSL_PREFIX)/include/openssl),)
                $(error OpenSSL not found. Please install with: brew install openssl)
            endif
        endif
    endif
    
    CFLAGS += -I$(OPENSSL_PREFIX)/include
    LDFLAGS += -L$(OPENSSL_PREFIX)/lib
    
    # Add rpath so the binary can find the dynamic library
    LDFLAGS += -Wl,-rpath,$(OPENSSL_PREFIX)/lib
endif

# Linux specific settings
ifeq ($(UNAME_S),Linux)
    # Most Linux distros have OpenSSL in standard locations
endif

TARGETS = server client
SOURCES = server.c client.c
HEADERS = sham.h

all: $(TARGETS)

server: server.c $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o server server.c $(LIBS)

client: client.c $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o client client.c $(LIBS)

# Debug target to check OpenSSL paths
debug-openssl:
	@echo "Operating System: $(UNAME_S)"
ifeq ($(UNAME_S),Darwin)
	@echo "OpenSSL Prefix: $(OPENSSL_PREFIX)"
	@echo "Include Path: $(OPENSSL_PREFIX)/include"
	@echo "Library Path: $(OPENSSL_PREFIX)/lib"
	@echo "Checking for OpenSSL headers..."
	@ls -la $(OPENSSL_PREFIX)/include/openssl/ 2>/dev/null || echo "Headers not found!"
	@echo "Checking for OpenSSL libraries..."
	@ls -la $(OPENSSL_PREFIX)/lib/libcrypto* 2>/dev/null || echo "Libraries not found!"
endif

clean:
	rm -f $(TARGETS) *.o  received_file test_*.txt

test: all
	@echo "Running basic file transfer test..."
	@echo "Test file content for S.H.A.M. protocol" > test_input.txt
	./server 8080 &
	sleep 1
	./client 127.0.0.1 8080 test_input.txt output_test.txt
	@echo "Test completed. Check received_file for output."
	killall server 2>/dev/null || true
	rm -f test_input.txt output_test.txt

test-log: all
	@echo "Running test with logging enabled..."
	@echo "Test file with logging" > test_log_input.txt
	RUDP_LOG=1 ./server 8080 &
	sleep 1
	RUDP_LOG=1 ./client 127.0.0.1 8080 test_log_input.txt output_log.txt
	@echo "Check server_log.txt and client_log.txt for detailed logs"
	killall server 2>/dev/null || true
	rm -f test_log_input.txt output_log.txt

test-loss: all
	@echo "Testing with 10% packet loss..."
	@echo "Test file content for loss testing" > test_loss_input.txt
	./server 8080 0.1 &
	sleep 1
	./client 127.0.0.1 8080 test_loss_input.txt output_loss.txt 0.1
	killall server 2>/dev/null || true
	rm -f test_loss_input.txt output_loss.txt

# macOS OpenSSL installation helpers
install-deps-macos:
	@echo "Installing OpenSSL on macOS..."
	brew install openssl
	@echo "OpenSSL installed. If you get linking errors, try:"
	@echo "  export LDFLAGS=\"-L$(shell brew --prefix openssl)/lib\""
	@echo "  export CPPFLAGS=\"-I$(shell brew --prefix openssl)/include\""

# Alternative macOS installation for newer Homebrew
install-deps-macos-alt:
	@echo "Installing OpenSSL 3.x on macOS..."
	brew install openssl@3
	@echo "OpenSSL 3.x installed."

install-deps-ubuntu:
	sudo apt update
	sudo apt install libssl-dev build-essential

help:
	@echo "Available targets:"
	@echo "  all          - Build both server and client"
	@echo "  server       - Build server only"  
	@echo "  client       - Build client only"
	@echo "  clean        - Remove built files and logs"
	@echo "  test         - Run basic file transfer test"
	@echo "  test-log     - Run test with logging enabled"
	@echo "  test-loss    - Run test with packet loss simulation"
	@echo "  debug-openssl - Show OpenSSL paths and check installation"
	@echo "  install-deps-ubuntu - Install dependencies on Ubuntu/Debian"
	@echo "  install-deps-macos  - Install OpenSSL on macOS via Homebrew"
	@echo "  install-deps-macos-alt - Install OpenSSL 3.x on macOS"
	@echo ""
	@echo "Usage examples:"
	@echo "  File transfer: ./server 8080"
	@echo "                 ./client 127.0.0.1 8080 input.txt output.txt"
	@echo "  Chat mode:     ./server 8080 --chat"
	@echo "                 ./client 127.0.0.1 8080 --chat"
	@echo "  With logging:  RUDP_LOG=1 ./server 8080"
	@echo "  With loss:     ./server 8080 0.1"
	@echo ""
	@echo "Troubleshooting:"
	@echo "  If compilation fails on macOS, try: make debug-openssl"
	@echo "  If libraries not found, try: make install-deps-macos"

.PHONY: all clean test test-log test-loss install-deps-ubuntu install-deps-macos install-deps-macos-alt debug-openssl help