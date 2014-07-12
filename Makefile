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
CFLAGS += -fPIC -Wall -Wno-unused-variable -Wno-unused-function
#CFLAGS += -O2
CFLAGS += -g
CFLAGS += $(INC)

LDFLAGS = -fPIC

LIBS1 = `pkg-config --libs libtoxcore`
LIBS2 = `pkg-config --libs libtoxav`

OBJS1 = lua_tox.o lua_common.o
OBJS2 = lua_toxav.o lua_common.o
OBJS3 = lua_toxdns.o lua_common.o

TARGET1 = tox.so
TARGET2 = toxav.so
TARGET3 = toxdns.so

all: $(TARGET1) $(TARGET2) $(TARGET3)

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

$(TARGET3): $(OBJS3)
	@echo "Linking $@..."
	@$(LD) $(LDFLAGS) -shared -o $@ $(OBJS3) $(LIBS1)
	@$(STRIP) $@

test: all
	@echo "Testing tox..."
	@lua tests/test_tox.lua
	@echo "Testing toxav..."
	@lua tests/test_toxav.lua
	@echo "Testing toxdns..."
	@lua tests/test_toxdns.lua

clean:
	@$(RM) -f $(OBJS1) $(OBJS2) $(OBJS3)

extraclean: clean
	@$(RM) -f $(TARGET1) $(TARGET2) $(TARGET3)
