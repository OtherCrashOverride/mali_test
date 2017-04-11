#include "X11Window.h"
#include "Stopwatch.h"
#include "GL.h"
//extern "C"
//{
//#include <GLES2/gl2.h>
//#include <GLES3/gl3.h>
//}

//#define GL_GLEXT_PROTOTYPES
//#include <GLES2/gl2.h>

#include <cstdlib>

#include <xf86drm.h>


int main()
{
	int fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
	if (fd < 0)
	{
		throw Exception("DRM device open failed.");
	}


	Stopwatch sw;
	std::shared_ptr<X11Window> window = std::make_shared<X11Window>();
	window->SetFullscreen(true);


	sw.Start();


	bool isRunning = true;
	int frames = 0;
	float totalTime = 0;

	while (isRunning)
	{
		isRunning = window->ProcessMessages();


		// Render
		float red = rand() % 256 / 255.0f;
		float green = rand() % 256 / 255.0f;
		float blue = rand() % 256 / 255.0f;

		glClearColor(red, green, blue, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT |
			GL_DEPTH_BUFFER_BIT |
			GL_STENCIL_BUFFER_BIT);


		// Wait for VSYNC
		glFinish();

		drmVBlank vbl =
		{
			.request =
			{
				.type = DRM_VBLANK_RELATIVE,
				.sequence = 1,
			}
		};

		int io = drmWaitVBlank(fd, &vbl);
		if (io)
		{
			throw Exception("drmWaitVBlank failed.");
		}


		// Swap
		window->SwapBuffers();


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
