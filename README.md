# FBInk
FrameBuffer eInker

Licensed under the [AGPLv3](/LICENSE).
Housed [here on GitHub](https://github.com/NiLuJe/FBInk).

# What's it for?

This is intended to fill the void felt by Kobo developpers and tinkerers when they realize they do not have a builtin way to print stuff on the device's screen!
It's especially cruel when moving to a Kobo, after being used to the ubiquity of `eips` on Kindles...

In short, it prints messages on your screen, handling the low-level tinkering with both the Linux framebuffer interface, and the iMX EPD driver.
It's been tested on Kobos and Kindles, but porting it to other Linux, iMX eInk devices should be trivial.

# How do I install this?

There's a discussion thread open [over here](https://www.mobileread.com/forums/showthread.php?t=299110) on MobileRead, where you'll happen to find standalone binaries.
It's purposefully lacking in detailed instructions, because the target audience is mainly developpers and tinkerers. Think of this as a safety precaution ;).

As an example of usage in the wild, see [KFMon](https://github.com/NiLuJe/kfmon), where I'm using it to provide visual feedback.

# How can I tinker with it?

The tool is available both as a commandline utility, and as a shared or static library for C projects (beware, though, it's licensed under the AGPLv3+, not the LGPL).
See the [public header](fbink.h) for basic API usage.
Launch the `fbink` tool with no argument for a quick manual.

NOTE: It currently makes absolutely *NO* attempt at handling rotation, because that currently appears to be the right thing to do with current Kobo FW versions.
YMMV on older FW, or if something else is fudging with fb rotation, or if your application is implementing rotation in software (i.e., a rotated viewport).

# How does it look?

Like this :).

![FBInk on a Kobo H2O](https://raw.githubusercontent.com/NiLuJe/FBInk/master/fbink_readme.png)

(Although the fontsize may be different in current releases. On the H2O shown as an example, it's now 24x24 vs. the 32x32 showcased here.)

# NOTES

Kindle support is split into Touch devices, and legacy devices (up to, and *including* the K4). This is because older devices use a completely different eInk controller.

Kobo support has been tested on a H2O running a recent FW version (i.e., 32bpp fb).
