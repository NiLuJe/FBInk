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
	ifdef SHARED
		ifeq "$(CC_IS_CLANG)" "0"
			# Applies when building a shared library as well as just PIC in general.
			# Fun fact: apparently the default on Clang ;).
			SHARED_CFLAGS+=-fno-semantic-interposition
		endif
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
	# NOTE: -Wformat-truncation=2 is still a tad too aggressive w/ GCC 9, so, tone it down to avoid false-positives...
	EXTRA_CFLAGS+=-Wformat-truncation=1
	# NOTE: This doesn't really play nice w/ FORTIFY, leading to an assload of false-positives, unless LTO is enabled
	ifeq (,$(findstring flto,$(CFLAGS)))
		# NOTE: GCC 9 is more verbose, so nerf that, too, when building w/o LTO on native systems...
		ifeq "$(CC_IS_CROSS)" "0"
			EXTRA_CFLAGS+=-Wno-stringop-truncation
		endif
	endif
	EXTRA_CFLAGS+=-Wnull-dereference
	EXTRA_CFLAGS+=-Wuninitialized
	ifeq (flto,$(findstring flto,$(CFLAGS)))
		# NOTE: Inlining put_pixel in fbink_print_ot triggers a few -Wmaybe-uninitialized when we pass grayscale pixels...
		#       Actually harmless, because they trip in an RGB565 codepath, which we make sure always get fed RGB32.
		#       Unfortunately, since they're tripped at link-time, I can't pragma'em away :/.
		EXTRA_CFLAGS+=-Wno-maybe-uninitialized
	endif
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
FBINK_VERSION:=$(shell git describe || cat VERSION)
# Only use it if we got something useful...
ifdef FBINK_VERSION
	ifdef KINDLE
		LIB_CFLAGS+=-DFBINK_VERSION='"$(FBINK_VERSION) for Kindle"'
	else
		ifdef CERVANTES
			LIB_CFLAGS+=-DFBINK_VERSION='"$(FBINK_VERSION) for Cervantes"'
		else
			ifdef LINUX
				LIB_CFLAGS+=-DFBINK_VERSION='"$(FBINK_VERSION) for Linux"'
			else
				ifdef KOBO
					LIB_CFLAGS+=-DFBINK_VERSION='"$(FBINK_VERSION) for Kobo"'
				else
					ifdef REMARKABLE
						LIB_CFLAGS+=-DFBINK_VERSION='"$(FBINK_VERSION) for reMarkable"'
					else
						ifdef POCKETBOOK
							LIB_CFLAGS+=-DFBINK_VERSION='"$(FBINK_VERSION) for PocketBook"'
						else
							# NOTE: Should never happen!
							LIB_CFLAGS+=-DFBINK_VERSION='"$(FBINK_VERSION)"'
						endif
					endif
				endif
			endif
		endif
	endif
else
	# Just so we don't use an empty var down the line...
	FBINK_VERSION:=dev
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
# Unless we're asking for a minimal build, include the other features, except for Unifont.
ifdef MINIMAL
	FEATURES_CPPFLAGS+=-DFBINK_MINIMAL
else
	FEATURES_CPPFLAGS+=-DFBINK_WITH_DRAW
	FEATURES_CPPFLAGS+=-DFBINK_WITH_BITMAP
	FEATURES_CPPFLAGS+=-DFBINK_WITH_FONTS
	FEATURES_CPPFLAGS+=-DFBINK_WITH_IMAGE
	FEATURES_CPPFLAGS+=-DFBINK_WITH_OPENTYPE
	# Connect button scanning is Kobo specific
	ifdef KOBO
		WITH_BUTTON_SCAN:=True
		FEATURES_CPPFLAGS+=-DFBINK_WITH_BUTTON_SCAN
	endif
	# Unifont is *always* optional, because it'll add almost 2MB to the binary size!
	ifdef UNIFONT
		FEATURES_CPPFLAGS+=-DFBINK_WITH_UNIFONT
	endif
endif

