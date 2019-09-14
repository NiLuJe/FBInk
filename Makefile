# SPDX-License-Identifier: GPL-3.0-or-later
#
# Pickup our cross-toolchains automatically...
# c.f., http://trac.ak-team.com/trac/browser/niluje/Configs/trunk/Kindle/Misc/x-compile.sh
#       https://github.com/NiLuJe/crosstool-ng
#       https://github.com/koreader/koxtoolchain
# NOTE: We want the "bare" variant of the TC env, to make sure we vendor the right stuff...
#       i.e., source ~SVN/Configs/trunk/Kindle/Misc/x-compile.sh kobo env bare
ifdef CROSS_TC
	CC:=$(CROSS_TC)-gcc
	CXX:=$(CROSS_TC)-g++
	STRIP:=$(CROSS_TC)-strip
	# NOTE: This relies on GCC plugins! Enforce AR & RANLIB to point to their real binary and not the GCC wrappers if your TC doesn't support that!
	AR:=$(CROSS_TC)-gcc-ar
	RANLIB:=$(CROSS_TC)-gcc-ranlib
else
	CC?=gcc
	CXX?=g++
	STRIP?=strip
	AR?=gcc-ar
	RANLIB?=gcc-ranlib
endif

DEBUG_CFLAGS:=-Og -fno-omit-frame-pointer -pipe -g
# Fallback CFLAGS, we honor the env first and foremost!
OPT_CFLAGS:=-O2 -fomit-frame-pointer -pipe

ifdef DEBUG
	OUT_DIR:=Debug
	CFLAGS?=$(DEBUG_CFLAGS)
	CXXFLAGS?=$(DEBUG_CFLAGS)
	EXTRA_CPPFLAGS+=-DDEBUG
else
	OUT_DIR:=Release
	CFLAGS?=$(OPT_CFLAGS)
	CXXFLAGS?=$(OPT_CFLAGS)
	EXTRA_CPPFLAGS+=-DNDEBUG
endif

# Explictly enforce debug CFLAGS for the debug target (vs., simply having DEBUG set in the env)
ifdef DEBUGFLAGS
	CFLAGS:=$(DEBUG_CFLAGS)
endif

# Detect GCC version because reasons...
# (namely, GCC emitting an error instead of a warning on unknown -W options)
MOAR_WARNIGS:=0
# Tests heavily inspired from Linux's build system ;).
CC_IS_CLANG:=$(shell $(CC) -v 2>&1 | grep -q "clang version" && echo 1 || echo 0)
CC_VERSION:=$(shell printf "%02d%02d%02d" `echo __GNUC__ | $(CC) -E -x c - | tail -n 1` `echo __GNUC_MINOR__ | $(CC) -E -x c - | tail -n 1` `echo __GNUC_PATCHLEVEL__ | $(CC) -E -x c - | tail -n 1`)
# Detect Clang's SA, too...
ifeq "$(CC_IS_CLANG)" "0"
	ifeq "$(lastword $(subst /, ,$(CC)))" "ccc-analyzer"
		CC_IS_CLANG:=1
	endif
endif
ifeq "$(CC_IS_CLANG)" "1"
	# This is Clang
	MOAR_WARNIGS:=1
endif
ifeq "$(shell expr $(CC_VERSION) \>= 070000)" "1"
	# This is GCC >= 7
	MOAR_WARNIGS:=1
else
	# We may be silencing warnings unknown to older compilers via pragmas, so don't warn about those...
	EXTRA_CFLAGS+=-Wno-pragmas
endif

# Detect whether our TC is cross (at least as far as the target arch is concerned)
HOST_ARCH:=$(shell uname -m)
TARGET_ARCH:=$(shell $(CC) -dumpmachine 2>/dev/null)
CC_IS_CROSS:=0
# Host doesn't match target, assume it's a cross TC
ifeq (,$(findstring $(HOST_ARCH),$(TARGET_ARCH)))
	CC_IS_CROSS:=1
