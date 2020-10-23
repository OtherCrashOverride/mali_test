/*
*
* Copyright (C) 2016 OtherCrashOverride@users.noreply.github.com.
* All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2, as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
* more details.
*
*/

#include "X11Window.h"

#include "Egl.h"
#include "GL.h"

#include <linux/fb.h>


typedef struct fbdev_window
{
	unsigned short width;
	unsigned short height;
} fbdev_window;

static fbdev_window fbwin;;

X11Window::X11Window()
	: WindowBase()
{
	// Open the framebuffer to determine its properties
	int fd_fb0 = open("/dev/fb0", O_RDWR);
	printf("file handle: %x\n", fd_fb0);

	fb_var_screeninfo info;
	int ret = ioctl(fd_fb0, FBIOGET_VSCREENINFO, &info);
	if (ret < 0)
	{
		printf("FBIOGET_VSCREENINFO failed.\n");
		exit(1);
	}

	close(fd_fb0);

	int width = info.xres;
	int height = info.yres;
	int bpp = info.bits_per_pixel;
	int dataLen = width * height * (bpp / 8);

	printf("screen info: width=%d, height=%d, bpp=%d\n", width, height, bpp);


	// Set the EGL window size
	fbwin.width = width;
	fbwin.height = height;

	// Egl
	eglDisplay = Egl::Intialize((NativeDisplayType)EGL_DEFAULT_DISPLAY);

	EGLConfig eglConfig = Egl::FindConfig(eglDisplay, 8, 8, 8, 8, 0, 0);
	if (eglConfig == 0)
		throw Exception("Compatible EGL config not found.");


	EGLint windowAttr[] = {
		EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
		EGL_NONE };
	
	surface = eglCreateWindowSurface(eglDisplay, eglConfig, (NativeWindowType)nullptr, windowAttr);
	if (surface == EGL_NO_SURFACE)
	{
		Egl::CheckError();
	}


	// Create a context
	eglBindAPI(EGL_OPENGL_ES_API);

	EGLint contextAttributes[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE };

	context = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttributes);
	if (context == EGL_NO_CONTEXT)
	{
		Egl::CheckError();
	}

	EGLBoolean success = eglMakeCurrent(eglDisplay, surface, surface, context);
	if (success != EGL_TRUE)
	{
		Egl::CheckError();
	}
}

X11Window::~X11Window()
{
}


void X11Window::WaitForMessage()
{
}

bool X11Window::ProcessMessages()
{
	bool run = true;

	return run;
}

void X11Window::SwapBuffers()
{
	eglSwapBuffers(EglDisplay(), Surface());
	Egl::CheckError();
}

void X11Window::HideMouse()
{
}

void X11Window::UnHideMouse()
{
}

void X11Window::SetFullscreen(bool value)
{
}