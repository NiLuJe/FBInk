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

ifdef DEBUG
	OUT_DIR=Debug
	CFLAGS:=$(DEBUG_CFLAGS)
	EXTRA_CFLAGS+=-DDEBUG
else
	OUT_DIR=Release
	CFLAGS?=$(OPT_CFLAGS)
endif

# Detect GCC version because reasons...
# (namely, stupid GCC builds dying on unknown -W options when they shouldn't, according to the docs)
MOAR_WARNIGS:=0
GCC_VER:=$(shell $(CC) -dumpversion)
ifeq "$(GCC_VER)" "4.2.1"
	# This is Clang (or you really need to update GCC ;D)
	MOAR_WARNIGS:=1
endif
ifeq "$(shell expr `echo $(GCC_VER) | cut -f1 -d.` \>= 7)" "1"
	# This is GCC 7
	MOAR_WARNIGS:=1
endif

# Moar warnings!
ifeq "$(MOAR_WARNIGS)" "1"
	EXTRA_CFLAGS+=-Wall -Wformat=2 -Wformat-signedness -Wformat-truncation=2
	EXTRA_CFLAGS+=-Wextra -Wunused
	EXTRA_CFLAGS+=-Wnull-dereference
	EXTRA_CFLAGS+=-Wuninitialized
	EXTRA_CFLAGS+=-Wduplicated-branches -Wduplicated-cond
	EXTRA_CFLAGS+=-Wundef
	EXTRA_CFLAGS+=-Wbad-function-cast
	EXTRA_CFLAGS+=-Wwrite-strings
	EXTRA_CFLAGS+=-Wjump-misses-init
	EXTRA_CFLAGS+=-Wlogical-op
	EXTRA_CFLAGS+=-Wstrict-prototypes -Wold-style-definition
	EXTRA_CFLAGS+=-Wshadow
	EXTRA_CFLAGS+=-Wmissing-prototypes -Wmissing-declarations
	EXTRA_CFLAGS+=-Wnested-externs
	EXTRA_CFLAGS+=-Winline
	EXTRA_CFLAGS+=-Wcast-qual
	# NOTE: GCC 8 introduces -Wcast-align=strict to warn regardless of the target architecture (i.e., like clang)
	EXTRA_CFLAGS+=-Wcast-align
	EXTRA_CFLAGS+=-Wconversion
	# Output padding info when debugging (NOTE: Clang is slightly more verbose)
	# As well as function attribute hints
	ifdef DEBUG
		EXTRA_CFLAGS+=-Wpadded
		EXTRA_CFLAGS+=-Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wsuggest-attribute=format -Wmissing-format-attribute
	endif
endif

# We need to build PIC to support running as/with a shared library
# NOTE: We should be safe with -fpic instead of -fPIC ;).
# And we also handle symbol visibility properly...
SHARED_CFLAGS+=-fpic
SHARED_CFLAGS+=-fvisibility=hidden
SHARED_CFLAGS+=-DFBINK_SHAREDLIB

# Assume we'll be safe to use by threaded applications...
EXTRA_CPPFLAGS+=-D_REENTRANT=1

# Toggle Kindle support
ifdef KINDLE
	EXTRA_CPPFLAGS+=-DFBINK_FOR_KINDLE
endif
# Toggle Legacy Kindle support
ifdef LEGACY
	EXTRA_CPPFLAGS+=-DFBINK_FOR_LEGACY
endif

# A version tag...
FBINK_VERSION=$(shell git describe)
ifdef LEGACY
	LIB_CFLAGS+=-DFBINK_VERSION='"$(FBINK_VERSION) for Kindle (Legacy)"'
else
ifdef KINDLE
	LIB_CFLAGS+=-DFBINK_VERSION='"$(FBINK_VERSION) for Kindle"'
else
	LIB_CFLAGS+=-DFBINK_VERSION='"$(FBINK_VERSION) for Kobo"'
endif
endif

# NOTE: Always use as-needed to avoid unecessary DT_NEEDED entries :)
LDFLAGS?=-Wl,--as-needed

# And we want to link against our own library ;).
ifdef DEBUG
	EXTRA_LDFLAGS+=-L./Debug
else
	EXTRA_LDFLAGS+=-L./Release
endif
ifdef SHARED
	LIBS:=-lfbink
else
	LIBS:=-l:libfbink.a
endif
# And with our own rpath for standalone distribution
ifdef STANDALONE
	EXTRA_LDFLAGS+=-Wl,-rpath=/usr/local/fbink/lib
endif
# NOTE: Don't use in production, this was to help wrap my head around fb rotation experiments...
ifdef MATHS
	EXTRA_CPPFLAGS+=-DFBINK_WITH_MATHS
	LIBS+=-lm
endif

##
# Now that we're done fiddling with flags, let's build stuff!
LIB_SRCS=fbink.c fbink_device_id.c utf8/utf8.c
CMD_SRCS=fbink_cmd.c
# Unless we're asking for a minimal build, include the Unscii fonts, too
ifdef MINIMAL
	EXTRA_CPPFLAGS+=-DFBINK_MINIMAL