endif

ifndef DEBUG
	# Don't hobble GCC just for the sake of being interposable
	ifdef SHARED
		ifneq "$(CC_IS_CLANG)" "1"
			# Applies when building a shared library as well as just PIC in general.
			SHARED_CFLAGS+=-fno-semantic-interposition
		endif
	endif
	# Enable loop unrolling & vectorization in the hope it'll do something smart with our pixel loops
	EXTRA_CFLAGS+=-ftree-vectorize
	EXTRA_CFLAGS+=-funroll-loops
	# Graphite stuff (none of my TCs are built w/ graphite enabled, and it doesn't seem to have a noticeable impact anyway).
	#EXTRA_CFLAGS+=-fgraphite
	#EXTRA_CFLAGS+=-fgraphite-identity
	#EXTRA_CFLAGS+=-floop-nest-optimize
	#EXTRA_CFLAGS+=-floop-parallelize-all
	# More loop/vectorization tweaks
	#EXTRA_CFLAGS+=-ftree-loop-distribution -ftree-loop-im -ftree-loop-ivcanon -fivopts
	# (Extremely verbose) debug info about the auto-vectorization pass...
	# NOTE: Much more useful since GCC 9!
	#EXTRA_CFLAGS+=-fopt-info-vec-all=vecall.txt
	# Rather verbose debug info about unvectorized blocks/loops
	#EXTRA_CFLAGS+=-fopt-info-vec-missed=vecmissed.txt
	# Much less verbose info about successfully (or partially successfully) vectorized sections
	#EXTRA_CFLAGS+=-fopt-info-vec
	# When playing with GProf
	#EXTRA_CFLAGS+=-g -pg -fno-omit-frame-pointer
endif

# Enforce LTO if need be (utils won't link without it).
# I *highly* recommend *always* building the full project with LTO, though.
ifeq (,$(findstring flto,$(CFLAGS)))
	LTO_JOBS:=$(shell getconf _NPROCESSORS_ONLN 2> /dev/null || sysctl -n hw.ncpu 2> /dev/null || echo 1)
	LTO_CFLAGS:=-flto=$(LTO_JOBS) -fuse-linker-plugin
endif

# NOTE: We require C11 support (GCC >= 4.9, Clang >= 3.0), so enforce that on older compilers,
#       because otherwise the errors generated are highly unhelpful...
ifeq "$(CC_IS_CLANG)" "0"
	# NOTE: GCC 5.1.0 switched to gnu11, while GCC 6.1.0 switched to gnu++14, so only tweak older ones.
	#       We only care about C here, so enforce gnu11 on GCC < 5.1
	#       c.f., https://stackoverflow.com/q/14737104 for a nice recap.
	ifeq "$(shell expr $(CC_VERSION) \< 050100)" "1"
		EXTRA_CFLAGS+=-std=gnu11
	endif
endif

# Moar warnings!
ifeq "$(MOAR_WARNIGS)" "1"
	EXTRA_CFLAGS+=-Wall
	EXTRA_CFLAGS+=-Wextra -Wunused
	EXTRA_CFLAGS+=-Wformat=2
	EXTRA_CFLAGS+=-Wformat-signedness
	# NOTE: -Wformat-truncation=2 is still a tad too aggressive w/ GCC 9, so, tone it down to avoid false-positives...
	EXTRA_CFLAGS+=-Wformat-truncation=1
	# NOTE: This doesn't really play nice w/ FORTIFY, leading to an assload of false-positives, unless LTO is enabled
	ifeq (,$(findstring flto,$(CFLAGS)))
		# NOTE: GCC 9 is more verbose, so nerf that, too, when building w/o LTO on native systems...
		ifneq "$(CC_IS_CROSS)" "1"
			EXTRA_CFLAGS+=-Wno-stringop-truncation
		endif
	endif
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
	# And disable this, because it obviously doesn't play well with using goto to handle cleanup on error codepaths...
	EXTRA_CFLAGS+=-Wno-jump-misses-init
	# And just because that's annoying...
	ifeq "$(CC_IS_CLANG)" "1"
		EXTRA_CFLAGS+=-Wno-ignored-optimization-argument -Wno-unknown-warning-option
	endif
