#ifndef __BASE_H__
#define __BASE_H__

#include <cstdio>

#ifdef _WIN32
#include <Windows.h>
//#pragma comment(lib, "glew32s.lib")  
#pragma comment(lib, "opengl32.lib")
#endif

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/wglew.h>

#include <bass.h>

#include <cassert>

extern int g_window_width;
extern int g_window_height;
extern bool g_app_run;

extern "C" bool fspec_init(void);

extern "C" void fspec_teardown(void);

extern "C" void fspec_update(float dt);

extern "C" void fspec_render(void);

#endif