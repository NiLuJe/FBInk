# FBInk Command Line App Manual

## Synopsis

* ```sh
  fbink [OPTIONS] [STRING ...]
  ```

  Print `STRING`s on your device's screen.

* ```sh
  fbink [-fcWDHbhxyS] --image file=PATH,x=NUM,y=NUM,halign=ALIGN,valign=ALIGN,w=NUM,h=NUM,dither [--img PATH]
  ```

  Print image on your device's screen. // TODO: verify allowed flags in this usage scenario

* ```sh
  fbink [-WHf XXXXX] [-k] --refresh [top=NUM,left=NUM,width=NUM,height=NUM]
  ```

  Refresh the screen as per your specification, without touching the framebuffer. // TODO, exhaustively list options that are compatible with `--refresh`.

* ```sh
  fbink [-WHf XXXXX] --cls [top=NUM,left=NUM,width=NUM,height=NUM]
  ```

  Clear the screen (or a region of it). // TODO, exhaustively list options that are compatible with `--refresh`.

## Examples

* ```sh
  fbink -x 1 -y 10 "Hello World!"
  ```

  Prints 'Hello World!' on the eleventh line, starting at the second column from the left.

* ```sh
  fbink -pmh -y -5 "Hello World!"
  ```

  Prints 'Hello World!', highlighted (i.e., white on black with the default colors), centered & padded on both sides, on the fifth line starting from the bottom.

* ```sh
  fbink -pmM -y -8 "Hello World!"
  ```

  Prints 'Hello World!', centered & padded on both sides, eight lines above the center of the screen.

## Options

### Options affecting the message's position on screen

* `-x`, `--col` `NUM`

  Begin printing `STRING` @ column `NUM` (Default: 0).

  You might consider beginning at column 1 instead of 0, as column 0 (the leftmost one) may sometimes be slightly obscured by a bezel.

  Use a negative value to count back from the right edge of the screen.

* `-y`, `--row` `NUM`

  Begin printing `STRING` @ row `NUM` (Default: 0).

  You might consider beginning at row 1 instead of 0, as row 0 (the topmost one) may sometimes be slightly obscured by a bezel, especially on Kobos.

  Use a negative value to count back from the bottom of the screen.

* `-X`, `--hoffset` `NUM`

  Adjust final text position by `NUM` pixels on the horizontal axis (Default: 0).

  Honors negative values, and will let you push stuff off-screen, often without warning.

* `-Y`, `--voffset` `NUM`

  Adjust final text position by `NUM` pixels on the vertical axis (Default: 0).

  Honors negative values, and will let you push stuff off-screen, often without warning.

* `-m`, `--centered`

  Dynamically override col to print `STRING` at the center of the screen.

  Special care is taken to avoid the very edges of the screen, to ensure the complete legibility of the message.

* `-M`, `--halfway`

  Dynamically adjust row to print `STRING` in the middle of the screen.

  The value specified in row then becomes an offset, starting from the middle of the screen.

* `-p`, `--padded`

  Left pad `STRING` with blank spaces.

  Most useful when combined with --centered to ensure a line will be completely filled, while still centering `STRING`, i.e., padding it on both sides.

* `-r`, `--rpadded`

  Right pad `STRING` with blank spaces.

### Options affecting the message's appearance

* `-h`, `--invert`

  Print `STRING` in `BACKGROUND_COLOR` over `FOREGROUND_COLOR` instead of the reverse. (See `-B` and `-C`)

* `-f`, `--flash`

  Ask the eInk driver to do a black flash when refreshing the area of the screen where `STRING` will be printed.

  Note if compiled with `FBINK_FOR_KINDLE`: on legacy einkfb devices, this may not always be honored by the hardware.

* `-c`, `--clear`

  Clear the full screen before printing.

  Honors `-B`, `--background`; `-h`, `--invert`; `-H`, `--nightmode`; `-W`, `--waveform`; `-D`, `--dither`; `-b`, `--norefresh`; `-w`, `--wait`

  Can be specified on its own, without any `STRING`.

  NOTE: If your intent is to simply clear the screen and *nothing else*, use `-k`, `--cls` instead!