# Manage modular MINIMAL builds...
ifdef MINIMAL
	# Support tweaking a MINIMAL build to still include drawing primitives
	ifdef DRAW
		FEATURES_CPPFLAGS+=-DFBINK_WITH_DRAW
	endif

	# Support tweaking a MINIMAL build to still include fixed-cell font rendering
	ifdef BITMAP
		# Make sure we actually have drawing support
		ifndef DRAW
			FEATURES_CPPFLAGS+=-DFBINK_WITH_DRAW
		endif
		FEATURES_CPPFLAGS+=-DFBINK_WITH_BITMAP
	endif

	# Support tweaking a MINIMAL build to still include extra bitmap fonts
	ifdef FONTS
		# Make sure we actually have drawing support
		ifndef DRAW
			FEATURES_CPPFLAGS+=-DFBINK_WITH_DRAW
		endif
		# Make sure we actually have fixed-cell support
		ifndef BITMAP
			FEATURES_CPPFLAGS+=-DFBINK_WITH_BITMAP
		endif
		FEATURES_CPPFLAGS+=-DFBINK_WITH_FONTS
		# As well as, optionally, the full Unifont...
		ifdef UNIFONT
			FEATURES_CPPFLAGS+=-DFBINK_WITH_UNIFONT
		endif
	endif

	# Support tweaking a MINIMAL build to still include image support
	ifdef IMAGE
		# Make sure we actually have drawing support
		ifndef DRAW
			FEATURES_CPPFLAGS+=-DFBINK_WITH_DRAW
		endif
		FEATURES_CPPFLAGS+=-DFBINK_WITH_IMAGE
	endif

	# Support tweaking a MINIMAL build to still include OpenType support
	ifdef OPENTYPE
		# Make sure we actually have drawing support
		ifndef DRAW
			FEATURES_CPPFLAGS+=-DFBINK_WITH_DRAW
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
	ifdef BUTTON_SCAN
		# Make sure we actually have drawing support
		ifndef DRAW
			FEATURES_CPPFLAGS+=-DFBINK_WITH_DRAW
		endif
		WITH_BUTTON_SCAN:=True
		FEATURES_CPPFLAGS+=-DFBINK_WITH_BUTTON_SCAN
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

# How we handle our library creation
FBINK_SHARED_LDFLAGS:=-shared -Wl,-soname,libfbink.so.1
FBINK_SHARED_NAME_FILE:=libfbink.so.1.0.0
FBINK_SHARED_NAME:=libfbink.so
FBINK_SHARED_NAME_VER:=libfbink.so.1
FBINK_STATIC_AR_OPTS:=rc
FBINK_STATIC_NAME:=libfbink.a
FBINK_PARTIAL_GCC_OPTS:=-r
FBINK_PARTIAL_NAME:=libfbink.o
FBINK_PARTIAL_LDFLAGS:=-Wl,--whole-archive

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
UNIBREAK_CFLAGS:=-Wno-conversion -Wno-sign-conversion -Wno-suggest-attribute=pure
$(UB_SHAREDLIB_OBJS): QUIET_CFLAGS:=$(UNIBREAK_CFLAGS)
$(UB_STATICLIB_OBJS): QUIET_CFLAGS:=$(UNIBREAK_CFLAGS)

# Silence a few warnings when building libi2c
I2C_CFLAGS:=-Wno-sign-conversion

# Ditto for libevdev
EVDEV_CFLAGS:=-Wno-conversion -Wno-sign-conversion -Wno-undef -Wno-vla-parameter -Wno-format -Wno-null-dereference -Wno-bad-function-cast -Wno-inline
# A random -Werror is injected in Debug builds...
EVDEV_CFLAGS+=-Wno-suggest-attribute=pure -Wno-suggest-attribute=const -Wno-padded
# And when *linking* libevdev (w/ LTO)
EVDEV_LDFLAGS+=-Wno-null-dereference

