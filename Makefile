# SPDX-License-Identifier: GPL-3.0-or-later
#
# Pickup our cross-toolchains automatically...
# c.f., http://trac.ak-team.com/trac/browser/niluje/Configs/trunk/Kindle/Misc/x-compile.sh
#       https://github.com/NiLuJe/crosstool-ng
#       https://github.com/koreader/koxtoolchain
# NOTE: We want the "bare" variant of the TC env, to make sure we vendor the right stuff...
#       i.e., source ~SVN/Configs/trunk/Kindle/Misc/x-compile.sh kobo env bare
ifdef CROSS_TC
	# NOTE: If we have a CROSS_TC toolchain w/ CC set to Clang,
	#       assume we know what we're doing, and that everything is setup the right way already (i.e., via x-compile.sh tc env clang)...
	ifneq "$(CC)" "clang"
		CC:=$(CROSS_TC)-gcc
		CXX:=$(CROSS_TC)-g++
		STRIP:=$(CROSS_TC)-strip
		# NOTE: This relies on GCC plugins!
		#       Enforce AR & RANLIB to point to their real binary and not the GCC wrappers if your TC doesn't support that!
		AR:=$(CROSS_TC)-gcc-ar
		RANLIB:=$(CROSS_TC)-gcc-ranlib
	endif
else ifdef CROSS_COMPILE
	CC:=$(CROSS_COMPILE)cc
	CXX:=$(CROSS_COMPILE)cxx
	STRIP:=$(CROSS_COMPILE)strip
	AR:=$(CROSS_COMPILE)gcc-ar
	RANLIB:=$(CROSS_COMPILE)gcc-ranlib
else
	CC?=gcc
	CXX?=g++
	STRIP?=strip
	AR?=gcc-ar
	RANLIB?=gcc-ranlib
endif

# For use in recursive calls we don't want `make -n` to follow through.
SUBMAKE = $(MAKE)

DEBUG_CFLAGS:=-Og -fno-omit-frame-pointer -pipe -g
# Fallback CFLAGS, we honor the env first and foremost!
OPT_CFLAGS:=-O2 -fomit-frame-pointer -pipe

ifdef DEBUG
	OUT_DIR:=Debug
	CFLAGS?=$(DEBUG_CFLAGS)
	CXXFLAGS?=$(DEBUG_CFLAGS)
	EXTRA_CPPFLAGS+=-DDEBUG
	STRIP:=true
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

# Try to use sane defaults for DESTDIR, while still playing nice with PMS
DESTDIR?=/usr/local
INCDIR:=$(DESTDIR)/include/fbink
BINDIR:=$(DESTDIR)/bin
LIBDIR:=$(DESTDIR)/lib
DOCDIR:=$(DESTDIR)/share/doc/fbink

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
TARGET_ARCH:=$(shell $(CC) $(CFLAGS) -dumpmachine 2>/dev/null)
CC_IS_CROSS:=0
# Host doesn't match target, assume it's a cross TC
ifeq (,$(findstring $(HOST_ARCH),$(TARGET_ARCH)))
	CC_IS_CROSS:=1
endif
# Detect musl because some cross musl TCs are special snowflakes without plugin support,
# and most of the utils won't build without LTO and linker plugins...
CC_IS_MUSL:=0
ifeq (musl,$(findstring musl,$(TARGET_ARCH)))
	CC_IS_MUSL:=1

	# NOTE: Can't be tab-indented or make thinks it's part of a recipe, for some reason...
        $(info )
        $(info /!\)
        $(warning Detected a musl TC: it *may* be built without linker plugins support, which are a hard dependency of the utils target!)
        $(info /!\)
        $(info )
endif

ifndef DEBUG
	# Don't hobble GCC just for the sake of being interposable
	ifeq "$(CC_IS_CLANG)" "0"
		# Fun fact: apparently the default on Clang ;).
		EXTRA_CFLAGS+=-fno-semantic-interposition
	endif
	# Enable loop unrolling & vectorization in the hope it'll do something smart with our pixel loops
	EXTRA_CFLAGS+=-ftree-vectorize
	EXTRA_CFLAGS+=-funroll-loops
	# Always match GCC >= 10 new default
	ifeq "$(CC_IS_CLANG)" "0"
		EXTRA_CFLAGS+=-fno-common
	endif
	##
	# Graphite stuff (none of my TCs are built w/ graphite enabled, and it doesn't seem to have a noticeable impact anyway).
	ifeq "$(CC_IS_CLANG)" "0"
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
		# When I just want my debug prints in a release build ;p.
		#EXTRA_CFLAGS+=-DDEBUG
	endif
	##
	# Clang's version of optimization reports
	# c.f., https://clang.llvm.org/docs/UsersManual.html#options-to-emit-optimization-reports)
	#    &  https://llvm.org/docs/Vectorizers.html#diagnostics
	ifeq "$(CC_IS_CLANG)" "1"
		# NOTE: ThinLTO appears to be inhibiting those reports from the vectorizer...
		#EXTRA_CFLAGS+=-Rpass-analysis=loop-vectorize -gline-tables-only -gcolumn-info -fsave-optimization-record
		# Here be dragons ;).
		#EXTRA_CFLAGS+=-Rpass-analysis=loop-* -mllvm -enable-loop-distribute
	endif
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
	# NOTE: -Wformat-truncation=2 is still a tad too aggressive w/ GCC 14, so, tone it down to avoid false-positives...
	EXTRA_CFLAGS+=-Wformat-truncation=1
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

# NOTE: When targeting armv7 with GCC > 7, if you *really* want to squeeze the last bit of performance out of the compiler,
#       especially if you primarily deal with RGBA PNGs, consider enforcing ARM mode instead of Thumb mode,
#       because there's a bit of a performance regression around PNG decompression in stbi with GCC > 7...
#ifeq "$(CC_IS_CLANG)" "0"
#	ifeq (arm-,$(findstring arm-,$(TARGET_ARCH)))
#		ifeq "$(shell expr $(CC_VERSION) \>= 080000)" "1"
#			EXTRA_CFLAGS+=-marm
#		endif
#	endif
#endif

