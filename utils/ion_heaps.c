/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2023 NiLuJe <ninuje@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later

	----

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// Stupid tool to validate the strace decoding of ION_IOC_HEAP_QUERY
// Inspired by https://unix.stackexchange.com/a/503505

// Because we're pretty much Linux-bound ;).
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>

// We need ION ;).
#include "../eink/ion-kobo.h"

// c.f., https://android.googlesource.com/platform/system/memory/libion/+/master/ion.c
static int
    ion_query_heap_cnt(int fd, uint32_t* cnt)
{
	if (!cnt) {
		return -EINVAL;
	}

	struct ion_heap_query query = { 0 };

	int ret = ioctl(fd, ION_IOC_HEAP_QUERY, &query);
	if (ret < 0) {
		return -(errno);
	}

	*cnt = query.cnt;

	return ret;
}

static int
    ion_query_get_heaps(int fd, uint32_t cnt, void* buffers)
{
	if (!buffers) {
		return -EINVAL;
	}

	struct ion_heap_query query = {
		.cnt   = cnt,
		.heaps = (uint64_t) (uintptr_t) buffers,
	};

	int ret = ioctl(fd, ION_IOC_HEAP_QUERY, &query);
	if (ret < 0) {
		return -(errno);
	}

	return ret;
}

int
    main(void)
{
	int                   ret  = EXIT_SUCCESS;
	struct ion_heap_data* data = NULL;

	// Register as an ION client
	int fd = open("/dev/ion", O_RDONLY | O_CLOEXEC);
	if (fd == -1) {
		perror("open");
	}

	// Start with a buffer-less query ioctl to get the heap count
	uint32_t cnt = 0;
	ret          = ion_query_heap_cnt(fd, &cnt);
	if (ret != EXIT_SUCCESS) {
		fprintf(stderr, "Failed to query ION heap count: %m\n");
		goto cleanup;
	}

	// Now that we have a count, allocate our buffer accordingly...
	data = calloc(cnt, sizeof(*data));
	if (!data) {
		fprintf(stderr, "Failed to allocate ION heap report buffer: %m\n");
		goto cleanup;
	}

	// And query it...
	ret = ion_query_get_heaps(fd, cnt, (void*) data);
	// NOTE: There's a fun bug in the Elipsa kernel,
	//       where ion_query_heaps never actually sets the return value to 0 on success...
	//       It only does so for bufferless queries :s.
	// NOTE: It was only fixed for Linux 4.12,
	//       which is also the kernel where this all mess became useless because of the new dmabuff API :D
	//       c.f., https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/drivers/staging/android/ion/ion.c?h=v4.12&id=804ca94a98237e12ad8fabf7cdc566f14b05b7df
	if (ret != EXIT_SUCCESS && ret != -(EINVAL)) {
		fprintf(stderr, "Failed to query ION heap report: %m\n");
		goto cleanup;
	} else {
		for (size_t i = 0U; i < cnt; i++) {
			struct ion_heap_data* heap = &data[i];
			printf("Heap %zu of %u\n", i + 1U, cnt);
			printf("Name: %.*s\n", MAX_HEAP_NAME, heap->name);
			printf("Type: %u\n", heap->type);
			printf("ID: %u\n", heap->heap_id);
			printf("\n");
		}
	}

cleanup:
	// Release resources
	free(data);
	if (fd != -1) {
		close(fd);
	}

	return ret;
}