* `-W`, `--waveform` `MODE` (not available if compiled with `FBINK_FOR_LINUX`)

  Request a specific waveform update mode from the eInk controller, if supported (mainly useful for images).

  Available waveform modes: `A2`, `DU`, `GL16`, `GC16` &amp; `AUTO`

  Note if compiled with `FBINK_FOR_KINDLE`: as well as `REAGL`, `REAGLD`, `GC16_FAST`, `GL16_FAST`, `DU4`, `GL4`, `GL16_INV`, `GCK16` & `GLKW16` on some Kindles, depending on the model & FW version. Note that specifying a waveform mode is ignored on legacy einkfb devices, because the hardware doesn't expose such capabilities.

  Note if compiled with `FBINK_FOR_POCKETBOOK`: as well as `GC4`, `A2IN`, `A2OUT`, `DU4`, `REAGL`, `REAGLD`, `GC16HQ` & `GS16`.

  Note if compiled with `FBINK_FOR_KOBO` or `FBINK_FOR_CERVANTES`: as well as `GC4`, `REAGL` & `REAGLD`.

  Note if not compiled with `FBINK_FOR_REMARKABLE`: Unsupported modes *should* safely downgrade to `AUTO`. Operative word being "should" ;). On some devices, `REAGL` & `REAGLD` expect to be flashing in order to behave properly.

  Note if compiled with `FBINK_FOR_POCKETBOOK`: On devices with a B288 SoC, `AUTO` is *not* supported. FBInk will silently use `GC16` instead!

* `-D`, `--dither` (not available if compiled with `FBINK_FOR_LINUX`)

  Available dithering modes: `PASSTHROUGH`, `FLOYD_STEINBERG`, `ATKINSON`, `ORDERED`, `QUANT_ONLY` & `LEGACY`.

  Note that this is only supported on recent devices, and that only a subset of these options may actually be supported by the HW (usually, `PASSTHROUGH` & `ORDERED`, check dmesg).

  `LEGACY` may be supported on more devices, but what exactly it does in practice (and how well it works) depends on the exact device and/or FW version.

  Note if compiled with `FBINK_FOR_KINDLE`: True (i.e., not `LEGACY`) hardware dithering is completely untested on Kindle, and, while the Oasis 2, PaperWhite 4 & Oasis 3 *should* support it, they *may* not, or at least not in the way FBInk expects...

* `-H`, `--nightmode` (not available if compiled with `FBINK_FOR_LINUX`)

  Request full hardware inversion from the eInk controller, if supported.

  Note that this can be used *in combination* with `-h`, `--invert`! One does not exclude the other, which may lead to some confusing behavior ;).

  Note if compiled with `FBINK_FOR_KINDLE`: Note that requesting nightmode is ignored on legacy einkfb devices, because the hardware doesn't (easily) expose such capabilities.

  Note that this may be ignored on some specific devices where it is known to be or have been unstable at some point.

* `-b`, `--norefresh` (not available if compiled with `FBINK_FOR_LINUX`)

  Only update the framebuffer, but don't actually refresh the eInk screen (useful when drawing in batch).

* `-w`, `--wait` (not available if compiled with `FBINK_FOR_LINUX`)

  Block until the kernel has finished processing the *last* update we sent, if any.

  The actual delay depends for the most part on the waveform mode that was used.

  See the API documentation around `fbink_wait_for_submission` & `fbink_wait_for_complete` for more details.

  As a point of reference, eips only does a `wait_for_complete` after the flashing refresh of an image. We used to do that by default for *all* flashing updates until FBInk 1.20.0.

* `-S`, `--size`

  Override the automatic font scaling multiplier (Default: 0, automatic selection, ranging from 1 (no scaling), to 4 (4x upscaling), depending on screen resolution).

  Note if compiled with `FBINK_WITH_FONTS`: Note that user-supplied values will be clamped to safe boundaries (from 1 to around 45 for most fonts, and from 1 to around 30 for `TALL`).

  Note if not compiled with `FBINK_WITH_FONTS`: Note that user-supplied values will be clamped to safe boundaries (from 1 to around 45).

  The exact upper value depends on the resolution of your screen.

