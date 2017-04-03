#include "X11Window.h"
#include "Stopwatch.h"
#include "GL.h"
#include "Matrix4.h"

#include <cstdlib>

#include <libdrm/drm.h>
#include <xf86drm.h>
#include <drm/drm_fourcc.h>
#include <sys/mman.h>

#define EGL_EGLEXT_PROTOTYPES 1
#include <EGL/eglext.h>
#include <GLES2/gl2ext.h>



const float quad[] =
{
	-1,  1, 0,
	-1, -1, 0,
	1, -1, 0,

	1, -1, 0,
	1,  1, 0,
	-1,  1, 0
};

const float quadUV[] =
{
	0, 0,
	0, 1,
	1, 1,

	1, 1,
	1, 0,
	0, 0
};


const char* vertexSource = "\n \
attribute mediump vec4 Attr_Position;\n \
attribute mediump vec2 Attr_TexCoord0;\n \
\n \
uniform mat4 WorldViewProjection;\n \
\n \
varying mediump vec2 TexCoord0;\n \
\n \
void main()\n \
{\n \
\n \
  gl_Position = Attr_Position * WorldViewProjection;\n \
  TexCoord0 = Attr_TexCoord0;\n \
}\n \
\n \
 ";

const char* fragmentSource = "\n \
uniform lowp sampler2D DiffuseMap;\n \
\n \
varying mediump vec2 TexCoord0;\n \
\n \
void main()\n \
{\n \
  mediump vec4 rgba = texture2D(DiffuseMap, TexCoord0);\n \
\n \
  gl_FragColor = rgba;\n \
}\n \
\n \
";

int OpenDRM()
{
	int fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
	if (fd < 0)
	{
		throw Exception("DRM device open failed.");
	}

	uint64_t hasDumb;
	if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &hasDumb) < 0 ||
		!hasDumb)
	{
		throw Exception("DRM device does not support dumb buffers");
	}

	return fd;
}

int CreateBuffer(int fd)
{
	// Create dumb buffer
	drm_mode_create_dumb buffer = { 0 };
	buffer.width = 512;
	buffer.height = 512;
	buffer.bpp = 32;

	int ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &buffer);
	if (ret < 0)
	{
		throw Exception("DRM_IOCTL_MODE_CREATE_DUMB failed.");		
	}

	
	// Get the dmabuf for the buffer
	drm_prime_handle prime = { 0 };
	prime.handle = buffer.handle;
	prime.flags = DRM_CLOEXEC | DRM_RDWR;
	
	ret = drmIoctl(fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &prime);
	if (ret < 0)
	{
		throw Exception("DRM_IOCTL_PRIME_HANDLE_TO_FD failed.");
	}
	
	return prime.fd;
}