endif

# We need to build PIC to support running as/with a shared library
# NOTE: We should be safe with -fpic instead of -fPIC ;).
# And we also want to handle symbol visibility sanely...
ifdef SHARED
	SHARED_CFLAGS+=-fpic
	# The symbol visibility shenanigans only make sense for shared builds.
	# (Unless you *really* want that main symbol to be local in fbink_cmd in static builds ;)).
	SHARED_CFLAGS+=-fvisibility=hidden
	# And this ensures that only what needs to be public in the library will actually be.
	LIB_CFLAGS+=-DFBINK_SHAREDLIB
endif

# Assume we'll be safe to use by threaded applications...
EXTRA_CPPFLAGS+=-D_REENTRANT=1
# We're Linux-bound anyway...
EXTRA_CPPFLAGS+=-D_GNU_SOURCE

# Toggle Kindle support
ifdef KINDLE
	EXTRA_CPPFLAGS+=-DFBINK_FOR_KINDLE
endif
# Toggle Legacy Kindle support
ifdef LEGACY
	EXTRA_CPPFLAGS+=-DFBINK_FOR_LEGACY
endif
# Toggle Bq Cervantes support
ifdef CERVANTES
	EXTRA_CPPFLAGS+=-DFBINK_FOR_CERVANTES
endif
# Toggle generic Linux support
ifdef LINUX
	EXTRA_CPPFLAGS+=-DFBINK_FOR_LINUX
endif

# A version tag...
FBINK_VERSION:=$(shell git describe)
ifdef KINDLE
	LIB_CFLAGS+=-DFBINK_VERSION='"$(FBINK_VERSION) for Kindle"'
else
	ifdef CERVANTES
		LIB_CFLAGS+=-DFBINK_VERSION='"$(FBINK_VERSION) for Cervantes"'
	else
		ifdef LINUX
			LIB_CFLAGS+=-DFBINK_VERSION='"$(FBINK_VERSION) for Linux"'
		else
			LIB_CFLAGS+=-DFBINK_VERSION='"$(FBINK_VERSION) for Kobo"'
		endif
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
	LIBS+=-lfbink
else
	LIBS+=-l:libfbink.a
endif

# Pick up our vendored build of libunibreak, if requested
ifdef UNIBREAK
	EXTRA_LDFLAGS+=-LLibUniBreakBuild/src/.libs
	LIBS+=-l:libunibreak.a
endif

# And with our own rpath for standalone distribution
ifdef STANDALONE
	EXTRA_LDFLAGS+=-Wl,-rpath=/usr/local/fbink/lib
endif
# NOTE: Don't use in production, this was to help wrap my head around fb rotation experiments...
ifdef MATHS
	EXTRA_CPPFLAGS+=-DFBINK_WITH_MATHS_ROTA
endif
# NOTE: Despite attempts at using mostly GCC builtins, OpenType support needs lm
ifndef MINIMAL
	# NOTE: Here be dragons! Static linking bits of the glibc is usually considered a fairly terrible idea.
	ifdef STATIC_LIBM
		LIBS+=-l:libm.a
	else
		LIBS+=-lm
	endif
	# NOTE: We can optionally forcibly disable the NEON/SSE4 codepaths in QImageScale!
	#       Although, generally, the SIMD variants are a bit faster ;).
	#EXTRA_CPPFLAGS+=-DFBINK_QIS_NO_SIMD
endif