# We need to build PIC to support running as/with a shared library
# NOTE: We should be safe with -fpic instead of -fPIC ;).
# And we also want to handle symbol visibility sanely...
SHARED_CFLAGS+=-fpic
# The symbol visibility shenanigans only make sense for shared builds.
# (Unless you *really* want that main symbol to be local in fbink_cmd in static builds ;)).
SHARED_LIB_CFLAGS+=-fvisibility=hidden
# And this ensures that only what needs to be public in the library will actually be.
SHARED_LIB_CFLAGS+=-DFBINK_SHAREDLIB

# Assume we'll be safe to use by threaded applications...
EXTRA_CPPFLAGS+=-D_REENTRANT=1
# We're Linux-bound anyway...
EXTRA_CPPFLAGS+=-D_GNU_SOURCE

# That target is a bit of a hack, so the preprocessor needs to be aware of it to deal with the fallout...
INPUT_LIB_CFLAGS:=-DFBINK_INPUT_LIB
# We also need to silence a whole load of -Wunused-function warnings because of that...
INPUT_LIB_CFLAGS+=-Wno-unused-function
# We also want LTO enabled for better DCE...
INPUT_LIB_CFLAGS+=$(LTO_CFLAGS)

# Backward compatibility shenanigan: before FBINK_FOR_KOBO was implemented, we assumed KOBO was the default/fallback platform.
# Keep honoring that.
ifndef LINUX
ifndef CERVANTES
ifndef LEGACY
ifndef KINDLE
ifndef REMARKABLE
ifndef POCKETBOOK
	KOBO=true
endif
endif
endif
endif
endif
endif

# Toggle Kindle support
ifdef KINDLE
	TARGET_CPPFLAGS+=-DFBINK_FOR_KINDLE
endif
# Toggle Legacy Kindle support
ifdef LEGACY
	TARGET_CPPFLAGS+=-DFBINK_FOR_LEGACY
endif
# Toggle Bq Cervantes support
ifdef CERVANTES
	TARGET_CPPFLAGS+=-DFBINK_FOR_CERVANTES
endif
# Toggle generic Linux support
ifdef LINUX
	TARGET_CPPFLAGS+=-DFBINK_FOR_LINUX
endif
# Toggle Kobo support
ifdef KOBO
	TARGET_CPPFLAGS+=-DFBINK_FOR_KOBO
endif
# Toggle reMarkable support
ifdef REMARKABLE
	TARGET_CPPFLAGS+=-DFBINK_FOR_REMARKABLE
endif
# Toggle PocketBook support
ifdef POCKETBOOK
	TARGET_CPPFLAGS+=-DFBINK_FOR_POCKETBOOK
endif

# And that should definitely be honored by everything, so, add it to EXTRA_CPPFLAGS
EXTRA_CPPFLAGS+=$(TARGET_CPPFLAGS)

# A version tag...
# NOTE: Don't redirect stderr, so we can get an idea of what happened when things go wrong...
FBINK_VERSION:=$(shell git describe || git rev-parse --short HEAD || cat VERSION)
# Only use it if we got something useful...
ifdef FBINK_VERSION
	LIB_CFLAGS+=-DFBINK_VERSION='"$(FBINK_VERSION)"'
else
	# Just so we don't use an empty var down the line...
	FBINK_VERSION:=dev
endif

# NOTE: Always use as-needed to avoid unecessary DT_NEEDED entries :)
LDFLAGS?=-Wl,--as-needed

# And we want to link against our own library ;).
EXTRA_LDFLAGS+=-L$(OUT_DIR)

LIBS_FOR_SHARED+=-lfbink
LIBS_FOR_STATIC+=-l:libfbink.a

# We also want to be able to optionally enforce PIC on static builds...
ifdef PIC
	EXTRA_CFLAGS+=$(SHARED_CFLAGS)
endif

# Pick up our vendored build of libunibreak, if requested
ifdef UNIBREAK
	LIB_LDFLAGS+=-Llibunibreak-staged/src/.libs
	UNIBREAK_LIBS:=-l:libunibreak.a
	# And also vendor it inside our shared library.
	# I'm not necessarily a fan of this approach in principle, but it ensures stuff Just Works for library users.
	# Plus, the default build already does that by picking up the object files manually...
	SHARED_LIBS+=$(UNIBREAK_LIBS)
	# NOTE: We do the same for static builds, but the magic happens via partial linking in the staticlib target...
	STATIC_LIBS+=libunibreak-staged/src/.libs/libunibreak.a
endif

# Same for libi2c, on Kobo
ifdef KOBO
	I2C_CPPFLAGS+=-Ilibi2c-staged/include
	I2C_LDFLAGS+=-Llibi2c-staged/lib
	I2C_LIBS:=-l:libi2c.a
	LIB_CPPFLAGS+=$(I2C_CPPFLAGS)
	LIB_LDFLAGS+=$(I2C_LDFLAGS)
	SHARED_LIBS+=$(I2C_LIBS)
	STATIC_LIBS+=libi2c-staged/lib/libi2c.a
endif

# Pick up our vendored build of libevdev (for the PoC that relies on it)
EVDEV_CPPFLAGS:=-Ilibevdev-staged/include/libevdev-1.0
EVDEV_LDFLAGS:=-Llibevdev-staged/lib
EVDEV_LIBS:=-l:libevdev.a

# And with our own rpath for standalone distribution
ifdef STANDALONE
	EXTRA_LDFLAGS+=-Wl,-rpath=/usr/local/fbink/lib
endif
# NOTE: Don't use in production, this was to help wrap my head around fb rotation experiments...
ifdef MATHS
	FEATURES_CPPFLAGS+=-DFBINK_WITH_MATHS_ROTA
endif
# NOTE: Despite attempts at using mostly GCC builtins, OpenType support needs lm
ifndef MINIMAL
	# NOTE: Here be dragons! Static linking bits of the glibc is usually considered a fairly terrible idea.
	ifdef STATIC_LIBM
		LIBS+=-l:libm.a
		SHARED_LIBS+=-l:libm.a
	else
		LIBS+=-lm
		SHARED_LIBS+=-lm
	endif
	# NOTE: We can optionally forcibly disable the NEON/SSE4 codepaths in QImageScale!
	#       Although, generally, the SIMD variants are a bit faster ;).
	#FEATURES_CPPFLAGS+=-DFBINK_QIS_NO_SIMD
endif

# We need libdl on PocketBook in order to dlopen InkView...
ifdef POCKETBOOK
	LIBS+=-ldl
	SHARED_LIBS+=-ldl
	UTILS_LIBS+=-ldl
