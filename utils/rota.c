#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

int fbfd = -1;

static const char*
    fb_rotate_to_string(uint32_t rotate)
{
	switch (rotate) {
		case FB_ROTATE_UR:
			return "Upright, 0°";
		case FB_ROTATE_CW:
			return "Clockwise, 90°";
		case FB_ROTATE_UD:
			return "Upside Down, 180°";
		case FB_ROTATE_CCW:
			return "Counter Clockwise, 270°";
		default:
			return "Unknown?!";
	}
}

static void
    get_fbinfo(struct fb_var_screeninfo* vInfo, struct fb_fix_screeninfo* fInfo)
{
	// Get variable fb info
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, vInfo)) {
		perror("ioctl GET_V");
	}
	fprintf(stdout,
		"Variable fb info: %ux%u, %ubpp @ rotation: %u (%s)\n",
		vInfo->xres,
		vInfo->yres,
		vInfo->bits_per_pixel,
		vInfo->rotate,
		fb_rotate_to_string(vInfo->rotate));
	// Get fixed fb information
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, fInfo)) {
		perror("ioctl GET_F");
	}
	fprintf(stdout,
		"Fixed fb info: ID is \"%s\", length of fb mem: %u bytes & line length: %u bytes\n",
		fInfo->id,
		fInfo->smem_len,
		fInfo->line_length);
}

static void
    set_fbinfo(int rota, struct fb_var_screeninfo* vInfo)
{
	// Set variable fb info
	vInfo->rotate = (uint32_t) rota;
	fprintf(stdout, "Setting rotate to %u (%s)\n", vInfo->rotate, fb_rotate_to_string(vInfo->rotate));

	if (ioctl(fbfd, FBIOPUT_VSCREENINFO, vInfo)) {
		perror("ioctl PUT_V");
	}

	fprintf(stdout, "Rotate is now %u (%s)\n", vInfo->rotate, fb_rotate_to_string(vInfo->rotate));
}

static void
    do_eet(int rota, struct fb_var_screeninfo* vInfo, struct fb_fix_screeninfo* fInfo)
{
	set_fbinfo(rota, vInfo);
	sleep(1);
	get_fbinfo(vInfo, fInfo);
	sleep(1);
}

int
    main(void)
{
	struct fb_var_screeninfo vInfo = { 0 };
	struct fb_fix_screeninfo fInfo = { 0 };

	fbfd = open("/dev/fb0", O_RDWR | O_CLOEXEC);
	if (!fbfd) {
		perror("open");
	}

	// Print initial status
	get_fbinfo(&vInfo, &fInfo);

	// let's check how quirky it is...
	fprintf(stdout, "\nFB_ROTATE_UR to FB_ROTATE_CCW, +1 increments\n");
	for (int i = FB_ROTATE_UR; i <= FB_ROTATE_CCW; i++) {
		do_eet(i, &vInfo, &fInfo);
	}

	// Now we'll try to break it...
	fprintf(stdout, "\nFB_ROTATE_UR to FB_ROTATE_CCW, +2 increments\n");
	for (int i = FB_ROTATE_UR; i <= FB_ROTATE_CCW; i += 2) {
		do_eet(i, &vInfo, &fInfo);
	}
	fprintf(stdout, "\nFB_ROTATE_CW to FB_ROTATE_CCW, +2 increments\n");
	for (int i = FB_ROTATE_CW; i <= FB_ROTATE_CCW; i += 2) {
		do_eet(i, &vInfo, &fInfo);
	}

	/*
	// Try doubling the ioctls?
	fprintf(stdout, "\nFB_ROTATE_UR to FB_ROTATE_CCW, +2 increments, dual ioctls\n");
	for (int i = FB_ROTATE_UR; i <= FB_ROTATE_CCW; i += 2) {
		do_eet(i, &vInfo, &fInfo);
		do_eet(i, &vInfo, &fInfo);
	}
	fprintf(stdout, "\nFB_ROTATE_CW to FB_ROTATE_CCW, +2 increments, dual ioctls\n");
	for (int i = FB_ROTATE_CW; i <= FB_ROTATE_CCW; i += 2) {
		do_eet(i, &vInfo, &fInfo);
		do_eet(i, &vInfo, &fInfo);
	}

	// Try doing weird shit?
	fprintf(stdout, "\nFB_ROTATE_UR to FB_ROTATE_CCW, +2 increments, set ^ 2\n");
	for (int i = FB_ROTATE_UR; i <= FB_ROTATE_CCW; i += 2) {
		do_eet(i ^ 2, &vInfo, &fInfo);
	}
	fprintf(stdout, "\nFB_ROTATE_CW to FB_ROTATE_CCW, +2 increments, set ^ 2\n");
	for (int i = FB_ROTATE_CW; i <= FB_ROTATE_CCW; i += 2) {
		do_eet(i ^ 2, &vInfo, &fInfo);
	}
	fprintf(stdout, "\nFB_ROTATE_UR to FB_ROTATE_CCW, +2 increments, set ^ 2, dual ioctls\n");
	for (int i = FB_ROTATE_UR; i <= FB_ROTATE_CCW; i += 2) {
		do_eet(i ^ 2, &vInfo, &fInfo);
		do_eet(i ^ 2, &vInfo, &fInfo);
	}
	fprintf(stdout, "\nFB_ROTATE_CW to FB_ROTATE_CCW, +2 increments, set ^ 2, dual ioctls\n");
	for (int i = FB_ROTATE_CW; i <= FB_ROTATE_CCW; i += 2) {
		do_eet(i ^ 2, &vInfo, &fInfo);
		do_eet(i ^ 2, &vInfo, &fInfo);
	}
	*/

	// And let's try to fix it, now...
	fprintf(stdout, "\nFB_ROTATE_UR to FB_ROTATE_CCW, +2 increments, intermerdiary rota if ==\n");
	for (int i = FB_ROTATE_UR; i <= FB_ROTATE_CCW; i += 2) {
		// If current rotate = to be set value, set += 1 (wrapping at 4) first to swap portrait/landscape
		if (vInfo.rotate == (uint32_t) i) {
			fprintf(stdout, "Intermerdiary rotation...\n");
			do_eet((i + 1) % 4, &vInfo, &fInfo);
			fprintf(stdout, "Requested rotation\n");
			do_eet(i, &vInfo, &fInfo);
		} else {
			do_eet(i, &vInfo, &fInfo);
		}
	}
	fprintf(stdout, "\nFB_ROTATE_CW to FB_ROTATE_CCW, +2 increments, intermerdiary rota if ==\n");
	for (int i = FB_ROTATE_CW; i <= FB_ROTATE_CCW; i += 2) {
		// If current rotate = to be set value, set += 1 (wrapping at 4) first to swap portrait/landscape
		if (vInfo.rotate == (uint32_t) i) {
			fprintf(stdout, "Intermerdiary rotation...\n");
			do_eet((i + 1) % 4, &vInfo, &fInfo);
			fprintf(stdout, "Requested rotation\n");
			do_eet(i, &vInfo, &fInfo);
		} else {
			do_eet(i, &vInfo, &fInfo);
		}
	}

	close(fbfd);
}
