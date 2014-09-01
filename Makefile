## TODO: guess architecture and OS
##       and cross compile from here

CC = gcc
LD = $(CC)
RM = rm
STRIP = strip

S := @

DEBUG ?= yes

LUA52=0

STATIC ?= yes

ifeq ($(LUA52),1)
	INC += `pkg-config --cflags lua5.2`
else
	INC += `pkg-config --cflags lua5.1`
endif

ifeq (${STATIC},yes)
	SO   = so
	A    = a
	DEST = lin
	HOST = x86_64-linux-gnu
	## if x86_64
	LDFLAGS += -fPIC
	PIC  = -with-pic
	
	STATIC_TOX = -Wl,-Bstatic -ltoxcore
	STATIC_TOX += -lsodium
	#STATIC_TOX += -Wl,-Bdynamic -lsodium

	STATIC_TOXAV = -Wl,-Bstatic -ltoxav $(STATIC_TOX)


	STATIC_TOXDNS = -Wl,-Bstatic -ltoxdns $(STATIC_TOX)

	LIBS  = -Wl,-Bdynamic -lm -lrt
	#LIB_TOX = -lsodium
	LIB_TOXAV = -Wl,-Bdynamic -lopus -lvpx

	STATIC_FLAGS = -L./toxcore/build/$(DEST)/lib
	STATIC_FLAGS += -L./libsodium/build/$(DEST)/lib

	TOX    = toxcore/build/$(DEST)/lib/libtoxcore.$(A)
	TOXAV  = toxcore/build/$(DEST)/lib/libtoxav.$(A)
	TOXDNS = toxcore/build/$(DEST)/lib/libtoxdns.$(A)
	NACL   = libsodium/build/$(DEST)/lib/libsodium.$(A)

	INC += -I./toxcore/build/$(DEST)/include
	DO_TOX = ./toxcore/toxcore/tox.h
	DO_TOXAV = ./toxcore/toxav/toxav.h
	DO_TOXDNS = ./toxcore/toxdns/toxdns.h
else
	INC   += -I. `pkg-config --cflags libtoxcore`
	LIB_TOX = `pkg-config --libs libtoxcore`
	LIB_TOXAV = `pkg-config --libs libtoxav`
	## if x86_64
	PIC  = -with-pic
	LDFLAGS += -fPIC -lrt
endif

INC += -I. 

CFLAGS += -std=c99
CFLAGS += -fPIC -Wall -Wno-unused-variable -Wno-unused-function
ifeq (${DEBUG},yes)
	CFLAGS += -g
else
	CFLAGS += -O2
endif
CFLAGS += $(INC)

LTOX    = tox.$(SO)
LTOXAV  = toxav.$(SO)
LTOXDNS = toxdns.$(SO)

LTOX_O 	  = lua_tox.o lua_common.o
LTOXAV_O  = lua_toxav.o lua_common.o
LTOXDNS_O = lua_toxdns.o lua_common.o

all: $(LTOX) $(LTOXAV) $(LTOXDNS)

$(NACL):
	@echo "Building $(NACL)..."
	@cd libsodium && autoreconf -if \
		&& CC=$(CC) CXX=$(CXX) ./configure $(PIC) \
			--host=$(HOST) \
			--prefix=$(PWD)/libsodium/build/$(DEST) \
		&& make install

$(TOX): $(NACL) $(DO_TOX)
	@echo "Building $(TOX)..."
	@cd toxcore && autoreconf -if \
		&& CC=$(CC) CXX=$(CXX) ./configure $(PIC) \
			--host=$(HOST) \
			--prefix=$(PWD)/toxcore/build/$(DEST) \
			--with-libsodium-libs=$(PWD)/libsodium/build/$(DEST)/lib \
			--with-libsodium-headers=$(PWD)/libsodium/build/$(DEST)/include \
		&& make install

%.o: %.c
	@echo "Compiling $@..."
	$(S)$(CC) $(CFLAGS) -o $@ -c $<

$(LTOX): $(TOX) $(LTOX_O)
	@echo "Linking $@..."
	$(S)$(LD) $(LDFLAGS) -shared -o $@ $(LTOX_O) $(STATIC_FLAGS) $(STATIC_TOX) $(LIB_TOX) $(LIBS) 
	@$(STRIP) $@

$(LTOXAV): $(TOXAV) $(LTOXAV_O) $(DO_TOXAV)
	@echo "Linking $@..."
	$(S)$(LD) $(LDFLAGS) -shared -o $@ $(LTOXAV_O) $(STATIC_FLAGS) $(STATIC_TOXAV) $(LIB_TOXAV) $(LIBS) 
	@$(STRIP) $@

$(LTOXDNS): $(TOXDNS) $(LTOXDNS_O) $(DO_TOXDNS)
	@echo "Linking $@..."
	$(S)$(LD) $(LDFLAGS) -shared -o $@ $(LTOXDNS_O) $(STATIC_FLAGS) $(STATIC_TOXDNS) $(LIB_TOXDNS) $(LIBS)
	@$(STRIP) $@

test: all
	@echo "Testing tox..."
	@lua tests/test_tox.lua
	@echo "Testing toxav..."
	@lua tests/test_toxav.lua
	@echo "Testing toxdns..."
	@lua tests/test_toxdns.lua

clean:
	@echo Cleaning...
	@$(RM) -f $(LTOX_O) $(LTOXAV_O) $(LTOXDNS_O)

extraclean: clean
	@$(RM) -f $(LTOX) $(LTOXAV) $(LTOXDNS)

distclean: extraclean
	@$(RM) -rf toxcore/build/lin
	@$(RM) -rf toxcore/build/win
	@$(RM) -rf toxcore/build/osx
	@$(RM) -rf toxcore/build/ios
	@$(RM) -rf toxcore/build/and
	@$(RM) -rf libsodium/build/lin
	@$(RM) -rf libsodium/build/win
	@$(RM) -rf libsodium/build/osx
	@$(RM) -rf libsodium/build/ios
	@$(RM) -rf libsodium/build/and