endif

##
# Now that we're done fiddling with flags, let's build stuff!
LIB_SRCS:=fbink.c cutef8/utf8.c cutef8/dfa.c
LIB_INPUT_SRCS:=fbink_input_scan.c
# Jump through a few hoops to set a few libunibreak-specific CFLAGS to silence some warnings...
LIB_UB_SRCS:=libunibreak/src/linebreak.c libunibreak/src/linebreakdata.c libunibreak/src/unibreakdef.c libunibreak/src/linebreakdef.c libunibreak/src/eastasianwidthdef.c
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
# Unless we're asking for a minimal build, include the other features, except for Unifont.
ifdef MINIMAL
	FEATURES_CPPFLAGS+=-DFBINK_MINIMAL
else
	FEATURES_CPPFLAGS+=-DFBINK_WITH_DRAW
	FEATURES_CPPFLAGS+=-DFBINK_WITH_BITMAP
	FEATURES_CPPFLAGS+=-DFBINK_WITH_FONTS
	FEATURES_CPPFLAGS+=-DFBINK_WITH_IMAGE
	FEATURES_CPPFLAGS+=-DFBINK_WITH_OPENTYPE
	FEATURES_CPPFLAGS+=-DFBINK_WITH_INPUT
	# Unifont is *always* optional, because it'll add almost 2MB to the binary size!
	ifdef UNIFONT
		FEATURES_CPPFLAGS+=-DFBINK_WITH_UNIFONT
	endif
	# button_scan has been deprecated, so it's now optional
	ifdef KOBO
		ifdef BUTTON_SCAN
			WITH_BUTTON_SCAN:=True
			FEATURES_CPPFLAGS+=-DFBINK_WITH_BUTTON_SCAN
		endif
	endif
endif

# Manage modular MINIMAL builds...
ifdef MINIMAL
	# Support tweaking a MINIMAL build to still include extra bitmap fonts
	ifdef FONTS
		# Make sure we actually have drawing support
		ifndef DRAW
			DRAW:=1
		endif
		# Make sure we actually have fixed-cell support
		ifndef BITMAP
			BITMAP:=1
		endif
		FEATURES_CPPFLAGS+=-DFBINK_WITH_FONTS
		# As well as, optionally, the full Unifont...
		ifdef UNIFONT
			FEATURES_CPPFLAGS+=-DFBINK_WITH_UNIFONT
		endif
	endif

	# Support tweaking a MINIMAL build to still include fixed-cell font rendering
	ifdef BITMAP
		# Make sure we actually have drawing support
		ifndef DRAW
			DRAW:=1
		endif
		FEATURES_CPPFLAGS+=-DFBINK_WITH_BITMAP
	endif

	# Support tweaking a MINIMAL build to still include image support
	ifdef IMAGE
		# Make sure we actually have drawing support
		ifndef DRAW
			DRAW:=1
		endif
		FEATURES_CPPFLAGS+=-DFBINK_WITH_IMAGE
	endif

	# Support tweaking a MINIMAL build to still include OpenType support
	ifdef OPENTYPE
		# Make sure we actually have drawing support
		ifndef DRAW
			DRAW:=1
		endif
		FEATURES_CPPFLAGS+=-DFBINK_WITH_OPENTYPE
		ifdef STATIC_LIBM
			LIBS+=-l:libm.a
			SHARED_LIBS+=-l:libm.a
		else
			LIBS+=-lm
			SHARED_LIBS+=-lm
		endif
	endif

	# Support tweaking a MINIMAL build to still include button scan support
	ifdef KOBO
		ifdef BUTTON_SCAN
			# Make sure we actually have drawing support
			ifndef DRAW
				DRAW:=1
			endif
			WITH_BUTTON_SCAN:=True
			FEATURES_CPPFLAGS+=-DFBINK_WITH_BUTTON_SCAN
		endif
	endif

	# Support tweaking a MINIMAL build to still include drawing primitives
	ifdef DRAW
		FEATURES_CPPFLAGS+=-DFBINK_WITH_DRAW
	endif

	# Support tweaking a MINIMAL build to still include input utilities
	ifdef INPUT
		FEATURES_CPPFLAGS+=-DFBINK_WITH_INPUT
	endif
endif

# On the other hand, we want to enforce MINIMAL features for the tools that don't link against FBInk,
# but instead piggyback on the internal API via fbink.c + LTO...
TOOLS_CPPFLAGS+=-DFBINK_MINIMAL
# On Kobo, fbink_rota_quirks.h needs access to the i2c-tools headers
TOOLS_CPPFLAGS+=$(I2C_CPPFLAGS)
# Except for doom, because it needs Image support...
DOOM_CPPFLAGS:=$(TOOLS_CPPFLAGS)
DOOM_CPPFLAGS+=-DFBINK_WITH_DRAW -DFBINK_WITH_IMAGE

# We also want matching feature sets for our frozen minimal builds for the tools that *do* link against such a static lib
TINIER_FEATURES:=-DFBINK_MINIMAL
TINYISH_FEATURES:=-DFBINK_MINIMAL -DFBINK_WITH_INPUT
TINY_FEATURES:=-DFBINK_MINIMAL -DFBINK_WITH_DRAW -DFBINK_WITH_INPUT
SMALL_FEATURES:=-DFBINK_MINIMAL -DFBINK_WITH_DRAW -DFBINK_WITH_BITMAP -DFBINK_WITH_IMAGE

# How we handle our library creation
FBINK_SHARED_LDFLAGS:=-shared -Wl,-soname,libfbink.so.1
FBINK_SHARED_NAME_FILE:=libfbink.so.1.0.0
FBINK_SHARED_NAME:=libfbink.so
FBINK_SHARED_NAME_VER:=libfbink.so.1
FBINK_STATIC_AR_OPTS:=rc
FBINK_STATIC_NAME:=libfbink.a
FBINK_PARTIAL_GCC_OPTS:=-r
FBINK_PARTIAL_NAME:=libfbink.o
FBINK_PARTIAL_LDFLAGS:=-Wl,--whole-archive -nostdlib
# For the standalone libfbink_input variant
FBINK_INPUT_SHARED_LDFLAGS:=-shared -Wl,-soname,libfbink_input.so.1
FBINK_INPUT_SHARED_NAME_FILE:=libfbink_input.so.1.0.0
FBINK_INPUT_SHARED_NAME:=libfbink_input.so
FBINK_INPUT_SHARED_NAME_VER:=libfbink_input.so.1
FBINK_INPUT_STATIC_NAME:=libfbink_input.a

