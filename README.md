# FBInk
FrameBuffer eInker

Licensed under the [AGPLv3](/LICENSE).
Housed [here on GitHub](https://github.com/NiLuJe/FBInk).

# What's it for?

This is intended to fill the void felt by Kobo developpers and tinkerers when they realize they do not have a builtin way to print stuff on the device's screen!
It's especially cruel when moving to a Kobo, after being used to the ubiquity of `eips` on Kindle...

In short, it prints messages or images on your screen, handling the low-level tinkering with both the Linux framebuffer interface, and the i.MX EPD driver.
It's been tested on Kobo, Kindle and BQ Cervantes, but porting it to other Linux, i.MX eInk devices should be trivial (hell, even Sipix support shouldn't be too hard).

By default, text rendering relies on bundled fixed cell bitmap fonts ([see this post](https://www.mobileread.com/forums/showpost.php?p=3765426&postcount=31) for a small sampling),
but thanks to [@shermp](https://github.com/shermp)'s contributions ([#20](https://github.com/NiLuJe/FBInk/pull/20)), you can also rely on full-fledged TrueType/OpenType font rendering!

It also happens to work perfectly fine on *any* kind of Linux framebuffer device, and supports a wide range of bitdepths (4bpp, 8bpp, 16bpp, 24bpp & 32bpp),
so you could use this to draw on your EFI fb, for instance ;).

# How do I install this?

For Kobo devices, there's a discussion thread open [over here](https://www.mobileread.com/forums/showthread.php?t=299110) on MobileRead, where you'll happen to find standalone binaries.
It's purposefully lacking in detailed instructions, because the target audience is mainly developpers and tinkerers. Think of this as a safety precaution ;).

There's also a sister thread for Kindle devices [over here](https://www.mobileread.com/forums/showthread.php?t=299620) where, besides binaries, you'll also find examples of people doing crazy things with it ;).

As an example of usage in the wild, see [KFMon](https://github.com/NiLuJe/kfmon), where I'm using it to provide visual feedback, or [kobo-rclone](https://github.com/shermp/kobo-rclone), where it's also used for screen scraping. We're also using it in [KOReader](https://github.com/koreader/koreader), to make the OTA update process more user-friendly.

# How can I tinker with it?

The tool is available both as a commandline utility, and as a shared or static library for C projects (beware, though, it's licensed under the AGPLv3+, not the LGPL).
See the [public header](fbink.h) for basic API usage.
Launch the `fbink` tool with no argument for a quick manual & rundown of its capabilities.

NOTE: It generally makes *NO* attempt at handling software rotation, because that currently appears to be the right thing to do with both current Kobo FW versions and on Kindle.
YMMV on older FW, or if something else is fudging with fb rotation, or if your application is implementing rotation in software (i.e., a rotated viewport).
As far as hardware rotation is concerned, there are a few specific exceptions made for Kobo devices:
- Those running in 16bpp mode and appearing to be in landscape mode: since that seems to be their native state, we *attempt* to compensate for this,
as we can legitimately be used before Nickel itself corrects this.
- On devices with an accelerometer, like the Forma, where Nickel itself will handle the hardware rotation.

# How does it look?

Like this for its text printing facilities :).

![FBInk on a Kobo H2O](https://raw.githubusercontent.com/NiLuJe/FBInk/master/resources/fbink_readme.png)

And like this when displaying an image :).

![FBInk 1.2.0 on a Kobo H2O](https://raw.githubusercontent.com/NiLuJe/FBInk/master/resources/fbink_image.png)

And with all the bells and whistles of working transparency, even on ancient hardware :).

![FBInk 1.2.5 on a Kindle 3](https://raw.githubusercontent.com/NiLuJe/FBInk/master/resources/fbink_alpha.png)

And with a few other fonts, as well as a progress bar...

![FBInk 1.6.2 on a Kobo H2O](https://raw.githubusercontent.com/NiLuJe/FBInk/master/resources/fbink_bars.png)

And when using shiny TrueType fonts :).

![FBInk 1.8.0 on a Kobo H2O](https://raw.githubusercontent.com/NiLuJe/FBInk/master/resources/fbink_ot.png)

# How do I build it?

Unless you're just trying to take it for a spin on a native pure Linux system (`make linux`), you'll need a cross-compiler targeting your, well, target device.
The Makefile is tailored to automatically detect my own cross-compilation [ToolChain](http://trac.ak-team.com/trac/browser/niluje/Configs/trunk/Kindle/Misc/x-compile.sh) [setups](https://github.com/koreader/koxtoolchain), which I evidently heartily recommend using instead of relying on generic cross-compilation toolchains which may not exactly target the right kernel/libc duo ;).

In case you're using your own toolchain, please note that we require C11 support (GCC >= 4.9, Clang >= 3.0).

With that out of the way, the default target (i.e., `make`) will yield a static Kobo build, while `make kobo` will yield a stripped shared build, and additionally package everything the Kobo way. The package found in the Kobo thread is built that way.

There's a few convenience targets for usual build types (`make static` for a static build, `make shared` for a shared build, `make strip` for a stripped static build, `make release` for a stripped shared build, `make debug` for a debug build), as well as a few unusual ones for very specific use cases, usually related to FFI bindings (`make pic` for a PIC static build, or passing `STATIC_LIBM=1` to make to attempt to link against libm statically).

The choice of target platform is handled via a simple variable:
- Pass `KINDLE=1` to make for a Kindle build (`make kindle` does that on a stripped static build)
- Pass `KINDLE=1 LEGACY=1` to make for a FW 2.x Kindle build (`make legacy` does that on a stripped static build). This basically just disables CLOEXEC, which might not be supported on FW 2.x.
- Pass `CERVANTES=1` to make for a BQ/Cervantes build (`make cervantes` does that on a stripped static build)

The same logic is used to allow for a bit of tailoring:
- Pass `MINIMAL=1` to make for a build with limited functionality (only fixed cell font rendering, no image rendering, no extra fonts, no OpenType), which yields a much smaller application & library.
- Pass `DEBUG=1` to make for a Debug build, and pass `DEBUG=1 DEBUGFLAGS=1` to make for a Debug build with enforced debug CFLAGS.

# NOTES

Kindle support covers the full Kindle lineup, starting from the K2.

Kobo support has been tested on a H2O running a recent FW version (i.e., 32bpp fb), but the full lineup should be supported.

BQ Cervantes support has been contributed by [@pazos](https://github.com/pazos) ([#17](https://github.com/NiLuJe/FBInk/pull/17)), and should handle the current lineup.

# Bindings in other languages

So that everyone gets to have fun, even if you can't stand C!

[Go](https://golang.org/): [go-fbink](https://github.com/shermp/go-fbink) and its successor [go-fbink-v2](https://github.com/shermp/go-fbink-v2) by [@shermp](https://github.com/shermp)
