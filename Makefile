CC = gcc
CFLAGS = -std=c99 -Wall -Wextra
SRCDIR = src
BINDIR = bin

SOURCES = $(SRCDIR)/main.c \
          $(SRCDIR)/fileio.c \
          $(SRCDIR)/gestione.c \
          $(SRCDIR)/filtri.c \
          $(SRCDIR)/ordinamento.c \
          $(SRCDIR)/report.c \
          $(SRCDIR)/interfaccia.c

TARGET = $(BINDIR)/centro_riparazioni

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SOURCES)
	@if not exist "$(BINDIR)" mkdir "$(BINDIR)"
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) -lm

run: all
	$(TARGET)

clean:
	@if exist "$(BINDIR)\centro_riparazioni.exe" del /q "$(BINDIR)\centro_riparazioni.exe"