else
	EXTRA_CPPFLAGS+=-DFBINK_WITH_UNSCII
	LIB_SRCS+=fbink_unscii.c
endif

# How we handle our library creation
FBINK_SHARED_FLAGS:=-shared -Wl,-soname,libfbink.so.1
FBINK_SHARED_NAME_FILE:=libfbink.so.1.0.0
FBINK_SHARED_NAME:=libfbink.so
FBINK_SHARED_NAME_VER:=libfbink.so.1
FBINK_STATIC_FLAGS:=rc
FBINK_STATIC_NAME:=libfbink.a

default: all

SHAREDLIB_OBJS:=$(LIB_SRCS:%.c=$(OUT_DIR)/shared/%.o)
STATICLIB_OBJS:=$(LIB_SRCS:%.c=$(OUT_DIR)/static/%.o)
CMD_OBJS:=$(CMD_SRCS:%.c=$(OUT_DIR)/%.o)

$(OUT_DIR)/shared/%.o: %.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LIB_CFLAGS) $(SHARED_CFLAGS) -o $@ -c $<

$(OUT_DIR)/static/%.o: %.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LIB_CFLAGS) -o $@ -c $<

$(OUT_DIR)/%.o: %.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<

outdir:
	mkdir -p $(OUT_DIR)/shared/utf8 $(OUT_DIR)/static/utf8

all: outdir fbink

staticlib: $(STATICLIB_OBJS)
	$(AR) $(FBINK_STATIC_FLAGS) $(OUT_DIR)/$(FBINK_STATIC_NAME) $(STATICLIB_OBJS)
	$(RANLIB) $(OUT_DIR)/$(FBINK_STATIC_NAME)

sharedlib: $(SHAREDLIB_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LIB_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(FBINK_SHARED_FLAGS) -o$(OUT_DIR)/$(FBINK_SHARED_NAME_FILE) $(SHAREDLIB_OBJS)
	ln -sf $(FBINK_SHARED_NAME_FILE) $(OUT_DIR)/$(FBINK_SHARED_NAME)
	ln -sf $(FBINK_SHARED_NAME_FILE) $(OUT_DIR)/$(FBINK_SHARED_NAME_VER)

fbink: $(CMD_OBJS) sharedlib staticlib
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/$@$(BINEXT) $(CMD_OBJS) $(LIBS)

strip: all
	$(STRIP) --strip-unneeded $(OUT_DIR)/$(FBINK_SHARED_NAME_FILE) $(OUT_DIR)/fbink

debug:
	$(MAKE) all DEBUG=true

shared:
	$(MAKE) all SHARED=true

release:
	$(MAKE) strip SHARED=true STANDALONE=true

kindle:
	$(MAKE) strip KINDLE=true

legacy:
	$(MAKE) strip LEGACY=true

kobo: release
	mkdir -p Kobo/usr/local/fbink/bin Kobo/usr/local/fbink/lib
	cp -av $(CURDIR)/Release/fbink Kobo/usr/local/fbink/bin
	cp -av $(CURDIR)/Release/$(FBINK_SHARED_NAME_FILE) Kobo/usr/local/fbink/lib
	ln -sf $(FBINK_SHARED_NAME_FILE) Kobo/usr/local/fbink/lib/$(FBINK_SHARED_NAME)
	ln -sf $(FBINK_SHARED_NAME_FILE) Kobo/usr/local/fbink/lib/$(FBINK_SHARED_NAME_VER)
	cp -av $(CURDIR)/README.md Kobo/usr/local/fbink/README.md
	cp -av $(CURDIR)/LICENSE Kobo/usr/local/fbink/LICENSE
	cp -av $(CURDIR)/CREDITS Kobo/usr/local/fbink/CREDITS
	tar --exclude="./mnt" --exclude="FBInk-*.zip" --owner=root --group=root -cvzf Release/KoboRoot.tgz -C Kobo .
	rm -rf Kobo/usr
	mv -v Release/KoboRoot.tgz Kobo/KoboRoot.tgz
	cp -av $(CURDIR)/README.md Kobo/README.md
	cp -av $(CURDIR)/LICENSE Kobo/LICENSE
	cp -av $(CURDIR)/CREDITS Kobo/CREDITS
	pushd Kobo && zip -r ../Release/FBInk-$(FBINK_VERSION).zip . && popd
	mv -v Release/FBInk-$(FBINK_VERSION).zip Kobo/

clean:
	rm -rf Kobo/
	rm -rf Release/*.a
	rm -rf Release/*.so*
	rm -rf Release/shared/*.o
	rm -rf Release/shared/utf8/*.o
	rm -rf Release/static/*.o
	rm -rf Release/static/utf8/*.o
	rm -rf Release/*.o
	rm -rf Release/fbink
	rm -rf Debug/*.a
	rm -rf Debug/*.so*
	rm -rf Debug/shared/*.o
	rm -rf Debug/shared/utf8/*.o
	rm -rf Debug/static/*.o
	rm -rf Debug/static/utf8/*.o
	rm -rf Debug/*.o
	rm -rf Debug/fbink

.PHONY: default outdir all staticlib sharedlib fbink strip debug shared release kindle legacy kobo clean
