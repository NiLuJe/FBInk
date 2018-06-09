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
ifeq "$(SHARED)" "true"
	EXTRA_CFLAGS+=-fpic
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
FBINK_SHARED:=-shared -Wl,-soname,libfbink.so.1
FBINK_AR_FLAGS:=rc
FBINK_AR_NAME:=libfbink.a

default: all

LIB_OBJS:=$(LIB_SRCS:%.c=$(OUT_DIR)/%.o)
CMD_OBJS:=$(CMD_SRCS:%.c=$(OUT_DIR)/%.o)

$(OUT_DIR)/%.o: %.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<

outdir:
	mkdir -p $(OUT_DIR)

all: outdir fbink

staticlib: $(LIB_OBJS)
	$(AR) $(FBINK_AR_FLAGS) $(OUT_DIR)/$(FBINK_AR_NAME) $(LIB_OBJS)
	$(RANLIB) $(OUT_DIR)/$(FBINK_AR_NAME)

sharedlib: $(LIB_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(FBINK_SHARED) -o$(OUT_DIR)/$@$(BINEXT) $(LIB_OBJS)

fbink: $(CMD_OBJS) staticlib
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/$@$(BINEXT) $(CMD_OBJS) $(LIBS)

strip: all
	$(STRIP) --strip-unneeded $(OUT_DIR)/fbink

debug:
	$(MAKE) all DEBUG=true

shared:
	$(MAKE) all SHARED=true

clean:
	rm -rf Release/*.o
	rm -rf Release/fbink
	rm -rf Debug/*.o
	rm -rf Debug/fbink

.PHONY: default outdir all fbink strip debug clean