* `-F`, `--font` `NAME`

  Render glyphs from builtin font `NAME` (Default: IBM).

  Note if compiled with `FBINK_WITH_FONTS`: Available font families: `IBM`, `UNSCII`, `ALT`, `THIN`, `FANTASY`, `MCR`, `TALL`, `BLOCK`, `LEGGIE`, `VEGGIE`, `KATES`, `FKP`, `CTRLD`, `ORP`, `ORPB`, `ORPI`, `SCIENTIFICA`, `SCIENTIFICAB`, `SCIENTIFICAI`, `TERMINUS`, `TERMINUSB`, `FATTY`, `SPLEEN`, `TEWI`, `TEWIB`, `TOPAZ`, `MICROKNIGHT`, `VGA`, `COZETTE`

  Note if compiled with `FBINK_WITH_UNIFONT`: as well as `UNIFONT` & `UNIFONTDW`

  NOTE: On low dpi, 600x800 devices, `ORP` or `TEWI`'s form factor may feel more familiar at default scaling.

  Note if not compiled with `FBINK_WITH_FONTS`: Available font families: `IBM`

  Note if compiled with `FBINK_WITH_OPENTYPE`: NOTE: If you're looking for vector font rendering, see the OpenType section a few lines down!

* `-C`, `--color` `NAME`

  `-B`, `--background` `NAME`

  Color of the background the text will be printed on (Default: WHITE).

  Color the text will be printed in (Default: BLACK).

  Available colors: `BLACK`, `GRAY1`, `GRAY2`, `GRAY3`, `GRAY4`, `GRAY5`, `GRAY6`, `GRAY7`, `GRAY8`, `GRAY9`, `GRAYA`, `GRAYB`, `GRAYC`, `GRAYD`, `GRAYE`, `WHITE`.

* `-o`, `--overlay`

  Don't draw background pixels, and compute foreground pixel color based on the inverse of the underlying framebufer pixel.

  Obviously ignores `-h`, `--invert` & `-C`, `--color` *as far as glyphs are concerned*. `-B`, `--background` is still honored if you combine this with `-c`, `--clear`.

* `-O`, `--bgless`

  Don't draw background pixels.

  Obviously mutually exclusive with `-o`, `--overlay`, because it's simply a subset of what overlay does. If both are enabled, `-o`, `--overlay` takes precedence.

* `-T`, `--fgless`

  Don't draw foreground pixels.

  Mutually exclusive with `-o`, `--overlay` or `-O`, `--bgless`, and takes precedence over them.

### Options affecting the program's verbosity

* `-v`, `--verbose`

  Toggle printing diagnostic messages.

* `-q`, `--quiet`

  Toggle hiding hardware setup messages.

* `-G`, `--syslog`

  Send output to syslog instead of stdout & stderr.

  Ought to be the first flag passed, otherwise, some commandline parsing errors might not honor it.

### Options affecting the program's behavior

* `-I`, `--interactive`

  Enter a very basic interactive mode.

* `-L`, `--linecountcode`

  When successfully printing text, returns the total amount of printed lines as the process exit code.

  NOTE: Will be inaccurate if there are more than 255 rows on screen!