# Shared lib
$(OUT_DIR)/shared/%.o: %.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(LIB_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(QUIET_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) -o $@ -c $<

# Static lib
$(OUT_DIR)/static/%.o: %.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(LIB_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(QUIET_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) -o $@ -c $<

# CLI front-end
$(OUT_DIR)/%.o: %.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) -o $@ -c $<

outdir:
	mkdir -p $(OUT_DIR)/shared/cutef8 $(OUT_DIR)/static/cutef8 $(OUT_DIR)/shared/libunibreak/src $(OUT_DIR)/static/libunibreak/src $(OUT_DIR)/shared/qimagescale $(OUT_DIR)/static/qimagescale

# Make absolutely sure we create our output directories first, even with unfortunate // timings!
# c.f., https://www.gnu.org/software/make/manual/html_node/Prerequisite-Types.html#Prerequisite-Types
$(SHAREDLIB_OBJS): libi2c.built | outdir
$(UB_SHAREDLIB_OBJS): | outdir
$(QT_SHAREDLIB_OBJS): | outdir
$(STATICLIB_OBJS): libi2c.built | outdir
$(UB_STATICLIB_OBJS): | outdir
$(QT_STATICLIB_OBJS): | outdir
$(CMD_OBJS): | outdir
$(BTN_OBJS): | outdir

all: static

ifdef UNIBREAK
staticlib: $(STATICLIB_OBJS) $(QT_STATICLIB_OBJS) libunibreak.built
	# FBInk
	$(AR) $(FBINK_STATIC_AR_OPTS) $(OUT_DIR)/$(FBINK_STATIC_NAME) $(STATICLIB_OBJS) $(QT_STATICLIB_OBJS)
	# Vendor in our dependencies
	$(CC) $(FBINK_PARTIAL_GCC_OPTS) -o $(OUT_DIR)/$(FBINK_PARTIAL_NAME) $(FBINK_PARTIAL_LDFLAGS) $(OUT_DIR)/$(FBINK_STATIC_NAME) $(STATIC_LIBS)
	rm -f $(OUT_DIR)/$(FBINK_STATIC_NAME)
	$(AR) $(FBINK_STATIC_AR_OPTS) $(OUT_DIR)/$(FBINK_STATIC_NAME) $(OUT_DIR)/$(FBINK_PARTIAL_NAME)
	# Finalize
	$(RANLIB) $(OUT_DIR)/$(FBINK_STATIC_NAME)

sharedlib: $(SHAREDLIB_OBJS) $(QT_SHAREDLIB_OBJS) libunibreak.built
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) $(LDFLAGS) $(LIB_LDFLAGS) $(EXTRA_LDFLAGS) $(FBINK_SHARED_LDFLAGS) -o$(OUT_DIR)/$(FBINK_SHARED_NAME_FILE) $(SHAREDLIB_OBJS) $(QT_SHAREDLIB_OBJS) $(SHARED_LIBS)
	ln -sf $(FBINK_SHARED_NAME_FILE) $(OUT_DIR)/$(FBINK_SHARED_NAME)
	ln -sf $(FBINK_SHARED_NAME_FILE) $(OUT_DIR)/$(FBINK_SHARED_NAME_VER)
else
staticlib: $(STATICLIB_OBJS) $(UB_STATICLIB_OBJS) $(QT_STATICLIB_OBJS)
	# FBInk
	$(AR) $(FBINK_STATIC_AR_OPTS) $(OUT_DIR)/$(FBINK_STATIC_NAME) $(STATICLIB_OBJS) $(UB_STATICLIB_OBJS) $(QT_STATICLIB_OBJS)
	# Vendor in our dependencies
	$(CC) $(FBINK_PARTIAL_GCC_OPTS) -o $(OUT_DIR)/$(FBINK_PARTIAL_NAME) $(FBINK_PARTIAL_LDFLAGS) $(OUT_DIR)/$(FBINK_STATIC_NAME) $(STATIC_LIBS)
	rm -f $(OUT_DIR)/$(FBINK_STATIC_NAME)
	$(AR) $(FBINK_STATIC_AR_OPTS) $(OUT_DIR)/$(FBINK_STATIC_NAME) $(OUT_DIR)/$(FBINK_PARTIAL_NAME)
	# Finalize
	$(RANLIB) $(OUT_DIR)/$(FBINK_STATIC_NAME)

