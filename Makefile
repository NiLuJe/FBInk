# Pickup our cross-toolchains automatically...
# c.f., http://trac.ak-team.com/trac/browser/niluje/Configs/trunk/Kindle/Misc/x-compile.sh
#       https://github.com/NiLuJe/crosstool-ng
#       https://github.com/koreader/koxtoolchain
# NOTE: We want the "bare" variant of the TC env, to make sure we vendor the right stuff...
#       i.e., source ~SVN/Configs/trunk/Kindle/Misc/x-compile.sh kobo env bare
ifdef CROSS_TC
	CC=$(CROSS_TC)-gcc
	STRIP=$(CROSS_TC)-strip
	AR=$(CROSS_TC)-gcc-ar
	RANLIB=$(CROSS_TC)-gcc-ranlib
else
	CC?=gcc
	STRIP?=strip
	AR?=gcc-ar
	RANLIB?=gcc-ranlib
endif

DEBUG_CFLAGS=-Og -fno-omit-frame-pointer -pipe -g
# Fallback CFLAGS, we honor the env first and foremost!
OPT_CFLAGS=-O2 -fomit-frame-pointer -pipe

ifeq "$(DEBUG)" "true"
	OUT_DIR=Debug
	CFLAGS:=$(DEBUG_CFLAGS)
	EXTRA_CFLAGS+=-DDEBUG
else
	OUT_DIR=Release
	CFLAGS?=$(OPT_CFLAGS)
endif

# Moar warnings!
EXTRA_CFLAGS+=-Wall -Wformat -Wformat-security
EXTRA_CFLAGS+=-Wextra -Wunused
EXTRA_CFLAGS+=-Wshadow
EXTRA_CFLAGS+=-Wmissing-prototypes
EXTRA_CFLAGS+=-Wcast-qual
# NOTE: GCC 8 introduces -Wcast-align=strict to warn regardless of the target architecture (i.e., like clang)
EXTRA_CFLAGS+=-Wcast-align
EXTRA_CFLAGS+=-Wconversion
# Output padding info when debugging (NOTE: Clang is slightly more verbose)
ifeq "$(DEBUG)" "true"
	EXTRA_CFLAGS+=-Wpadded
endif
# We need to build PIC to support running as/with a shared library
# NOTE: We should be safe with -fpic instead of -fPIC ;).
# And we handle symbol visibility properly...
ifeq "$(SHARED)" "true"
	SHARED_CFLAGS+=-fpic
	SHARED_CFLAGS+=-fvisibility=hidden
	SHARED_CFLAGS+=-DFBINK_SHAREDLIB
endif

# A version tag...
FBINK_VERSION=$(shell git describe)
EXTRA_CFLAGS+=-DFBINK_VERSION='"$(FBINK_VERSION)"'

# NOTE: Always use as-needed to avoid unecessary DT_NEEDED entries :)
LDFLAGS?=-Wl,--as-needed

# And we want to link against our own library ;).
ifeq "$(DEBUG)" "true"
	EXTRA_LDFLAGS+=-L./Debug
else
	EXTRA_LDFLAGS+=-L./Release
endif
LIBS:=-lfbink

##
# Now that we're done fiddling with flags, let's build stuff!
LIB_SRCS=fbink.c
CMD_SRCS=fbink_cmd.c

# How we handle our library creation
FBINK_SHARED_FLAGS:=-shared -Wl,-soname,libfbink.so.1
FBINK_SHARED_NAME_FILE:=libfbink.so.1.0.0
FBINK_SHARED_NAME:=libfbink.so
FBINK_SHARED_NAME_VER:=libfbink.so.1
FBINK_STATIC_FLAGS:=rc
FBINK_STATIC_NAME:=libfbink.a

default: all

SHAREDLIB_OBJS:=$(LIB_SRCS:%.c=$(OUT_DIR)/shared/%.o)
LIB_OBJS:=$(LIB_SRCS:%.c=$(OUT_DIR)/%.o)
CMD_OBJS:=$(CMD_SRCS:%.c=$(OUT_DIR)/%.o)

$(OUT_DIR)/shared/%.o: %.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) -o $@ -c $<

$(OUT_DIR)/%.o: %.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<

outdir:
	mkdir -p $(OUT_DIR)/shared

all: outdir fbink

staticlib: $(LIB_OBJS)
	$(AR) $(FBINK_STATIC_FLAGS) $(OUT_DIR)/$(FBINK_STATIC_NAME) $(LIB_OBJS)
	$(RANLIB) $(OUT_DIR)/$(FBINK_STATIC_NAME)

sharedlib: $(SHAREDLIB_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(FBINK_SHARED_FLAGS) -o$(OUT_DIR)/$(FBINK_SHARED_NAME_FILE) $(SHAREDLIB_OBJS)
	ln -sf $(FBINK_SHARED_NAME_FILE) $(OUT_DIR)/$(FBINK_SHARED_NAME)
	ln -sf $(FBINK_SHARED_NAME_FILE) $(OUT_DIR)/$(FBINK_SHARED_NAME_VER)

fbink: $(CMD_OBJS) sharedlib staticlib
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/$@$(BINEXT) $(CMD_OBJS) $(LIBS)

strip: all
	$(STRIP) --strip-unneeded $(OUT_DIR)/$(FBINK_SHARED_NAME) $(OUT_DIR)/fbink

debug:
	$(MAKE) all DEBUG=true

shared:
	$(MAKE) all SHARED=true

clean:
	rm -rf Release/*.a
	rm -rf Release/*.so*
	rm -rf Release/shared/*.o
	rm -rf Release/*.o
	rm -rf Release/fbink
	rm -rf Debug/*.a
	rm -rf Debug/*.so*
	rm -rf Debug/shared/*.o
	rm -rf Debug/*.o
	rm -rf Debug/fbink

.PHONY: default outdir all staticlib sharedlib fbink strip debug shared clean
