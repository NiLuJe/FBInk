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

Right now, you don't. It's mostly aimed at developpers.
While I'll probably end up distributing binaries somewhere on MobileRead, it probably won't be neatly packaged for end-users, the target audience *is* developpers.

For exampke, I plan on using it in [KFMon](https://github.com/NiLuJe/kfmon) to provide visual feedback.

# How can I tinker with it?

The tool is available both as a commandline utility, and as a shared or static library for C projects (beware, though, it's licensed under the AGPLv3, not the LGPL).
See the [public header](fbink.h) for basic API usage.
Launch the `fbink` tool with no argument for a quick manual.

# How does it look?

Like this :).

![FBInk on a Kobo H2O](https://raw.githubusercontent.com/NiLuJe/FBInk/master/fbink_readme.png)