##
# Now that we're done fiddling with flags, let's build stuff!
LIB_SRCS:=fbink.c cutef8/utf8.c cutef8/dfa.c
# Jump through a few hoops to set a few libunibreak-specific CFLAGS to silence some warnings...
LIB_UB_SRCS:=libunibreak/src/linebreak.c libunibreak/src/linebreakdata.c libunibreak/src/unibreakdef.c libunibreak/src/linebreakdef.c
LIB_QT_SRCS:=qimagescale/qimagescale.c
# We don't need any of that in MINIMAL builds
ifdef MINIMAL
	ifndef OPENTYPE
		LIB_UB_SRCS:=
	endif
	ifndef IMAGE
		LIB_QT_SRCS:=
	endif
endif

CMD_SRCS:=fbink_cmd.c
BTN_SRCS:=button_scan_cmd.c
# Unless we're asking for a minimal build, include the Unscii fonts, too
ifdef MINIMAL
	EXTRA_CPPFLAGS+=-DFBINK_MINIMAL
else
	EXTRA_CPPFLAGS+=-DFBINK_WITH_FONTS
	EXTRA_CPPFLAGS+=-DFBINK_WITH_IMAGE
	EXTRA_CPPFLAGS+=-DFBINK_WITH_OPENTYPE
	# Connect button scanning is Kobo specific
	ifndef KINDLE
		ifndef CERVANTES
			ifndef LINUX
				WITH_BUTTON_SCAN:=True
				EXTRA_CPPFLAGS+=-DFBINK_WITH_BUTTON_SCAN
			endif
		endif
	endif
endif

# Manage modular MINIMAL builds...
ifdef MINIMAL
	# Support tweaking a MINIMAL build to still include extra bitmap fonts
	ifdef FONTS
		EXTRA_CPPFLAGS+=-DFBINK_WITH_FONTS
	endif

	# Support tweaking a MINIMAL build to still include image support
	ifdef IMAGE
		EXTRA_CPPFLAGS+=-DFBINK_WITH_IMAGE
	endif

	# Support tweaking a MINIMAL build to still include OpenType support
	ifdef OPENTYPE
		EXTRA_CPPFLAGS+=-DFBINK_WITH_OPENTYPE
		ifdef STATIC_LIBM
			LIBS+=-l:libm.a
		else
			LIBS+=-lm
		endif
	endif

	# Support tweaking a MINIMAL build to still include button scan support
	ifdef BUTTON_SCAN
		WITH_BUTTON_SCAN:=True
		EXTRA_CPPFLAGS+=-DFBINK_WITH_BUTTON_SCAN
	endif
endif

# How we handle our library creation
FBINK_SHARED_FLAGS:=-shared -Wl,-soname,libfbink.so.1
FBINK_SHARED_NAME_FILE:=libfbink.so.1.0.0
FBINK_SHARED_NAME:=libfbink.so
FBINK_SHARED_NAME_VER:=libfbink.so.1
FBINK_STATIC_FLAGS:=rc
FBINK_STATIC_NAME:=libfbink.a

default: all

SHAREDLIB_OBJS:=$(addprefix $(OUT_DIR)/shared/, $(LIB_SRCS:.c=.o))
UB_SHAREDLIB_OBJS:=$(addprefix $(OUT_DIR)/shared/, $(LIB_UB_SRCS:.c=.o))
QT_SHAREDLIB_OBJS:=$(addprefix $(OUT_DIR)/shared/, $(LIB_QT_SRCS:.c=.o))
STATICLIB_OBJS:=$(addprefix $(OUT_DIR)/static/, $(LIB_SRCS:.c=.o))
UB_STATICLIB_OBJS:=$(addprefix $(OUT_DIR)/static/, $(LIB_UB_SRCS:.c=.o))
QT_STATICLIB_OBJS:=$(addprefix $(OUT_DIR)/static/, $(LIB_QT_SRCS:.c=.o))
CMD_OBJS:=$(addprefix $(OUT_DIR)/, $(CMD_SRCS:.c=.o))
BTN_OBJS:=$(addprefix $(OUT_DIR)/, $(BTN_SRCS:.c=.o))

