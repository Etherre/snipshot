CC      = x86_64-w64-mingw32-gcc
WINDRES = x86_64-w64-mingw32-windres
TARGET  = Snipshot.exe

SRCDIR  = src
RESDIR  = res
OBJDIR  = obj

SRC     = $(wildcard $(SRCDIR)/*.c)
OBJ     = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
RES     = $(RESDIR)/resource.rc
RESOBJ  = $(OBJDIR)/resource.o

CFLAGS  = -O2 -flto -s -mwindows -ffunction-sections -fdata-sections \
          -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables \
          -Wall -Wextra -DWIN32_LEAN_AND_MEAN -D_WIN32_WINNT=0x0A00 \
          -I$(SRCDIR)
LDFLAGS = -Wl,--gc-sections -Wl,--strip-all
LIBS    = -lgdi32 -luser32 -lshell32 -ladvapi32

all: $(TARGET)

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(RESOBJ): $(RES) | $(OBJDIR)
	$(WINDRES) -i $(RES) -O coff -o $(RESOBJ)

$(TARGET): $(OBJ) $(RESOBJ)
	$(CC) $(OBJ) $(RESOBJ) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $(TARGET)

clean:
	rm -rf $(OBJDIR) $(TARGET)