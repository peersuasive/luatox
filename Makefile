CC = gcc
LD = $(CC)
RM = rm
STRIP = strip

LUA52=0

INC = -I. `pkg-config --cflags libtoxcore`
ifeq ($(LUA52),1)
	INC += `pkg-config --cflags lua5.2`
else
	INC += `pkg-config --cflags lua5.1`
endif

CFLAGS = -std=c99
CFLAGS += -fPIC
#CFLAGS += -O2
CFLAGS += -g
CFLAGS += $(INC)

LDFLAGS = -fPIC

LIBS1 = `pkg-config --libs libtoxcore`
LIBS2 = `pkg-config --libs libtoxav`

OBJS1 = lua_tox.o lua_common.o
OBJS2 = lua_toxav.o lua_common.o

TARGET1 = tox.so
TARGET2 = toxav.so

all: $(TARGET1) $(TARGET2)

%.o: %.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET1): $(OBJS1)
	@echo "Linking $@..."
	@$(LD) $(LDFLAGS) -shared -o $@ $(OBJS1) $(LIBS1)
	@$(STRIP) $@

$(TARGET2): $(OBJS2)
	@echo "Linking $@..."
	@$(LD) $(LDFLAGS) -shared -o $@ $(OBJS2) $(LIBS2)
	@$(STRIP) $@

test: all
	@echo "Testing tox..."
	@lua tests/test_tox.lua
	@echo "Testing toxav..."
	@lua tests/test_toxav.lua

clean:
	@$(RM) -f $(OBJS1) $(OBJS2)

extraclean: clean
	@$(RM) -f $(TARGET1) $(TARGET2)
