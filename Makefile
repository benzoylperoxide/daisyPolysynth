# Project Name
TARGET = daisyPolysynth

# Sources
CPP_SOURCES = \
	./src/main.cpp \
  	./src/midi_uart_monitor.cpp

# Library Locations
LIBDAISY_DIR = ../DaisyExamples/libDaisy/
DAISYSP_DIR = ../DaisyExamples/DaisySP/

# Let the core Makefile know we want DaisySP built/linked
USE_DAISYSP = 1

# Add your project include dir(s)
C_INCLUDES += -Isrc

# Optional quality-of-life flags
C_DEFS += -DUSE_FPU=1
OPT ?= -O3

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
