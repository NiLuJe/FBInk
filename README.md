# FBInk
[![License](https://img.shields.io/github/license/NiLuJe/FBInk.svg)](/LICENSE) [![Total alerts](https://img.shields.io/lgtm/alerts/g/NiLuJe/FBInk.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/NiLuJe/FBInk/alerts/) [![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/NiLuJe/FBInk.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/NiLuJe/FBInk/context:cpp) [![Language grade: Python](https://img.shields.io/lgtm/grade/python/g/NiLuJe/FBInk.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/NiLuJe/FBInk/context:python) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/0e955c5239c54b10a167bbde1c2d75c1)](https://www.codacy.com/app/NiLuJe/FBInk?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=NiLuJe/FBInk&amp;utm_campaign=Badge_Grade) [![Latest tag](https://img.shields.io/github/tag-date/NiLuJe/FBInk.svg)](https://github.com/NiLuJe/FBInk/releases/)

FrameBuffer eInker

Licensed under the [GPLv3+](/LICENSE).
Housed [here on GitHub](https://github.com/NiLuJe/FBInk).

## What's it for?

This is intended to fill the void felt by Kobo developers and tinkerers when they realize they do not have a builtin way to print stuff on the device's screen!  
It's especially cruel when moving to a Kobo, after being used to the ubiquity of `eips` on Kindle...

In short, it prints messages or images on your screen, handling the low-level tinkering with both the Linux framebuffer interface, and the i.MX EPDC.  
It's been tested on Kobo, Kindle, BQ Cervantes, reMarkable and PocketBook, but porting it to other Linux, i.MX eInk devices should be trivial (hell, even Sipix support shouldn't be too hard).
[#64](https://github.com/NiLuJe/FBInk/pull/64) proved that we can even bend sunxi APIs to our will, if you don't care too much about losing your sanity in the process ;).

By default, text rendering relies on bundled fixed cell bitmap fonts ([see this post](https://www.mobileread.com/forums/showpost.php?p=3765426&postcount=31) for a small sampling),
but thanks to [@shermp](https://github.com/shermp)'s contributions ([#20](https://github.com/NiLuJe/FBInk/pull/20)), you can also rely on full-fledged TrueType/OpenType font rendering!

Image support includes most common formats (JPEG/PNG/TGA/BMP/GIF/PNM), as well as raw packed pixels in the most relevant pixel formats (Gray8 & RGB32; both +/- Alpha).

It also happens to work perfectly fine on *any* kind of Linux framebuffer device, and supports a wide range of bitdepths (4bpp, 8bpp, 16bpp, 24bpp & 32bpp),
so you could use this to draw on your EFI fb, for instance ;).

## How do I install this?

For Kobo devices, there's a discussion thread open [over here](https://www.mobileread.com/forums/showthread.php?t=299110) on MobileRead, where you'll happen to find standalone binaries.
It's purposefully lacking in detailed instructions, because the target audience is mainly developers and tinkerers. Think of this as a safety precaution ;).

There's also a sister thread for Kindle devices [over here](https://www.mobileread.com/forums/showthread.php?t=299620) where, besides binaries, you'll also find examples of people doing crazy things with it ;).

In practice, most Kindle & Kobo users will in fact get it for free, as it's bundled with most of my [packages](http://www.mobileread.com/forums/showthread.php?t=225030).

As an example of usage in the wild, see [KFMon](https://github.com/NiLuJe/kfmon), where I'm using it to provide visual feedback, or [kobo-rclone](https://github.com/shermp/kobo-rclone), where it's also used for screen scraping. We're also using it in [KOReader](https://github.com/koreader/koreader), to make the OTA update process more user-friendly.  
A quick GitHub search for code mentioning [fbink](https://github.com/search?q=fbink) should also yield interesting results, e.g., [a DAMAGE-handling shim for X11](https://github.com/schuhumi/fbink-xdamage), [a Qt5 QPA](https://github.com/Rain92/qt5-kobo-platform-plugin) or [InkVT, a terminal emulator](https://github.com/llandsmeer/inkvt).  
See also the various bindings in other languages, which often include a few examples.

## How can I tinker with it?

A CLI utility is available, built around the same public API that can be used via a shared or static library for C projects, or via FFI in other languages (beware, though, it's licensed under the GPLv3+, not the LGPL).
For the CLI utility, see [its documentation](CLI.md) or run `fbink --help` for details.
For the library, see the [public header](fbink.h). Don't hesitate to contact me if things appear unclear!  

NOTE: It generally makes *NO* attempt at handling software rotation, because that currently appears to be the right thing to do with both current Kobo FW versions and on Kindle.  
YMMV on older FW, or if something else is fudging with fb rotation, or if your application is implementing rotation in software (i.e., a rotated viewport).  
As far as hardware rotation is concerned, there are a few specific exceptions made for Kobo devices:
-   Those running in 16bpp mode and appearing to be in landscape mode: since that seems to be their native state, we *attempt* to compensate for this,
    as we can legitimately be used before Nickel itself corrects this.

-   On devices with an accelerometer, like the Forma & Libra, where Nickel itself will handle the hardware rotation.

## How does it look?

A few basic examples of the fixed cell text rendering...

![FBInk on a Kobo H2O](https://raw.githubusercontent.com/NiLuJe/FBInk/master/resources/fbink_readme.png)

Or if we drop an image in there...

![FBInk 1.2.0 on a Kobo H2O](https://raw.githubusercontent.com/NiLuJe/FBInk/master/resources/fbink_image.png)

And with all the bells and whistles of working transparency, even on ancient hardware :).

![FBInk 1.2.5 on a Kindle 3](https://raw.githubusercontent.com/NiLuJe/FBInk/master/resources/fbink_alpha.png)

Here's a few other fonts, as well as a progress bar...

![FBInk 1.6.2 on a Kobo H2O](https://raw.githubusercontent.com/NiLuJe/FBInk/master/resources/fbink_bars.png)

And when using shiny TrueType fonts :).

![FBInk 1.8.0 on a Kobo H2O](https://raw.githubusercontent.com/NiLuJe/FBInk/master/resources/fbink_ot.png)

## How do I build it?

Unless you're just trying to take it for a spin on a native pure Linux system (`make linux`), you'll need a cross-compiler targeting your, well, target device.  
The Makefile is tailored to automatically detect my own cross-compilation [ToolChain setups](http://trac.ak-team.com/trac/browser/niluje/Configs/trunk/Kindle/Misc/x-compile.sh), which I evidently heartily recommend using instead of relying on generic cross-compilation toolchains which may not exactly target the right kernel/libc duo ;).  
Using the [koxtoolchain](https://github.com/koreader/koxtoolchain) frontend should make building one of these a fairly painless process.

In case you're using your own toolchain, please note that we require C11 support (GCC >= 4.9, Clang >= 3.0).

Provided you're not using an older compiler, I highly recommend building this with LTO enabled!

With that out of the way, the default target (i.e., `make`) will yield a static Kobo build, while `make kobo` will yield a stripped shared build, and additionally package everything the Kobo way. The package found in the Kobo thread is built this way.

There's a few convenience targets for usual build types (`make static` for a static build, `make shared` for a shared build, `make strip` for a stripped static build, `make release` for a stripped shared build, `make debug` for a debug build), as well as a few unusual ones for very specific use cases, usually related to FFI bindings (`make pic` for a PIC static build, or passing `STATIC_LIBM=1` to make to attempt to link against libm statically).

The choice of target platform is handled via a simple variable:
-   Pass `KINDLE=1` to make for a Kindle build (`make kindle` does that on a stripped static build).
-   Pass `KINDLE=1 LEGACY=1` to make for a FW 2.x Kindle build (`make legacy` does that on a stripped static build). This basically just disables CLOEXEC, which might not be supported on FW 2.x.
-   Pass `CERVANTES=1` to make for a BQ/Cervantes build (`make cervantes` does that on a stripped static build).
-   Pass `REMARKABLE=1` to make for a reMarkable build (`make remarkable` does that on a stripped static build).
-   Pass `POCKETBOOK=1` to make for a PocketBook build (`make pocketbook` does that on a stripped static build).

The same logic is used to allow for a bit of tailoring:
-   Pass `MINIMAL=1` to make for a build with *very* limited functionality (no drawing primitives, no fixed-cell font rendering, no image rendering, no extra fonts, no OpenType), which yields a much smaller application & library.
-   Pass `DEBUG=1` to make for a Debug build, and pass `DEBUG=1 DEBUGFLAGS=1` to make for a Debug build with enforced debug CFLAGS.

You can also *append* features one by one to a `MINIMAL` build:
-   Pass `DRAW=1` to add support for drawing primitives.
-   Pass `BITMAP=1` to add support for fixed-cell font rendering. (Implies `DRAW`)
-   Pass `FONTS=1` to add support for the extra bundled fixed-cell fonts. (Implies `BITMAP`)
-   Pass `IMAGE=1` to add image support. (Implies `DRAW`)
-   Pass `OPENTYPE=1` to add OTF/TTF font rendering support. (Implies `DRAW`)
-   Pass `BUTTON_SCAN=1` to add support for the Kobo-specific button scan stuff. (Implies `DRAW`)

If you *really* need *extreme* Unicode coverage in the fixed-cell codepath, you can also choose to embed GNU Unifont, by passing `UNIFONT=1`.  
Be warned that this'll add almost 2MB to the binary size, and that the font is actually split in two (double-wide glyphs are punted off to a specific font), which may dampen its usefulness in practice...  
For obvious reasons, this is *never* enabled by default.  
Unless you're doing *very* specific things, you generally want *at least* `DRAW` & `BITMAP` enabled in a `MINIMAL` build...

Along the way, a few auxiliary tools may crop up in the `utils` folder. `make utils` will do a static build of these (which is the recommended way to do it, as they rather crudely piggyback on FBInk's *internal* API). Currently, these consist of a diagnostic tool regarding rotation behavior, and the doom stress-test mentioned below.  
Most of these have *only* been tested on Kobo, and should probably be left alone unless you know what you're doing ;).  

A tool to properly manipulate the bitdepth on eInk devices is also available, and can be built for e-Ink targets with `make fbdepth`.  
Its uninspired name is [`fbdepth`](https://github.com/NiLuJe/FBInk/blob/master/utils/fbdepth.c), and it's used by [KOReader](https://github.com/koreader/koreader) on Kobo & reMarkable to enforce a sane rotation and switch to a more efficient bitdepth.
It has also been tested on Kindle, where rotation handling, at the very least, should be behaving properly. Do note that on FW 5.x, the stock GUI runs under X, and X will *not* like you rotating the fb from under its feet ;).  
If you want the smallest binary possible, make sure you build it alone, from a pristine source tree.

There's also a fairly stupid [example](https://github.com/NiLuJe/FBInk/blob/master/utils/dump.c) showcasing the dump/restore API that can be built via `make dump`.  
Another stupid [demo](https://github.com/NiLuJe/FBInk/blob/master/utils/doom.c) based on the PSX Doom fire effect was implemented, to stress-test the EPDC in a mildly interesting manner.  

If you ever were curious about the whole mxcfb alt_buffer shindig, you can take a look at this [PoC](https://github.com/NiLuJe/FBInk/blob/master/utils/alt_buffer.c).

In the same vein, if you're looking into rotation & input shenanigans on Kobo, `make devcap` will build a tarball containing a few binaries and a [devcap_test.sh script](https://github.com/NiLuJe/FBInk/blob/master/utils/devcap_test.sh), that, when run on the target device, will compile quite a bit of info. In particular, if you ever need to report a bug against `fbdepth`, I'll probably ask you to run that and attach the results to the issue ;).

And on the subject of input & rotation on Kobo, `make ftrace` will build a simple [pointer trail](https://github.com/NiLuJe/FBInk/blob/master/utils/finger_trace.c) utility, which leverages [libevdev](https://gitlab.freedesktop.org/libevdev/libevdev) and a few of our funkier API calls to try to make sense of the input translation shenanigans happening on Kobo.  
If you intend to handle touch input in any way in your code, this should be a good place to look ;).  
It also demonstrates how to deal effectively with pen input & drawing on the Elipsa.

## NOTES

Kindle support covers the full Kindle lineup, starting from the K2.

Kobo support covers the full Kobo lineup, starting from the Kobo Touch A/B/C.

BQ Cervantes support has been contributed by [@pazos](https://github.com/pazos) ([#17](https://github.com/NiLuJe/FBInk/pull/17)), and should handle the current lineup.

reMarkable support has been contributed by [@tcrs](https://github.com/tcrs) ([#41](https://github.com/NiLuJe/FBInk/pull/41)).

PocketBook support was tested by [@ezdiy](https://github.com/ezdiy) ([#47](https://github.com/NiLuJe/FBInk/pull/47)), and should support the same set of devices as KOReader.

If, instead of *writing* to the framebuffer, you want to *grab* a PNG snapshot of it (which can come in handy), I have a heavily modified version of [FBGrab](http://trac.ak-team.com/trac/browser/niluje/Configs/trunk/Kindle/Misc/FBGrab) that should sanely deal with the various quirks of eInk framebuffers ;).

## Bindings in other languages

So that everyone gets to have fun, even if you can't stand C!

[Go](https://golang.org/): [go-fbink](https://github.com/shermp/go-fbink) and its successor [go-fbink-v2](https://github.com/shermp/go-fbink-v2) by [@shermp](https://github.com/shermp)

[LuaJIT](https://luajit.org/): [lua-fbink](https://github.com/NiLuJe/lua-fbink) by [@NiLuJe](https://github.com/NiLuJe)

[Python](https://www.python.org/): [py-fbink](https://github.com/NiLuJe/py-fbink) by [@NiLuJe](https://github.com/NiLuJe)

Note that as the API may not be entirely stable on master, these are all tethered to a specific tag (generally, the latest release). You should honor that requirement, or all hell will break loose ;).  
I generally attempt to keep breakages to a minimum, or barring that, make the upgrade paths as painless as possible, but, there you have it, supporting new stuff often means existing stuff has to work slightly differently.

I try to detail API/ABI breakages in each tag's comments, but a good way to visualize that is of course to diff the single public header (or, for a quick contextless overview, the [minimal headers generated for FFI bindings](https://github.com/NiLuJe/lua-fbink/commit/a467e796ca6b11119f450527fa211baa7de7307d)) ;).

<!-- kate: indent-mode cstyle; indent-width 4; replace-tabs on; remove-trailing-spaces none; -->