# Silence a few warnings, only when specifically compiling libunibreak...
# c.f., https://stackoverflow.com/q/1305665
UNIBREAK_CFLAGS := -Wno-conversion -Wno-sign-conversion -Wno-suggest-attribute=pure
$(UB_SHAREDLIB_OBJS): QUIET_CFLAGS := $(UNIBREAK_CFLAGS)
$(UB_STATICLIB_OBJS): QUIET_CFLAGS := $(UNIBREAK_CFLAGS)

# Shared lib
$(OUT_DIR)/shared/%.o: %.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(QUIET_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) -o $@ -c $<

# Static lib
$(OUT_DIR)/static/%.o: %.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(QUIET_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) -o $@ -c $<

# CLI front-end
$(OUT_DIR)/%.o: %.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) -o $@ -c $<

outdir:
	mkdir -p $(OUT_DIR)/shared/cutef8 $(OUT_DIR)/static/cutef8 $(OUT_DIR)/shared/libunibreak/src $(OUT_DIR)/static/libunibreak/src $(OUT_DIR)/shared/qimagescale $(OUT_DIR)/static/qimagescale

# Make absolutely sure we create our output directories first, even with unfortunate // timings!
# c.f., https://www.gnu.org/software/make/manual/html_node/Prerequisite-Types.html#Prerequisite-Types
$(SHAREDLIB_OBJS): | outdir
$(UB_SHAREDLIB_OBJS): | outdir
$(QT_SHAREDLIB_OBJS): | outdir
$(STATICLIB_OBJS): | outdir
$(UB_STATICLIB_OBJS): | outdir
$(QT_STATICLIB_OBJS): | outdir
$(CMD_OBJS): | outdir
$(BTN_OBJS): | outdir

all: static

ifdef UNIBREAK
staticlib: libunibreak.built $(STATICLIB_OBJS)
	$(AR) $(FBINK_STATIC_FLAGS) $(OUT_DIR)/$(FBINK_STATIC_NAME) $(STATICLIB_OBJS)
	$(RANLIB) $(OUT_DIR)/$(FBINK_STATIC_NAME)