default: all

SHAREDLIB_OBJS:=$(addprefix $(OUT_DIR)/shared/, $(LIB_SRCS:.c=.o))
INPUT_SHAREDLIB_OBJS:=$(addprefix $(OUT_DIR)/shared/, $(LIB_INPUT_SRCS:.c=.o))
UB_SHAREDLIB_OBJS:=$(addprefix $(OUT_DIR)/shared/, $(LIB_UB_SRCS:.c=.o))
QT_SHAREDLIB_OBJS:=$(addprefix $(OUT_DIR)/shared/, $(LIB_QT_SRCS:.c=.o))
STATICLIB_OBJS:=$(addprefix $(OUT_DIR)/static/, $(LIB_SRCS:.c=.o))
INPUT_STATICLIB_OBJS:=$(addprefix $(OUT_DIR)/static/, $(LIB_INPUT_SRCS:.c=.o))
UB_STATICLIB_OBJS:=$(addprefix $(OUT_DIR)/static/, $(LIB_UB_SRCS:.c=.o))
QT_STATICLIB_OBJS:=$(addprefix $(OUT_DIR)/static/, $(LIB_QT_SRCS:.c=.o))
CMD_OBJS:=$(addprefix $(OUT_DIR)/, $(CMD_SRCS:.c=.o))
BTN_OBJS:=$(addprefix $(OUT_DIR)/, $(BTN_SRCS:.c=.o))

# Silence a few warnings, only when specifically compiling libunibreak...
# c.f., https://stackoverflow.com/q/1305665
UNIBREAK_CFLAGS:=-Wno-conversion -Wno-cast-align -Wno-suggest-attribute=pure
$(UB_SHAREDLIB_OBJS): QUIET_CFLAGS:=$(UNIBREAK_CFLAGS)
$(UB_STATICLIB_OBJS): QUIET_CFLAGS:=$(UNIBREAK_CFLAGS)

# And when building libfbink_input standalone
$(INPUT_SHAREDLIB_OBJS): QUIET_CFLAGS:=$(INPUT_LIB_CFLAGS)
$(INPUT_STATICLIB_OBJS): QUIET_CFLAGS:=$(INPUT_LIB_CFLAGS)

# Silence a few warnings when building libi2c
I2C_CFLAGS:=-Wno-sign-conversion

# Ditto for libevdev
EVDEV_CFLAGS:=-Wno-conversion -Wno-sign-conversion -Wno-undef -Wno-vla-parameter -Wno-format -Wno-null-dereference -Wno-bad-function-cast -Wno-inline
# A random -Werror is injected in Debug builds...
EVDEV_CFLAGS+=-Wno-suggest-attribute=pure -Wno-suggest-attribute=const -Wno-padded
# And when *linking* libevdev (w/ LTO)
EVDEV_LDFLAGS+=-Wno-null-dereference

# Don't pollute our own exported symbols with vendored libraries'
UNIBREAK_CFLAGS+=-fvisibility=hidden
I2C_CFLAGS+=-fvisibility=hidden
EVDEV_CFLAGS+=-fvisibility=hidden

