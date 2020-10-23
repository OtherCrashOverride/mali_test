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


    // Map the screen
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
        // Draw
        int nextBuffer = (currentBuffer + 1) % buffers;

       
        const uint8_t r = rand() % 255;
        const uint8_t g = rand() % 255;
        const uint8_t b = rand() % 255;
        const uint8_t a = 0xff;
        const uint32_t argb = (a << 24) | (r << 16) | (g << 8) | b;

        int len = (width * height);
        unsigned int* dst = framebuffer + (len * nextBuffer);
        for (unsigned int i = 0; i < len; ++i)
        {
            dst[i] = argb;	//ARGB
        }


        // Swap buffers
        if (ioctl(fd, FBIOGET_VSCREENINFO, &var_info) < 0)
        {
            printf("FBIOGET_VSCREENINFO failed.\n");
            abort();
        }

        currentBuffer = nextBuffer;
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

        //sleep(1);

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