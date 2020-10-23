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



int main()
{
	Stopwatch sw;
	std::shared_ptr<X11Window> window = std::make_shared<X11Window>();
	window->SetFullscreen(true);

	eglSwapInterval(window->EglDisplay(), 1);

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