sharedlib: libunibreak.built $(SHAREDLIB_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(FBINK_SHARED_FLAGS) -o$(OUT_DIR)/$(FBINK_SHARED_NAME_FILE) $(SHAREDLIB_OBJS)
	ln -sf $(FBINK_SHARED_NAME_FILE) $(OUT_DIR)/$(FBINK_SHARED_NAME)
	ln -sf $(FBINK_SHARED_NAME_FILE) $(OUT_DIR)/$(FBINK_SHARED_NAME_VER)
else
staticlib: $(STATICLIB_OBJS) $(UB_STATICLIB_OBJS) $(QT_STATICLIB_OBJS)
	$(AR) $(FBINK_STATIC_FLAGS) $(OUT_DIR)/$(FBINK_STATIC_NAME) $(STATICLIB_OBJS) $(UB_STATICLIB_OBJS) $(QT_STATICLIB_OBJS)
	$(RANLIB) $(OUT_DIR)/$(FBINK_STATIC_NAME)

sharedlib: $(SHAREDLIB_OBJS) $(UB_SHAREDLIB_OBJS) $(QT_SHAREDLIB_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(FBINK_SHARED_FLAGS) -o$(OUT_DIR)/$(FBINK_SHARED_NAME_FILE) $(SHAREDLIB_OBJS) $(UB_SHAREDLIB_OBJS) $(QT_SHAREDLIB_OBJS)
	ln -sf $(FBINK_SHARED_NAME_FILE) $(OUT_DIR)/$(FBINK_SHARED_NAME)
	ln -sf $(FBINK_SHARED_NAME_FILE) $(OUT_DIR)/$(FBINK_SHARED_NAME_VER)
endif

ifdef WITH_BUTTON_SCAN
staticbin: $(OUT_DIR)/$(FBINK_STATIC_NAME) $(CMD_OBJS) $(BTN_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/fbink $(CMD_OBJS) $(LIBS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/button_scan $(BTN_OBJS) $(LIBS)
else
staticbin: $(OUT_DIR)/$(FBINK_STATIC_NAME) $(CMD_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/fbink $(CMD_OBJS) $(LIBS)
endif

ifdef WITH_BUTTON_SCAN
sharedbin: $(OUT_DIR)/$(FBINK_SHARED_NAME_FILE) $(CMD_OBJS) $(BTN_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/fbink $(CMD_OBJS) $(LIBS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/button_scan $(BTN_OBJS) $(LIBS)
else
sharedbin: $(OUT_DIR)/$(FBINK_SHARED_NAME_FILE) $(CMD_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/fbink $(CMD_OBJS) $(LIBS)
endif

striplib: $(OUT_DIR)/$(FBINK_SHARED_NAME_FILE)
	$(STRIP) --strip-unneeded $(OUT_DIR)/$(FBINK_SHARED_NAME_FILE)

# NOTE: Unless you *really* need to, I don't recommend stripping LTO archives,
#       strip a binary linked against the untouched archive instead.
#       (c.f., https://sourceware.org/bugzilla/show_bug.cgi?id=21479)
#       (i.e., strip doesn't know what to do with LTO archives, has no -plugin wrapper, and as such, breaks the symbol table).
striparchive: $(OUT_DIR)/$(FBINK_STATIC_NAME)
	$(STRIP) --strip-unneeded $(OUT_DIR)/$(FBINK_STATIC_NAME)
	$(RANLIB) $(OUT_DIR)/$(FBINK_STATIC_NAME)

ifdef WITH_BUTTON_SCAN
stripbin: $(OUT_DIR)/fbink
	$(STRIP) --strip-unneeded $(OUT_DIR)/fbink
	$(STRIP) --strip-unneeded $(OUT_DIR)/button_scan
else
stripbin: $(OUT_DIR)/fbink
	$(STRIP) --strip-unneeded $(OUT_DIR)/fbink
endif

ifdef LINUX
utils: | outdir
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) $(LTO_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/doom utils/doom.c -lrt
	$(STRIP) --strip-unneeded $(OUT_DIR)/doom
else
utils: | outdir
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) $(LTO_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/rota utils/rota.c
	$(STRIP) --strip-unneeded $(OUT_DIR)/rota
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) $(LTO_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/fbdepth utils/fbdepth.c
	$(STRIP) --strip-unneeded $(OUT_DIR)/fbdepth
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) $(LTO_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/doom utils/doom.c -lrt
	$(STRIP) --strip-unneeded $(OUT_DIR)/doom
endif

dump: static
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) $(LTO_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/dump utils/dump.c $(LIBS)
	$(STRIP) --strip-unneeded $(OUT_DIR)/dump

strip: static
	$(MAKE) stripbin

debug:
	$(MAKE) static DEBUG=true DEBUGFLAGS=true

static:
	$(MAKE) staticlib
	$(MAKE) staticbin

# NOTE: This one may be a bit counter-intuitive... It's to build a static library built like if it were shared (i.e., PIC),
#       because apparently that's a requirement for FFI in some high-level languages (i.e., Go; c.f., #7)
pic:
	$(MAKE) static SHARED=true

shared:
	$(MAKE) sharedlib SHARED=true
	$(MAKE) sharedbin SHARED=true STANDALONE=true

release: shared
	$(MAKE) striplib
	$(MAKE) stripbin

kindle:
	$(MAKE) strip KINDLE=true

legacy:
	$(MAKE) strip KINDLE=true LEGACY=true

linux:
	$(MAKE) strip LINUX=true

cervantes:
	$(MAKE) strip CERVANTES=true
	$(CURDIR)/tools/do_debian_package.sh $(OUT_DIR) armel

libunibreak.built:
	mkdir -p LibUniBreakBuild
	cd libunibreak && \
	env NOCONFIGURE=1 ./autogen.sh
	cd LibUniBreakBuild && \
	env CPPFLAGS="$(CPPFLAGS) $(EXTRA_CPPFLAGS)" \
	CFLAGS="$(CFLAGS) $(EXTRA_CFLAGS) $(UNIBREAK_CFLAGS)" \
	LDFLAGS="$(LDFLAGS)" \
	../libunibreak/configure \
	$(if $(CROSS_TC),--host=$(CROSS_TC),) \
	--enable-static \
	--disable-shared \
	$(if $(SHARED),--with-pic=yes,)
	cd LibUniBreakBuild && \
	$(MAKE)
	touch libunibreak.built

armcheck:
ifeq (,$(findstring arm-,$(CC)))
	$(error You forgot to setup a cross TC, you dummy!)
endif

kobo: armcheck release
	mkdir -p Kobo/usr/local/fbink/bin Kobo/usr/bin Kobo/usr/local/fbink/lib Kobo/usr/local/fbink/include
	cp -av $(CURDIR)/Release/fbink Kobo/usr/local/fbink/bin
	ln -sf /usr/local/fbink/bin/fbink Kobo/usr/bin/fbink
	cp -av $(CURDIR)/Release/button_scan Kobo/usr/local/fbink/bin
	ln -sf /usr/local/fbink/bin/button_scan Kobo/usr/bin/button_scan
	cp -av $(CURDIR)/Release/$(FBINK_SHARED_NAME_FILE) Kobo/usr/local/fbink/lib
	ln -sf $(FBINK_SHARED_NAME_FILE) Kobo/usr/local/fbink/lib/$(FBINK_SHARED_NAME)
	ln -sf $(FBINK_SHARED_NAME_FILE) Kobo/usr/local/fbink/lib/$(FBINK_SHARED_NAME_VER)
	cp -av $(CURDIR)/fbink.h Kobo/usr/local/fbink/include
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

libunibreakclean:
	cd libunibreak && \
	git reset --hard

clean:
	rm -rf Kobo/
	rm -rf Release/*.a
	rm -rf Release/*.so*
	rm -rf Release/shared/*.o
	rm -rf Release/shared/cutef8/*.o
	rm -rf Release/shared/libunibreak/src/*.o
	rm -rf Release/shared/qimagescale/*.o
	rm -rf Release/shared/utf8
	rm -rf Release/static/*.o
	rm -rf Release/static/cutef8/*.o
	rm -rf Release/static/libunibreak/src/*.o
	rm -rf Release/static/qimagescale/*.o
	rm -rf Release/static/utf8
	rm -rf Release/*.o
	rm -rf Release/fbink
	rm -rf Release/button_scan
	rm -rf Release/rota
	rm -rf Release/fbdepth
	rm -rf Release/doom
	rm -rf Release/dump
	rm -rf Debug/*.a
	rm -rf Debug/*.so*
	rm -rf Debug/shared/*.o
	rm -rf Debug/shared/cutef8/*.o
	rm -rf Debug/shared/libunibreak/src/*.o
	rm -rf Debug/shared/qimagescale/*.o
	rm -rf Debug/shared/utf8
	rm -rf Debug/static/*.o
	rm -rf Debug/static/cutef8/*.o
	rm -rf Debug/static/libunibreak/src/*.o
	rm -rf Debug/static/qimagescale/*.o
	rm -rf Debug/static/utf8
	rm -rf Debug/*.o
	rm -rf Debug/fbink
	rm -rf Debug/button_scan
	rm -rf Debug/rota
	rm -rf Debug/fbdepth
	rm -rf Debug/doom
	rm -rf Debug/dump

distclean: clean libunibreakclean
	rm -rf LibUniBreakBuild
	rm -rf libunibreak.built

.PHONY: default outdir all staticlib sharedlib static shared striplib striparchive stripbin strip debug static pic shared release kindle legacy cervantes linux armcheck kobo libunibreakclean utils dump clean distclean
