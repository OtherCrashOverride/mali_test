#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "Stopwatch.h"


int main()
{
    // Open devices
    int fd = open("/dev/fb0", O_RDWR);
    if (fd < 0)
    {
        printf("open /dev/fb0 failed.\n");
        abort();
    }


    // Clear the screen
    fb_fix_screeninfo fixed_info;
    if (ioctl(fd, FBIOGET_FSCREENINFO, &fixed_info) < 0)
    {
        printf("FBIOGET_FSCREENINFO failed.\n");
        abort();
    }

    unsigned int* framebuffer = (unsigned int*)mmap(NULL,
        fixed_info.smem_len,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0);


    const uint32_t color = 0xff000000; // RGBA
    const uint8_t a = ((color & 0xff000000) >> 24);
    const uint8_t b = ((color & 0x00ff0000) >> 16);
    const uint8_t g = ((color & 0x0000ff00) >> 8);
    const uint8_t r = (color & 0x000000ff);
    const uint32_t argb = (a << 24) | (r << 16) | (g << 8) | b;

    for (unsigned int i = 0; i < fixed_info.smem_len / sizeof(uint32_t); ++i)
    {
        framebuffer[i] = argb;	//ARGB
    }

    munmap(framebuffer, fixed_info.smem_len);


    fb_var_screeninfo var_info;
    if (ioctl(fd, FBIOGET_VSCREENINFO, &var_info) < 0)
    {
        printf("FBIOGET_VSCREENINFO failed.\n");
    }

    const int width = var_info.xres;
    const int height = var_info.yres;
    const int buffers = var_info.yres_virtual / height;

    printf("width=%d, height=%d, buffers=%d\n", width, height, buffers);


    int currentBuffer = 0;
    int frames = 0;
    float totalTime = 0;

    Stopwatch sw;
    sw.Start();

    while (1)
    {
        // Swap buffers
        if (ioctl(fd, FBIOGET_VSCREENINFO, &var_info) < 0)
        {
            printf("FBIOGET_VSCREENINFO failed.\n");
            abort();
        }

        currentBuffer++;
        currentBuffer = currentBuffer % buffers;

        var_info.yoffset = currentBuffer * height;

#if 1
        if (ioctl(fd, FBIO_WAITFORVSYNC, 0) < 0)
        {
            printf("FBIO_WAITFORVSYNC failed.\n");
            abort();
        }

        if (ioctl(fd, FBIOPAN_DISPLAY, &var_info) < 0)
        {
            printf("FBIOPAN_DISPLAY failed.\n");
            abort();
        }
#else
        if (ioctl(fd, FBIOPUT_VSCREENINFO, &var_info) < 0) 
        {
            printf("FBIOPUT_VSCREENINFO failed.\n");
            abort();
        }
#endif



        // Measure FPS
		++frames;
		totalTime += (float)sw.Elapsed(); //GetTime();

		if (totalTime >= 1.0f)
		{
			int fps = (int)(frames / totalTime);
			fprintf(stderr, "FPS: %i\n", fps);

			frames = 0;
			totalTime = 0;
		}

		sw.Reset();
    }

    return 0;
}