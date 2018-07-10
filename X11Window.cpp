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


X11Window::X11Window()
	: WindowBase()
{
	fd = open("/dev/dri/card0", O_RDWR);
	if (fd < 0)	abort();

	printf("fd=%d\n", fd);

	gbm = gbm_create_device(fd);
	if (!gbm) abort();
	printf("gbm = %p\n", gbm);


	gbm_surface = gbm_surface_create(gbm,
			1920, 1080,
			GBM_FORMAT_ARGB8888,
			GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
	if (!gbm_surface) abort();
	printf("gbm_surface = %p\n", surface);


	// Egl
	eglDisplay = Egl::Intialize((NativeDisplayType)gbm);

	EGLConfig eglConfig = Egl::FindConfig(eglDisplay, 8, 8, 8, 8, 24, 8);
	//EGLConfig eglConfig = Egl::FindConfig(eglDisplay, 8, 8, 8, 0, 24, 8);
	if (eglConfig == 0)
		throw Exception("Compatible EGL config not found.");



	EGLint windowAttr[] = {
		EGL_RENDER_BUFFER, 
		//EGL_BACK_BUFFER,
		EGL_NONE };

	surface = eglCreateWindowSurface(eglDisplay, eglConfig, (NativeWindowType)gbm_surface, NULL /*windowAttr*/);

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