# Shared lib
$(OUT_DIR)/shared/%.o: %.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(LIB_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(QUIET_CFLAGS) $(SHARED_CFLAGS) $(SHARED_LIB_CFLAGS) $(LIB_CFLAGS) -o $@ -c $<

# Static lib
$(OUT_DIR)/static/%.o: %.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(LIB_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(QUIET_CFLAGS) $(LIB_CFLAGS) -o $@ -c $<

# CLI front-end
$(OUT_DIR)/%.o: %.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<

outdir: $(OUT_DIR)/shared/cutef8/ $(OUT_DIR)/static/cutef8/ $(OUT_DIR)/shared/libunibreak/src/ $(OUT_DIR)/static/libunibreak/src/ $(OUT_DIR)/shared/qimagescale/ $(OUT_DIR)/static/qimagescale/

$(OUT_DIR)/%/:
	mkdir -p "$@"

# Make absolutely sure we create our output directories first, even with unfortunate // timings!
# c.f., https://www.gnu.org/software/make/manual/html_node/Prerequisite-Types.html#Prerequisite-Types
$(SHAREDLIB_OBJS): libi2c.built | outdir
$(INPUT_SHAREDLIB_OBJS): libi2c.built | outdir
$(UB_SHAREDLIB_OBJS): | outdir
$(QT_SHAREDLIB_OBJS): | outdir
$(STATICLIB_OBJS): libi2c.built | outdir
$(INPUT_STATICLIB_OBJS): libi2c.built | outdir
$(UB_STATICLIB_OBJS): | outdir
$(QT_STATICLIB_OBJS): | outdir
$(CMD_OBJS): | outdir
$(BTN_OBJS): | outdir

all: static

ifdef UNIBREAK
# Plain FBInk archive
$(OUT_DIR)/static/$(FBINK_STATIC_NAME): $(STATICLIB_OBJS) $(QT_STATICLIB_OBJS) libunibreak.built
	$(AR) $(FBINK_STATIC_AR_OPTS) $@ $(STATICLIB_OBJS) $(QT_STATICLIB_OBJS)

$(OUT_DIR)/$(FBINK_SHARED_NAME_FILE): $(SHAREDLIB_OBJS) $(QT_SHAREDLIB_OBJS) libunibreak.built
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(SHARED_LIB_CFLAGS) $(LIB_CFLAGS) $(LDFLAGS) $(LIB_LDFLAGS) $(EXTRA_LDFLAGS) $(FBINK_SHARED_LDFLAGS) -o $@ $(SHAREDLIB_OBJS) $(QT_SHAREDLIB_OBJS) $(SHARED_LIBS)
else
# Plain FBInk archive
$(OUT_DIR)/static/$(FBINK_STATIC_NAME): $(STATICLIB_OBJS) $(UB_STATICLIB_OBJS) $(QT_STATICLIB_OBJS)
	$(AR) $(FBINK_STATIC_AR_OPTS) $@ $^

$(OUT_DIR)/$(FBINK_SHARED_NAME_FILE): $(SHAREDLIB_OBJS) $(UB_SHAREDLIB_OBJS) $(QT_SHAREDLIB_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(SHARED_LIB_CFLAGS) $(LIB_CFLAGS) $(LDFLAGS) $(LIB_LDFLAGS) $(EXTRA_LDFLAGS) $(FBINK_SHARED_LDFLAGS) -o $@ $^ $(SHARED_LIBS)
endif

# No partial linking necessary here
$(OUT_DIR)/$(FBINK_INPUT_STATIC_NAME): $(INPUT_STATICLIB_OBJS)
	$(AR) $(FBINK_STATIC_AR_OPTS) $@ $^
	$(RANLIB) $@

$(OUT_DIR)/$(FBINK_INPUT_SHARED_NAME_FILE): $(INPUT_SHAREDLIB_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(SHARED_LIB_CFLAGS) $(LIB_CFLAGS) $(LDFLAGS) $(LIB_LDFLAGS) $(EXTRA_LDFLAGS) $(FBINK_INPUT_SHARED_LDFLAGS) -o $@ $^ $(SHARED_LIBS)

# Vendor in our dependencies
$(OUT_DIR)/static/$(FBINK_PARTIAL_NAME): $(OUT_DIR)/static/$(FBINK_STATIC_NAME)
	$(CC) $(FBINK_PARTIAL_GCC_OPTS) -o $@ $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(FBINK_PARTIAL_LDFLAGS) $< $(STATIC_LIBS)
# Final static archive
$(OUT_DIR)/$(FBINK_STATIC_NAME): $(OUT_DIR)/static/$(FBINK_PARTIAL_NAME)
	$(AR) $(FBINK_STATIC_AR_OPTS) $@ $<
	$(RANLIB) $@

$(OUT_DIR)/$(FBINK_SHARED_NAME_VER): $(OUT_DIR)/$(FBINK_SHARED_NAME_FILE)
	ln -sf $(FBINK_SHARED_NAME_FILE) $@
$(OUT_DIR)/$(FBINK_SHARED_NAME): $(OUT_DIR)/$(FBINK_SHARED_NAME_VER)
	ln -sf $(FBINK_SHARED_NAME_VER) $@

$(OUT_DIR)/$(FBINK_INPUT_SHARED_NAME_VER): $(OUT_DIR)/$(FBINK_INPUT_SHARED_NAME_FILE)
	ln -sf $(FBINK_INPUT_SHARED_NAME_FILE) $@
$(OUT_DIR)/$(FBINK_INPUT_SHARED_NAME): $(OUT_DIR)/$(FBINK_INPUT_SHARED_NAME_VER)
	ln -sf $(FBINK_INPUT_SHARED_NAME_VER) $@

staticlib: $(OUT_DIR)/$(FBINK_STATIC_NAME)
sharedlib: $(OUT_DIR)/$(FBINK_SHARED_NAME_VER) $(OUT_DIR)/$(FBINK_SHARED_NAME)

staticinputlib: $(OUT_DIR)/$(FBINK_INPUT_STATIC_NAME)
sharedinputlib: $(OUT_DIR)/$(FBINK_INPUT_SHARED_NAME_VER) $(OUT_DIR)/$(FBINK_INPUT_SHARED_NAME)

# NOTE: We keep FEATURES_CPPFLAGS solely to handle ifdeffery crap ;)
$(OUT_DIR)/static/fbink: $(OUT_DIR)/$(FBINK_STATIC_NAME) $(CMD_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ $(CMD_OBJS) $(LIBS_FOR_STATIC) $(LIBS)

$(OUT_DIR)/static/button_scan: $(OUT_DIR)/$(FBINK_STATIC_NAME) $(BTN_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ $(BTN_OBJS) $(LIBS_FOR_STATIC) $(LIBS)

ifdef WITH_BUTTON_SCAN
staticbin: $(OUT_DIR)/static/fbink $(OUT_DIR)/static/button_scan
	ln -f $(OUT_DIR)/static/fbink $(OUT_DIR)/fbink
	ln -f $(OUT_DIR)/static/button_scan $(OUT_DIR)/button_scan
else
staticbin: $(OUT_DIR)/static/fbink
	ln -f $< $(OUT_DIR)/fbink
endif

# NOTE: Ditto
$(OUT_DIR)/shared/fbink: $(OUT_DIR)/$(FBINK_SHARED_NAME) $(OUT_DIR)/$(FBINK_SHARED_NAME_VER) $(CMD_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ $(CMD_OBJS) $(LIBS_FOR_SHARED) $(LIBS)

$(OUT_DIR)/shared/button_scan: $(OUT_DIR)/$(FBINK_SHARED_NAME) $(OUT_DIR)/$(FBINK_SHARED_NAME_VER) $(BTN_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ $(BTN_OBJS) $(LIBS_FOR_SHARED) $(LIBS)

ifdef WITH_BUTTON_SCAN
sharedbin: $(OUT_DIR)/shared/fbink $(OUT_DIR)/shared/button_scan
	ln -f $(OUT_DIR)/shared/fbink $(OUT_DIR)/fbink
	ln -f $(OUT_DIR)/shared/button_scan $(OUT_DIR)/button_scan
else
sharedbin: $(OUT_DIR)/shared/fbink
	ln -f $< $(OUT_DIR)/fbink
endif

striplib: $(OUT_DIR)/$(FBINK_SHARED_NAME) $(OUT_DIR)/$(FBINK_SHARED_NAME_VER)
	$(STRIP) --strip-unneeded $(OUT_DIR)/$(FBINK_SHARED_NAME_FILE)

stripinputlib: $(OUT_DIR)/$(FBINK_INPUT_SHARED_NAME) $(OUT_DIR)/$(FBINK_INPUT_SHARED_NAME_VER)
	$(STRIP) --strip-unneeded $(OUT_DIR)/$(FBINK_INPUT_SHARED_NAME_FILE)

# NOTE: Unless you *really* need to, I don't recommend stripping LTO archives,
#       strip a binary linked against the untouched archive instead.
#       (c.f., https://sourceware.org/bugzilla/show_bug.cgi?id=21479)
#       (i.e., strip doesn't know what to do with LTO archives, has no -plugin wrapper, and as such, breaks the symbol table).
striparchive: $(OUT_DIR)/$(FBINK_STATIC_NAME)
	$(STRIP) --strip-unneeded $(OUT_DIR)/$(FBINK_STATIC_NAME)
	$(RANLIB) $(OUT_DIR)/$(FBINK_STATIC_NAME)

ifdef WITH_BUTTON_SCAN
stripbin:
	$(STRIP) --strip-unneeded $(OUT_DIR)/fbink
	$(STRIP) --strip-unneeded $(OUT_DIR)/button_scan
else
stripbin: $(OUT_DIR)/fbink
	$(STRIP) --strip-unneeded $(OUT_DIR)/fbink
endif

ifdef LINUX
utils: | outdir
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(DOOM_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LIB_CFLAGS) $(LTO_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/doom utils/doom.c -lrt -lm
else
utils: libi2c.built | outdir
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(TOOLS_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LIB_CFLAGS) $(LTO_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/rota utils/rota.c
	$(STRIP) --strip-unneeded $(OUT_DIR)/rota
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(DOOM_CPPFLAGS) $(I2C_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LIB_CFLAGS) $(LTO_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(I2C_LDFLAGS) -o$(OUT_DIR)/doom utils/doom.c -lrt $(UTILS_LIBS) $(I2C_LIBS)
	$(STRIP) --strip-unneeded $(OUT_DIR)/doom
endif

# NOTE: That's a dumb little tool that was used to ease the migration to less convoluted rotation quirks handling...
#       As such, it's native, but tailored to a Kobo target...
rota_map: libi2c.built | outdir
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) -DFBINK_MINIMAL -DFBINK_FOR_KOBO $(I2C_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LIB_CFLAGS) $(LTO_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/rota_map tools/rota_map.c
	$(STRIP) --strip-unneeded $(OUT_DIR)/rota_map

ifdef KOBO
alt: libi2c.built | outdir
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(DOOM_CPPFLAGS) $(I2C_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LIB_CFLAGS) $(LTO_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(I2C_LDFLAGS) -o$(OUT_DIR)/alt_buffer utils/alt_buffer.c $(I2C_LIBS)
	$(STRIP) --strip-unneeded $(OUT_DIR)/alt_buffer
endif

ifdef KOBO
sunxi: libi2c.built | outdir
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/ion_heaps utils/ion_heaps.c
	$(STRIP) --strip-unneeded $(OUT_DIR)/ion_heaps
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(I2C_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(I2C_LDFLAGS) -o$(OUT_DIR)/kx122_i2c utils/kx122_i2c.c $(I2C_LIBS)
	$(STRIP) --strip-unneeded $(OUT_DIR)/kx122_i2c
endif

ifdef KOBO
$(OUT_DIR)/finger_trace: libevdev.built tiny.built
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(EVDEV_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(EVDEV_LDFLAGS) -o $@ utils/finger_trace.c $(LIBS_FOR_STATIC) $(EVDEV_LIBS) $(LIBS)
	$(STRIP) --strip-unneeded $@

ftrace: $(OUT_DIR)/finger_trace
endif

# NOTE: Same as the CLI tool, we keep FEATURES_CPPFLAGS (via *_FEATURES) for ifdef handling
$(OUT_DIR)/fbdepth: tinier.built
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(TINIER_FEATURES) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ utils/fbdepth.c $(LIBS_FOR_STATIC) $(LIBS)
	$(STRIP) --strip-unneeded $@

fbdepth: $(OUT_DIR)/fbdepth

$(OUT_DIR)/input_scan: tinyish.built
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(TINYISH_FEATURES) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ utils/input_scan.c $(LIBS_FOR_STATIC) $(LIBS)
	$(STRIP) --strip-unneeded $@

input_scan: $(OUT_DIR)/input_scan

$(OUT_DIR)/dump: small.built
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(SMALL_FEATURES) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ utils/dump.c $(LIBS_FOR_STATIC) $(LIBS)
	$(STRIP) --strip-unneeded $@

dump: $(OUT_DIR)/dump

strip: static
	$(MAKE) stripbin

debug:
	$(MAKE) static DEBUG=true DEBUGFLAGS=true

static:
	$(MAKE) staticlib
	$(MAKE) staticbin

tinier tinier.built:
	$(MAKE) cleanstaticlib
	$(MAKE) staticlib MINIMAL=true
	touch tinier.built

tinyish tinyish.built:
	$(MAKE) cleanstaticlib
	$(MAKE) staticlib MINIMAL=true INPUT=true
	touch tinyish.built

tiny tiny.built:
	$(MAKE) cleanstaticlib
	$(MAKE) staticlib MINIMAL=true DRAW=true INPUT=true
	touch tiny.built

small small.built:
	$(MAKE) cleanstaticlib
	$(MAKE) staticlib MINIMAL=true BITMAP=true IMAGE=true
	touch small.built

# NOTE: This one may be a bit counter-intuitive... It's to build a static library built like if it were shared (i.e., PIC),
#       because apparently that's a requirement for FFI in some high-level languages (i.e., Go; c.f., #7)
pic:
	$(MAKE) staticlib PIC=true

shared:
	$(MAKE) sharedlib
	$(MAKE) sharedbin STANDALONE=true

release: shared
	$(MAKE) striplib
	$(MAKE) stripbin

inputlib:
	$(MAKE) cleansharedlib
	$(MAKE) sharedinputlib MINIMAL=true INPUT=true
	$(MAKE) stripinputlib

kindle:
	$(MAKE) strip KINDLE=true

legacy:
	$(MAKE) strip KINDLE=true LEGACY=true

linux:
	$(MAKE) strip LINUX=true

cervantes:
	$(MAKE) strip CERVANTES=true
	$(CURDIR)/tools/do_debian_package.sh $(OUT_DIR) armel

remarkable:
	$(MAKE) strip REMARKABLE=true

pocketbook:
	$(MAKE) strip POCKETBOOK=true

libunibreak.built:
	mkdir -p libunibreak-staged
	cd libunibreak && \
	env NOCONFIGURE=1 ./autogen.sh
	cd libunibreak-staged && \
	env CPPFLAGS='$(CPPFLAGS) $(EXTRA_CPPFLAGS)' \
	CFLAGS='$(CFLAGS) $(EXTRA_CFLAGS) $(UNIBREAK_CFLAGS)' \
	LDFLAGS='$(LDFLAGS)' \
	"$$OLDPWD/libunibreak/configure" \
	$(if $(CROSS_TC),--host=$(CROSS_TC),) \
	--enable-static \
	--disable-shared \
	$(if $(PIC),--with-pic=yes,)
	$(SUBMAKE) -C libunibreak-staged
	touch "$@"

libi2c.built:
	mkdir -p libi2c-staged
	$(MAKE) -C i2c-tools \
	BUILD_DYNAMIC_LIB=0 USE_STATIC_LIB=1 BUILD_STATIC_LIB=1 V=1 \
	CC='$(CC)' AR='$(AR)' \
	CFLAGS='$(CFLAGS) $(EXTRA_CFLAGS) $(I2C_CFLAGS)' \
	PREFIX="/" libdir="/lib" DESTDIR='$(CURDIR)/libi2c-staged' \
	install-lib install-include
	touch "$@"

libevdev.built:
	mkdir -p libevdev-staged
	cd libevdev && \
	autoreconf -fi && \
	env CPPFLAGS='$(CPPFLAGS) $(EXTRA_CPPFLAGS)' \
	CFLAGS='$(CFLAGS) $(EXTRA_CFLAGS) $(EVDEV_CFLAGS)' \
	LDFLAGS='$(LDFLAGS)' \
	./configure $(if $(CROSS_TC),--host=$(CROSS_TC),) \
	--prefix='$(CURDIR)/libevdev-staged' \
	--enable-static \
	--disable-shared && \
	$(SUBMAKE) install
	touch "$@"

ifeq "$(CC_IS_CLANG)" "1"
armcheck:
	$(warning Using a Clang TC, assuming it's setup properly for cross-compilation)
else
armcheck:
ifeq (,$(findstring arm-,$(CC)))
	$(error You forgot to setup a cross TC, you dummy!)
endif
endif

kobo: armcheck
	$(MAKE) fbdepth KOBO=true
	mv -v $(CURDIR)/Release/fbdepth $(CURDIR)/fbdepth
	$(MAKE) clean
	$(MAKE) release utils KOBO=true
	mkdir -p Kobo/usr/local/fbink/bin Kobo/usr/bin Kobo/usr/local/fbink/lib Kobo/usr/local/fbink/include
	cp -av $(CURDIR)/Release/fbink Kobo/usr/local/fbink/bin
	ln -sf /usr/local/fbink/bin/fbink Kobo/usr/bin/fbink
	-cp -av $(CURDIR)/Release/button_scan Kobo/usr/local/fbink/bin && ln -sf /usr/local/fbink/bin/button_scan Kobo/usr/bin/button_scan
	mv -v $(CURDIR)/fbdepth  Kobo/usr/local/fbink/bin
	cp -av $(CURDIR)/Release/doom Kobo/usr/local/fbink/bin
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
	cd Kobo && zip -r ../Release/FBInk-$(FBINK_VERSION).zip .
	mv -v Release/FBInk-$(FBINK_VERSION).zip Kobo/

devcap: armcheck distclean
	$(MAKE) fbdepth KOBO=true
	mv -v $(CURDIR)/Release/fbdepth $(CURDIR)/fbdepth
	$(MAKE) clean
	$(MAKE) input_scan KOBO=true
	mv -v $(CURDIR)/Release/input_scan $(CURDIR)/input_scan
	$(MAKE) clean
	$(MAKE) ftrace KOBO=true
	mv -v $(CURDIR)/Release/finger_trace $(CURDIR)/finger_trace
	$(MAKE) clean
	$(MAKE) strip KOBO=true
	$(MAKE) utils KOBO=true
	mkdir -p Kobo
	cp -av $(CURDIR)/utils/devcap_test.sh Kobo
	cp -av $(CURDIR)/Release/fbink Kobo
	mv -v $(CURDIR)/fbdepth Kobo
	mv -v $(CURDIR)/input_scan Kobo
	mv -v $(CURDIR)/finger_trace Kobo
	cp -av $(CURDIR)/Release/rota Kobo
	wget "https://svn.ak-team.com/svn/Configs/trunk/Kindle/Kobo_Hacks/USBNetwork/src/usbnet/bin/evtest" -O Kobo/evtest
	chmod -cvR a+x Kobo/evtest
	wget "https://svn.ak-team.com/svn/Configs/trunk/Kindle/Kobo_Hacks/USBNetwork/src/usbnet/bin/fbgrab" -O Kobo/fbgrab
	chmod -cvR a+x Kobo/fbgrab
	wget "https://svn.ak-team.com/svn/Configs/trunk/Kindle/Kobo_Hacks/USBNetwork/src/usbnet/lib/libpng16.so.16" -O Kobo/libpng16.so.16
	wget "https://svn.ak-team.com/svn/Configs/trunk/Kindle/Kobo_Hacks/USBNetwork/src/usbnet/lib/libz.so.1" -O Kobo/libz.so.1
	tar --owner=root --group=root -cvzf Release/Kobo-DevCap-Test.tar.gz -C Kobo .

ci: armcheck distclean
	$(MAKE) fbdepth KOBO=true
	mv -v $(CURDIR)/Release/fbdepth $(CURDIR)/fbdepth
	$(MAKE) clean
	$(MAKE) input_scan KOBO=true
	mv -v $(CURDIR)/Release/input_scan $(CURDIR)/input_scan
	$(MAKE) clean
	$(MAKE) ftrace KOBO=true
	mv -v $(CURDIR)/Release/finger_trace $(CURDIR)/finger_trace
	$(MAKE) clean
	$(MAKE) strip KOBO=true
	mkdir -p Kobo
	cp -av $(CURDIR)/Release/fbink Kobo
	mv -v $(CURDIR)/fbdepth Kobo
	mv -v $(CURDIR)/input_scan Kobo
	mv -v $(CURDIR)/finger_trace Kobo
	cp -av $(CURDIR)/README.md Kobo/README.md
	cp -av $(CURDIR)/LICENSE Kobo/LICENSE
	cp -av $(CURDIR)/CREDITS Kobo/CREDITS
	tar --owner=root --group=root -cvzf Release/FBInk-Kobo-$(FBINK_VERSION).tar.gz -C Kobo .

libunibreakclean:
	-$(MAKE) -C libunibreak distclean
	-cd libunibreak && \
	git reset --hard && \
	git clean -fxdq

libi2cclean:
	-$(MAKE) -C i2c-tools clean
	-cd i2c-tools && \
	git reset --hard && \
	git clean -fxdq

libevdevclean:
	-$(MAKE) -C libevdev distclean
	-cd libevdev && \
	git reset --hard && \
	git clean -fxdq

cleansharedlib:
	rm -rf $(OUT_DIR)/*.so*
	rm -rf $(OUT_DIR)/shared/*.o
	rm -rf $(OUT_DIR)/shared/*.opt.yaml
	rm -rf $(OUT_DIR)/shared/cutef8/*.o
	rm -rf $(OUT_DIR)/shared/cutef8/*.opt.yaml
	rm -rf $(OUT_DIR)/shared/libunibreak/src/*.o
	rm -rf $(OUT_DIR)/shared/libunibreak/src/*.opt.yaml
	rm -rf $(OUT_DIR)/shared/qimagescale/*.o
	rm -rf $(OUT_DIR)/shared/qimagescale/*.opt.yaml
	rm -rf $(OUT_DIR)/shared/utf8

cleanstaticlib:
	rm -rf $(OUT_DIR)/*.a
	rm -rf $(OUT_DIR)/static/$(FBINK_PARTIAL_NAME)
	rm -rf $(OUT_DIR)/static/*.a
	rm -rf $(OUT_DIR)/static/*.o
	rm -rf $(OUT_DIR)/static/*.opt.yaml
	rm -rf $(OUT_DIR)/static/cutef8/*.o
	rm -rf $(OUT_DIR)/static/cutef8/*.opt.yaml
	rm -rf $(OUT_DIR)/static/libunibreak/src/*.o
	rm -rf $(OUT_DIR)/static/libunibreak/src/*.opt.yaml
	rm -rf $(OUT_DIR)/static/qimagescale/*.o
	rm -rf $(OUT_DIR)/static/qimagescale/*.opt.yaml
	rm -rf $(OUT_DIR)/static/utf8
	rm -rf small.built
	rm -rf tiny.built
	rm -rf tinyish.built
	rm -rf tinier.built

cleanlib: cleansharedlib cleanstaticlib

clean: cleanlib
	rm -rf Kobo/
	rm -rf $(OUT_DIR)/*.o
	rm -rf $(OUT_DIR)/*.opt.yaml
	rm -rf $(OUT_DIR)/shared/fbink
	rm -rf $(OUT_DIR)/static/fbink
	rm -rf $(OUT_DIR)/fbink
	rm -rf $(OUT_DIR)/shared/button_scan
	rm -rf $(OUT_DIR)/static/button_scan
	rm -rf $(OUT_DIR)/button_scan
	rm -rf $(OUT_DIR)/rota
	rm -rf $(OUT_DIR)/fbdepth
	rm -rf $(OUT_DIR)/input_scan
	rm -rf $(OUT_DIR)/alt_buffer
	rm -rf $(OUT_DIR)/doom
	rm -rf $(OUT_DIR)/dump
	rm -rf $(OUT_DIR)/Kobo-DevCap-Test.tar.gz
	rm -rf $(OUT_DIR)/kx122_i2c
	rm -rf $(OUT_DIR)/ion_heaps
	rm -rf $(OUT_DIR)/finger_trace
	rm -rf $(OUT_DIR)/rota_map
	rm -rf $(OUT_DIR)/FBInk-*.tar.*z

distclean: clean libunibreakclean libi2cclean libevdevclean
	rm -rf libunibreak-staged
	rm -rf libunibreak.built
	rm -rf libi2c-staged
	rm -rf libi2c.built
	rm -rf libevdev-staged
	rm -rf libevdev.built
	-[[ -d .git ]] && rm -rf VERSION

dist: distclean
	echo $(FBINK_VERSION) > VERSION
	mkdir -p Release
	tar --exclude-vcs --exclude-vcs-ignores \
	--exclude=stb/tests --exclude=stb/data --exclude=stb/tools --exclude=stb/deprecated \
	--exclude=libevdev/doc --exclude=libevdev/test \
	--exclude=.github --exclude=.gitlab-ci --exclude=.travis.yml \
	--exclude=tools/unibdf2hex \
	--exclude=fonts/*.hex --exclude=fonts/*.bdf --exclude=fonts/*.gz --exclude=fonts/*.fon --exclude=fonts/*.ttf --exclude=fonts/*.txt \
	--exclude=resources/*.png \
	-P --transform="s,$(CURDIR),FBInk-$(FBINK_VERSION),xS" --show-transformed-names \
	-cvJf Release/FBInk-$(FBINK_VERSION).tar.xz $(CURDIR)

# No deps, so just try to install all the things
install:
	install -d -m 755 $(INCDIR)
	install -m 644 '$(CURDIR)/fbink.h' $(INCDIR)
	install -d -m 755 $(DOCDIR)
	install -m 644 CLI.md $(DOCDIR)
	install -d -m 755 $(LIBDIR)
	-install '$(OUT_DIR)/$(FBINK_SHARED_NAME_FILE)' $(LIBDIR)
	-ln -sf $(FBINK_SHARED_NAME_FILE) '$(LIBDIR)/$(FBINK_SHARED_NAME_VER)'
	-ln -sf $(FBINK_SHARED_NAME_VER) '$(LIBDIR)/$(FBINK_SHARED_NAME)'
	-install '$(OUT_DIR)/$(FBINK_STATIC_NAME)' $(LIBDIR)
	install -d -m 755 $(BINDIR)
	-install '$(OUT_DIR)/fbink' $(BINDIR)
	-install '$(OUT_DIR)/fbdepth' $(BINDIR)

format:
	clang-format -style=file -i *.c *.h cutef8/*.c cutef8/*.h utils/*.c qimagescale/*.c qimagescale/*.h tools/*.c eink/*-kobo.h eink/*-kindle.h eink/einkfb.h


.PHONY: default outdir all staticlib sharedlib staticinputlib sharedinputlib static small tiny tinyish tinier shared striplib stripinputlib striparchive stripbin strip debug static pic shared release inputlib kindle legacy cervantes linux armcheck kobo remarkable pocketbook libunibreakclean libi2cclean libevdevclean utils rota_map alt sunxi ftrace fbdepth input_scan dump devcap ci clean cleansharedlib cleanstaticlib cleanlib distclean dist install format