sharedlib: $(SHAREDLIB_OBJS) $(UB_SHAREDLIB_OBJS) $(QT_SHAREDLIB_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) $(LDFLAGS) $(LIB_LDFLAGS) $(EXTRA_LDFLAGS) $(FBINK_SHARED_LDFLAGS) -o$(OUT_DIR)/$(FBINK_SHARED_NAME_FILE) $(SHAREDLIB_OBJS) $(UB_SHAREDLIB_OBJS) $(QT_SHAREDLIB_OBJS) $(SHARED_LIBS)
	ln -sf $(FBINK_SHARED_NAME_FILE) $(OUT_DIR)/$(FBINK_SHARED_NAME)
	ln -sf $(FBINK_SHARED_NAME_FILE) $(OUT_DIR)/$(FBINK_SHARED_NAME_VER)
endif

ifdef WITH_BUTTON_SCAN
staticbin: $(OUT_DIR)/$(FBINK_STATIC_NAME) $(CMD_OBJS) $(BTN_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/fbink $(CMD_OBJS) $(LIBS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/button_scan $(BTN_OBJS) $(LIBS)
else
staticbin: $(OUT_DIR)/$(FBINK_STATIC_NAME) $(CMD_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/fbink $(CMD_OBJS) $(LIBS)
endif

ifdef WITH_BUTTON_SCAN
sharedbin: $(OUT_DIR)/$(FBINK_SHARED_NAME_FILE) $(CMD_OBJS) $(BTN_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/fbink $(CMD_OBJS) $(LIBS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/button_scan $(BTN_OBJS) $(LIBS)
else
sharedbin: $(OUT_DIR)/$(FBINK_SHARED_NAME_FILE) $(CMD_OBJS)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/fbink $(CMD_OBJS) $(LIBS)
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
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(DOOM_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) $(LTO_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/doom utils/doom.c -lrt
else
utils: libi2c.built | outdir
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(TOOLS_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) $(LTO_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/rota utils/rota.c
	$(STRIP) --strip-unneeded $(OUT_DIR)/rota
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(DOOM_CPPFLAGS) $(I2C_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) $(LTO_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(I2C_LDFLAGS) -o$(OUT_DIR)/doom utils/doom.c -lrt $(UTILS_LIBS) $(I2C_LIBS)
	$(STRIP) --strip-unneeded $(OUT_DIR)/doom
endif

# NOTE: That's a dumb little tool that was used to ease the migration to less convoluted rotation quirks handling...
#       As such, it's native, but tailored to a Kobo target...
rota_map: libi2c.built | outdir
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(I2C_CPPFLAGS) -DFBINK_MINIMAL -DFBINK_FOR_KOBO $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) $(LTO_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/rota_map tools/rota_map.c
	$(STRIP) --strip-unneeded $(OUT_DIR)/rota_map

ifdef KOBO
alt: libi2c.built | outdir
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(DOOM_CPPFLAGS) $(I2C_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LIB_CFLAGS) $(LTO_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(I2C_LDFLAGS) -o$(OUT_DIR)/alt_buffer utils/alt_buffer.c $(I2C_LIBS)
	$(STRIP) --strip-unneeded $(OUT_DIR)/alt_buffer
endif

ifdef KOBO
sunxi: libi2c.built | outdir
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/ion_heaps utils/ion_heaps.c
	$(STRIP) --strip-unneeded $(OUT_DIR)/ion_heaps
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(I2C_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(I2C_LDFLAGS) -o$(OUT_DIR)/kx122_i2c utils/kx122_i2c.c $(I2C_LIBS)
	$(STRIP) --strip-unneeded $(OUT_DIR)/kx122_i2c
endif

ifdef KOBO
ftrace: libevdev.built tiny | outdir
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(EVDEV_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(EVDEV_LDFLAGS) -o$(OUT_DIR)/finger_trace utils/finger_trace.c $(LIBS) $(EVDEV_LIBS)
	$(STRIP) --strip-unneeded $(OUT_DIR)/finger_trace
endif

# NOTE: We keep FEATURES_CPPFLAGS because there's still a fair amount of ifdeffery left ;).
fbdepth: tinier
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/fbdepth utils/fbdepth.c $(LIBS)
	$(STRIP) --strip-unneeded $(OUT_DIR)/fbdepth

dump: static
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(FEATURES_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o$(OUT_DIR)/dump utils/dump.c $(LIBS)
	$(STRIP) --strip-unneeded $(OUT_DIR)/dump

strip: static
	$(MAKE) stripbin

debug:
	$(MAKE) static DEBUG=true DEBUGFLAGS=true

static:
	$(MAKE) staticlib
	$(MAKE) staticbin

tiny:
	$(MAKE) staticlib MINIMAL=true DRAW=true

tinier:
	$(MAKE) staticlib MINIMAL=true

# NOTE: This one may be a bit counter-intuitive... It's to build a static library built like if it were shared (i.e., PIC),
#       because apparently that's a requirement for FFI in some high-level languages (i.e., Go; c.f., #7)
pic:
	$(MAKE) staticlib SHARED=true

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

remarkable:
	$(MAKE) strip REMARKABLE=true

pocketbook:
	$(MAKE) strip POCKETBOOK=true

libunibreak.built:
	mkdir -p libunibreak-staged
	cd libunibreak && \
	env NOCONFIGURE=1 ./autogen.sh
	cd libunibreak-staged && \
	env CPPFLAGS="$(CPPFLAGS) $(EXTRA_CPPFLAGS)" \
	CFLAGS="$(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(UNIBREAK_CFLAGS)" \
	LDFLAGS="$(LDFLAGS)" \
	../libunibreak/configure \
	$(if $(CROSS_TC),--host=$(CROSS_TC),) \
	--enable-static \
	--disable-shared \
	$(if $(SHARED),--with-pic=yes,)
	$(MAKE) -C libunibreak-staged
	touch libunibreak.built

libi2c.built:
	mkdir -p libi2c-staged
	$(MAKE) -C i2c-tools \
	BUILD_DYNAMIC_LIB=0 USE_STATIC_LIB=1 BUILD_STATIC_LIB=1 V=1 \
	CC=$(CC) AR=$(AR) \
	CFLAGS="$(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(I2C_CFLAGS)" \
	PREFIX="/" libdir="/lib" DESTDIR="$(CURDIR)/libi2c-staged" \
	install-lib install-include
	touch libi2c.built

libevdev.built:
	mkdir -p libevdev-staged
	cd libevdev && \
	autoreconf -fi && \
	env CPPFLAGS="$(CPPFLAGS) $(EXTRA_CPPFLAGS)" \
	CFLAGS="$(CFLAGS) $(EXTRA_CFLAGS) $(SHARED_CFLAGS) $(EVDEV_CFLAGS)" \
	LDFLAGS="$(LDFLAGS)" \
	./configure $(if $(CROSS_TC),--host=$(CROSS_TC),) \
	--prefix="$(CURDIR)/libevdev-staged" \
	--enable-static \
	--disable-shared && \
	$(MAKE) install
	touch libevdev.built

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
	cp -av $(CURDIR)/Release/button_scan Kobo/usr/local/fbink/bin
	ln -sf /usr/local/fbink/bin/button_scan Kobo/usr/bin/button_scan
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
	$(MAKE) ftrace KOBO=true
	mv -v $(CURDIR)/Release/finger_trace $(CURDIR)/finger_trace
	$(MAKE) clean
	$(MAKE) strip KOBO=true
	$(MAKE) utils KOBO=true
	mkdir -p Kobo
	cp -av $(CURDIR)/utils/devcap_test.sh Kobo
	cp -av $(CURDIR)/Release/fbink Kobo
	mv -v $(CURDIR)/fbdepth Kobo
	mv -v $(CURDIR)/finger_trace Kobo
	cp -av $(CURDIR)/Release/rota Kobo
	wget "https://svn.ak-team.com/svn/Configs/trunk/Kindle/Kobo_Hacks/USBNetwork/src/usbnet/bin/evtest" -O Kobo/evtest
	chmod -cvR a+x Kobo/evtest
	wget "https://svn.ak-team.com/svn/Configs/trunk/Kindle/Kobo_Hacks/USBNetwork/src/usbnet/bin/fbgrab" -O Kobo/fbgrab
	chmod -cvR a+x Kobo/fbgrab
	wget "https://svn.ak-team.com/svn/Configs/trunk/Kindle/Kobo_Hacks/USBNetwork/src/usbnet/lib/libpng16.so.16" -O Kobo/libpng16.so.16
	wget "https://svn.ak-team.com/svn/Configs/trunk/Kindle/Kobo_Hacks/USBNetwork/src/usbnet/lib/libz.so.1" -O Kobo/libz.so.1
	tar --owner=root --group=root -cvzf Release/Kobo-DevCap-Test.tar.gz -C Kobo .

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

clean:
	rm -rf Kobo/
	rm -rf Release/*.a
	rm -rf Release/*.so*
	rm -rf Release/shared/*.o
	rm -rf Release/shared/*.opt.yaml
	rm -rf Release/shared/cutef8/*.o
	rm -rf Release/shared/cutef8/*.opt.yaml
	rm -rf Release/shared/libunibreak/src/*.o
	rm -rf Release/shared/libunibreak/src/*.opt.yaml
	rm -rf Release/shared/qimagescale/*.o
	rm -rf Release/shared/qimagescale/*.opt.yaml
	rm -rf Release/shared/utf8
	rm -rf Release/static/*.o
	rm -rf Release/static/*.opt.yaml
	rm -rf Release/static/cutef8/*.o
	rm -rf Release/static/cutef8/*.opt.yaml
	rm -rf Release/static/libunibreak/src/*.o
	rm -rf Release/static/libunibreak/src/*.opt.yaml
	rm -rf Release/static/qimagescale/*.o
	rm -rf Release/static/qimagescale/*.opt.yaml
	rm -rf Release/static/utf8
	rm -rf Release/*.o
	rm -rf Release/*.opt.yaml
	rm -rf Release/fbink
	rm -rf Release/button_scan
	rm -rf Release/rota
	rm -rf Release/fbdepth
	rm -rf Release/alt_buffer
	rm -rf Release/doom
	rm -rf Release/dump
	rm -rf Release/Kobo-DevCap-Test.tar.gz
	rm -rf Release/kx122_i2c
	rm -rf Release/ion_heaps
	rm -rf Release/finger_trace
	rm -rf Release/FBInk-*.tar.xz
	rm -rf Debug/*.a
	rm -rf Debug/*.so*
	rm -rf Debug/shared/*.o
	rm -rf Debug/shared/*.opt.yaml
	rm -rf Debug/shared/cutef8/*.o
	rm -rf Debug/shared/cutef8/*.opt.yaml
	rm -rf Debug/shared/libunibreak/src/*.o
	rm -rf Debug/shared/libunibreak/src/*.opt.yaml
	rm -rf Debug/shared/qimagescale/*.o
	rm -rf Debug/shared/qimagescale/*.opt.yaml
	rm -rf Debug/shared/utf8
	rm -rf Debug/static/*.o
	rm -rf Debug/static/*.opt.yaml
	rm -rf Debug/static/cutef8/*.o
	rm -rf Debug/static/cutef8/*.opt.yaml
	rm -rf Debug/static/libunibreak/src/*.o
	rm -rf Debug/static/libunibreak/src/*.opt.yaml
	rm -rf Debug/static/qimagescale/*.o
	rm -rf Debug/static/qimagescale/*.opt.yaml
	rm -rf Debug/static/utf8
	rm -rf Debug/*.o
	rm -rf Debug/*.opt.yaml
	rm -rf Debug/fbink
	rm -rf Debug/button_scan
	rm -rf Debug/rota
	rm -rf Debug/fbdepth
	rm -rf Debug/alt_buffer
	rm -rf Debug/doom
	rm -rf Debug/dump
	rm -rf Debug/Kobo-DevCap-Test.tar.gz
	rm -rf Debug/kx122_i2c
	rm -rf Debug/ion_heaps
	rm -rf Debug/finger_trace

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

format:
	clang-format -style=file -i *.c *.h cutef8/*.c cutef8/*.h utils/*.c qimagescale/*.c qimagescale/*.h tools/*.c eink/*-kobo.h eink/*-kindle.h eink/einkfb.h


.PHONY: default outdir all staticlib sharedlib static tiny tinier shared striplib striparchive stripbin strip debug static pic shared release kindle legacy cervantes linux armcheck kobo remarkable pocketbook libunibreakclean libi2cclean libevdevclean utils rota_map alt sunxi ftrace fbdepth dump devcap clean distclean dist format