int main()
{
	int fd = OpenDRM();
	int dmafd = CreateBuffer(fd);

	
	// Create test pattern
	void* frame = mmap(NULL, 512 * 512 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, dmafd, 0);
	if (frame == MAP_FAILED)
	{
		throw Exception("mmap failed.");
	}

	unsigned int* ptr = (unsigned int*)frame;
	for (int y = 0; y < 512; ++y)
	{
		for (int x = 0; x < 512; ++x)
		{
			unsigned char r = 0;
			unsigned char g = 0;
			unsigned char b = 0;
			unsigned char l = x >> 1;

			if (y < 128)
			{
				r = 255 - l;
			}
			else if (y < 256)
			{
				g = l;
			}
			else if (y < 384)
			{
				b = 255 - l;
			}
			else
			{
				r = l;
				g = l;
				b = l;
			}

			ptr[y * 512 + x] = r << 24 | g << 16 | b << 8 | 0xff;
		}
	}

	//  -------

	Stopwatch sw;
	WindowSPTR window = std::make_shared<X11Window>();

	// ---------

	// EGL_EXT_image_dma_buf_import
	EGLint img_attrs[] = {
		EGL_WIDTH, 512,
		EGL_HEIGHT, 512,
		EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_RGBA8888,
		EGL_DMA_BUF_PLANE0_FD_EXT, dmafd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, 512 * 4,
		EGL_NONE
	};

	EGLImageKHR image = eglCreateImageKHR(window->EglDisplay(), EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, 0, img_attrs);
	Egl::CheckError();

	fprintf(stderr, "EGLImageKHR = %p\n", image);

	
	// Texture
	GLuint texture2D;
	glGenTextures(1, &texture2D);
	GL::CheckError();

	glActiveTexture(GL_TEXTURE0);
	GL::CheckError();

	glBindTexture(GL_TEXTURE_2D, texture2D);
	GL::CheckError();

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	GL::CheckError();

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GL::CheckError();

	glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
	GL::CheckError();


	// Shader
	GLuint vertexShader = 0;
	GLuint fragmentShader = 0;

	for (int i = 0; i < 2; ++i)
	{
		GLuint shaderType;
		const char* sourceCode;

		if (i == 0)
		{
			shaderType = GL_VERTEX_SHADER;
			sourceCode = vertexSource;
		}
		else
		{
			shaderType = GL_FRAGMENT_SHADER;
			sourceCode = fragmentSource;
		}

		GLuint openGLShaderID = glCreateShader(shaderType);
		GL::CheckError();

		const char* glSrcCode[1] = { sourceCode };
		const int lengths[1] = { -1 }; // Tell OpenGL the string is NULL terminated

		glShaderSource(openGLShaderID, 1, glSrcCode, lengths);
		GL::CheckError();

		glCompileShader(openGLShaderID);
		GL::CheckError();


		GLint param;

		glGetShaderiv(openGLShaderID, GL_COMPILE_STATUS, &param);
		GL::CheckError();

		if (param == GL_FALSE)
		{
			throw Exception("Shader Compilation Failed.");
		}

		if (i == 0)
		{
			vertexShader = openGLShaderID;
		}
		else
		{
			fragmentShader = openGLShaderID;
		}
	}


	// Program
	GLuint openGLProgramID = glCreateProgram();
	GL::CheckError();

	glAttachShader(openGLProgramID, vertexShader);
	GL::CheckError();

	glAttachShader(openGLProgramID, fragmentShader);
	GL::CheckError();


	// Bind
	glEnableVertexAttribArray(0);
	GL::CheckError();

	glBindAttribLocation(openGLProgramID, 0, "Attr_Position");
	GL::CheckError();

	glEnableVertexAttribArray(1);
	GL::CheckError();

	glBindAttribLocation(openGLProgramID, 1, "Attr_TexCoord0");
	GL::CheckError();

	glLinkProgram(openGLProgramID);
	GL::CheckError();

	glUseProgram(openGLProgramID);
	GL::CheckError();


	// Get program uniform(s)
	GLuint wvpUniformLocation = glGetUniformLocation(openGLProgramID, "WorldViewProjection");
	GL::CheckError();

	if (wvpUniformLocation < 0)
		throw Exception();


	// Setup OpenGL
	glClearColor(1, 0, 0, 1);	// RED for diagnostic use
	//glClearColor(0, 0, 0, 0);	// Transparent Black
	GL::CheckError();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	GL::CheckError();

	glEnable(GL_CULL_FACE);
	GL::CheckError();

	glCullFace(GL_BACK);
	GL::CheckError();

	glFrontFace(GL_CCW);
	GL::CheckError();

	// ---------
	
	bool isRunning = true;
	int frames = 0;
	float totalTime = 0;

	sw.Start();

	while (isRunning)
	{
		isRunning = window->ProcessMessages();


		// Render
		glClear(GL_COLOR_BUFFER_BIT |
				GL_DEPTH_BUFFER_BIT |
				GL_STENCIL_BUFFER_BIT);

		// Quad
		{
			// Set the quad vertex data
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * 4, quad);
			GL::CheckError();

			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * 4, quadUV);
			GL::CheckError();


			// Set the matrix
			Matrix4 transpose = Matrix4::CreateTranspose(Matrix4::Identity);
			float* wvpValues = &transpose.M11;

			glUniformMatrix4fv(wvpUniformLocation, 1, GL_FALSE, wvpValues);
			GL::CheckError();


			// Draw
			glDrawArrays(GL_TRIANGLES, 0, 3 * 2);
			GL::CheckError();
		}


		eglSwapBuffers(window->EglDisplay(), window->Surface());
		Egl::CheckError();


		// Measure FPS
		++frames;
		totalTime += (float)sw.Elapsed();

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