* `-l`, `--linecount`

  When successfully printing text, outputs the total amount of printed lines in the final line of output to stdout (NOTE: enforces quiet & non-verbose!).

  NOTE: With OT/TTF rendering, will output a top margin value to use as-is instead (or 0 if there's no space left on screen)! The OT/TTF codepath also returns more data, including the results of the line-breaking computations, so it's in an eval-friendly format instead.

* `-E`, `--coordinates`

  When printing something, outputs the coordinates & dimensions of what was printed to stdout, in a format easily consumable by eval (NOTE: enforces quiet & non-verbose!).

  NOTE: For both `-l`, `--linecount` & `-E`, `--coordinates`, output will only be sent to stdout on *success*. On error, the usual error message is sent to stderr.

  Given that, you may want to store stdout only in a variable and check the return code for success before running eval on that var!

* `-P`, `--progressbar` `NUM`

  Draw a `NUM` full progress bar (full-width).

  Like other alternative modes, does *NOT* have precedence over text printing.

  Ignores `-o`, `--overlay`; `-x`, `--col`; `-X`, `--hoffset`; as well as `-m`, `--centered` & `-p`, `--padded`.

* `-A`, `--activitybar` `NUM`

  Draw an activity bar on step `NUM` (full-width). `NUM` must be between 0 and 16. Like other alternative modes, does *NOT* have precedence over text printing.

  NOTE: If `NUM` is negative, will cycle between each possible value every 750ms, until the death of the sun! Be careful not to be caught in an involuntary infinite loop!

  Ignores `-x`, `--col`; `-X`, `--hoffset`; as well as `-m`, `--centered` & `-p`, `--padded`.

* `-V`, `--noviewport`

  Ignore any & all viewport corrections, be it from Kobo devices with rows of pixels hidden by a bezel, or a dynamic offset applied to rows when vertical fit isn't perfect.

### Option for only clearing & refreshing screen

* `-s` , `--refresh` `[top=NUM,left=NUM,width=NUM,height=NUM]`

  Eschew printing a STRING, and simply refresh the screen as per your specification, without touching the framebuffer.

  Honors `-W`, `--waveform`; `-H`, `--nightmode` & `-f`, `--flash`.

  The specified rectangle *must* completely fit on screen, or the ioctl will fail. 

  Note if compiled with `FBINK_FOR_KOBO` or `FBINK_FOR_CERVANTES`: the arguments are passed as-is to the ioctl, no viewport or rotation quirks are applied!

  If you just want a full-screen refresh (which will honor `-f`, `--flash`), don't pass any suboptions, e.g., `fbink -s` (if you group short options together, it needs to be the last in its group, i.e., `-fs` and not `-sf`).

  Specifying one or more `STRING` takes precedence over this mode.

  Example:

  * ```sh
    fbink -s top=20,left=10,width=500,height=600 -W GC16 -D ORDERED
    ```

    Refreshes a 500x600 rectangle with its top-left corner at coordinates (10, 20) with a GC16 waveform mode and `ORDERED` hardware dithering.

* `-k`, `--cls` `[top=NUM,left=NUM,width=NUM,height=NUM]` 

  Clear the screen (or a region of it), and abort early.

  Honors `-B`, `--background`; `-h`, `--invert`; `-H`, `--nightmode`; `-W`, `--waveform`; `-D`, `--dither`; `-b`, `--norefresh`; `-w`, `--wait`

  This takes precedence over *everything* and will abort as soon as it's done.

  If you just want a full-screen clear (which will honor `-f`, `--flash`), don't pass any suboptions,

  e.g., `fbink -k` (if you group short options together, it needs to be the last in its group, i.e., `-fk` and not `-kf`).

### Option for OpenType & TrueType font support (if compiled with `FBINK_WITH_OPENTYPE`)

* `-t`, `--truetype` `regular=FILE,bold=FILE,italic=FILE,bolditalic=FILE,size=NUM,px=NUM,top=NUM,bottom=NUM,left=NUM,right=NUM,padding=PAD,style=STYLE,format,notrunc,compute`

  - `regular`, `bold`, `italic` & `bolditalic` should point to the font file matching their respective font style. At least one of them MUST be specified.

  - `size` sets the rendering size, in points. Defaults to 12pt if unset. Can be a decimal value.

  - `px` sets the rendering size, in pixels. Optional. Takes precedence over size if specified.

  - `top`, `bottom`, `left` & `right` set the margins used to define the display area. Defaults to 0, i.e., the full screen, starting at the top-left corner.

    NOTE: If a negative value is supplied, counts backward from the opposite edge. Mostly useful with top & left to position stuff relative to the bottom right corner.

  - `padding` can optionally be set to ensure the drawing area on both sides of the printed content is padded with the background color on one or both axis.

    Available padding axis: `HORIZONTAL`, `VERTICAL`, or `BOTH` (Defaults to `NONE`). Useful to avoid overlaps on consecutive prints at the same coordinates.

  - If style is specified, it dictates the default font style to use (e.g., `REGULAR`, `BOLD`, `ITALIC` or `BOLD_ITALIC`). Defaults to `REGULAR`.

  - If `format` is specified, instead of the default style, the underscore/star Markdown syntax will be honored to set the font style (i.e., `*italic*`, `**bold**` & `***bold italic***`).

  - If `notrunc` is specified, truncation will be considered a failure.

    NOTE: This may not prevent drawing/refreshing the screen if the truncation couldn't be predicted at compute time!

    On the CLI, this will prevent you from making use of the returned computation info, as this will chain a CLI abort.

  - If `compute` is specified, no rendering will be done, and only the line-breaking computation pass will run. You'll generally want to use that combined with `-l`, `--linecount`.

  Honors `-h`, `--invert`; `-f`, `--flash`; `-c`, `--clear`; `-W`, `--waveform`; `-D`, `--dither`; `-H`, `--nightmode`; `-b`, `--norefresh`; `-m`, `--centered`; `-M`, `--halfway`; `-o`, `--overlay`; `-T`, `--fgless`; `-O`, `--bgless`; `-C`, `--color`; `-B`, `--background`; `-l`, `--linecount`.

  Examples:

  * If compiled with `FBINK_FOR_KINDLE`:

    ```sh
    fbink -t regular=/usr/java/lib/fonts/Caecilia_LT_65_Medium.ttf,bold=/usr/java/lib/fonts/Caecilia_LT_75_Bold.ttf,size=24,top=100,bottom=500,left=25,right=50,format "Hello **world**!"
    ```

    Will use Caecilia to print "Hello world!" at 24pt in a display area starting from 100px down the top of the screen to 500px before the bottom of the screen, from 25px of the left edge of the screen until 50px before the right edge. Honoring the Markdown syntax, "Hello" will be printed with the Regular font style, while "world" will use the Bold font style. You will NOT be able to use obfuscated or encrypted fonts.

  * If not compiled with `FBINK_FOR_KINDLE`:

    ```sh
    fbink -t regular=/mnt/onboard/fonts/NotoSans-Regular.ttf,bold=/mnt/onboard/fonts/NotoSans-Bold.ttf,size=24,top=100,bottom=500,left=25,right=50,format "Hello **world**!"
    ```

    Will use NotoSans to print "Hello world!" at 24pt in a display area starting from 100px down the top of the screen to 500px before the bottom of the screen, from 25px of the left edge of the screen until 50px before the right edge. Honoring the Markdown syntax, "Hello" will be printed with the Regular font style, while "world" will use the Bold font style. You will NOT be able to use obfuscated or encrypted fonts.

  * Note if compiled with `FBINK_FOR_KOBO`: this means you will NOT be able to use system fonts on Kobo, because they're all obfuscated.

### Options for printing an image (if compiled with `FBINK_WITH_IMAGE`)

* `-g`, `--image` `file=PATH,x=NUM,y=NUM,halign=ALIGN,valign=ALIGN,w=NUM,h=NUM,dither`

  * `PATH` has limitations on allowable values, see the `-i`, `--img` option below.
  * Supported `ALIGN` values: `NONE` (or `LEFT` for halign, `TOP` for valign), `CENTER` or `MIDDLE`, `EDGE` (or `RIGHT` for halign, `BOTTOM` for valign).
  * If `dither` is specified, *software* dithering (ordered, 8x8) will be applied to the image, ensuring it'll match the eInk palette exactly.
    * This is *NOT* mutually exclusive with `-D`, `--dither`!
  * `w` & `h` *may* be used to request scaling. If one of them is set to 0, aspect ratio will be respected.
    * Set to -1 to request the viewport's dimension for that side.
    * If either side is set to something lower than -1, the image will be scaled to the largest possible dimension that fits on screen while honoring the original aspect ratio.
    * They both default to 0, meaning no scaling will be done.

  This honors `-f`, `--flash`, as well as `-c`, `--clear`; `-W`, `--waveform`; `-D`, `--dither`; `-H`, `--nightmode`; `-b`, `--norefresh` & `-h`, `--invert`.

  This also honors `-x`, `--col` & `-y`, `--row` (taking `-S`, `--size` into account), in addition to the coordinates you specify.

  The aim is to make it easier to align small images to text. And to make pixel-perfect adjustments, you can also specifiy negative values for `x` & `y`.

  Specifying one or more `STRING` takes precedence over this mode.

  `-s`, `--refresh` also takes precedence over this mode.

  Examples:

  * ```sh
    fbink -g file=hello.png
    ```

    Displays the image \"hello.png\", starting at the top left of the screen.

  * ```sh
    fbink -i hello,world.png -g x=-10,y=11 -x 5 -y 8
    ```

    Displays the image "hello,world.png", starting at the ninth line plus 11px and the sixth column minus 10px.

  * ```sh
    fbink -g file=hello.png,halign=EDGE,valign=CENTER
    ```

    Displays the image "hello.png", in the middle of the screen, aligned to the right edge.

  * ```sh
    fbink -g file=hello.png -W A2
    ```

    Displays the image "hello.png", in monochrome.

  * ```sh
    fbink -i wheee.png
    ```

    Displays the image "wheee.png" with the default settings.

* `-i`, `--img` `PATH`

  This option implies `-g` , `--image` with the `name` flag set to `PATH`.

  This option can be combined with `-g` to set a `PATH` along with other flags. In fact, this the only way to specify file paths which contain a comma (`,`).

* `-a`, `--flatten`

  Ignore the alpha channel.

Notes:

* Supported image formats: JPEG, PNG, TGA, BMP, GIF & PNM
* In some cases, exotic encoding settings may not be supported.
* Transparency is supported, but it may be slightly slower (because we may need to do alpha blending).
  * You can use the `-a`, `--flatten` flag to avoid the potential performance penalty by always ignoring alpha.







## Notes about multiple string arguments

You can specify multiple `STRING`s in a single invocation of `fbink`, each consecutive one will be printed on the subsequent line.

Although it's worth mentioning that this will lead to undesirable results when combined with `--clear`, because the screen is cleared before each `STRING`, meaning you'll only get to see the final one.

If you want to properly print a long string, better do it in a single argument, FBInk will do its best to spread it over multiple lines sanely. It will also honor the linefeed character (and I do mean the actual control character, not the human-readable escape sequence), which comes in handy when passing a few lines of logs straight from tail as an argument.

## Notes for shell script writers

Shell script writers can also use the `-e`, `--eval` flag to have FBInk just spit out a few of its internal state variables to stdout, e.g., `eval $(fbink -e)`.

## Daemon mode

For more complex & long-running use-cases involving *text* only (or a progress/activity bar), you can also switch to daemon mode.

* `-d`, `--daemon` `NUM_LINES`

  `NUM_LINES` is the amount of lines consecutive prints can occupy before wrapping back to the original coordinates. It it's set to 0, the behavior matches what usually happens when you pass multiple strings to FBInk (i.e., the only wrapping happens at screen egde). While, for example, setting it to 1 will ensure every print will start at the same coordinates.

  In this mode, FBInk will daemonize instantly, and then print its PID to stdout. You should consume stdout, and check the return code:

  * if it's 0, then you have a guarantee that what you've grabbed from stdout is *strictly* a PID.
  * You can then send a kill -0 to that PID to check for an early abort.

  By default, it will create a named pipe for IPC, check the pipe name `FBINK_PIPE` with `fbink -e`, (if the file already exists, whatever type it may be, FBInk will abort). You can ask for a custom path by setting `FBINK_NAMED_PIPE` to an absolute path in your environment. Creating and removing that FIFO is FBInk's responsibility. Don't create it yourself.

  Make sure you kill FBInk via SIGTERM so it has a chance to remove it itself on exit. (Otherwise, you may want to ensure that it doesn't already exist *before* launching a daemon mode session). 

  Remember that LFs are honored!

  Also, the daemon will NOT abort on FBInk errors, and it redirects stdout & stderr to /dev/null, so errors & bogus input will be silently ignored!

  With the technicalities out of the way, it's then as simple as writing to that pipe for stuff to show up on screen ;).

  It can abort on early setup errors, though, before *or* after having redirected stderr...

  It does enforce logging to the syslog, though, but, again, early commandline parsing errors may still be sent to stderr...

  Example:

  * ```sh
    echo -n 'Hello World!' > $FBINK_PIPE
    ```
