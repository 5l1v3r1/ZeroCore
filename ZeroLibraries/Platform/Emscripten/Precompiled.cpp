///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "../Empty/Browser.cpp"
#include "../Empty/ComPort.cpp"
#include "../Empty/CrashHandler.cpp"
#include "../Empty/Debug.cpp"
#include "../Empty/DebugSymbolInformation.cpp"
#include "../Empty/DirectoryWatcher.cpp"
#include "../Empty/ExecutableResource.cpp"
#include "../Empty/VirtualFileAndFileSystem.cpp"
#include "../Empty/Intrinsics.cpp"
#include "../Empty/Registry.cpp"
#include "../Empty/Socket.cpp"
#include "../Empty/Thread.cpp"
#include "../Empty/ThreadSync.cpp"
#include "../STL/Atomic.cpp"
#include "../STL/FpControl.cpp"
#include "../STL/Process.cpp"
#include "../SDL/Audio.cpp"
#include "../SDL/ExternalLibrary.cpp"
#include "../SDL/Main.cpp"
//#include "../SDL/OpenglRendererSDL.cpp"
#include "../SDL/Peripherals.cpp"
#include "../SDL/PlatformStandard.cpp"
#include "../SDL/Resolution.cpp"
#include "../SDL/Shell.cpp"
#include "../SDL/Timer.cpp"
#include "../SDL/Utilities.cpp"
#include "../Empty/Renderer.cpp"
//#include "../OpenGL/OpenglRenderer.cpp"
//#include "../OpenGL/OpenglRenderer.hpp"

#include <emscripten.h>

int vsprintf_s(char* buffer, size_t numberOfElements, const char* format, va_list args)
{
  return vsnprintf(buffer, numberOfElements, format, args);
  //return vsprintf(buffer, format, argptr);
}

int sprintf_s(char* buffer, size_t numberOfElements, const char* format, ...)
{
  va_list args;
  va_start(args, format);
  int result = vsnprintf(buffer, numberOfElements, format, args);
  va_end(args);
  return result;
}

int swprintf_s(wchar_t* buffer, size_t numberOfElements, const wchar_t* format, ...)
{
  va_list args;
  va_start(args, format);
  int result = vswprintf(buffer, numberOfElements, format, args);
  va_end(args);
  return result;
}

errno_t strcat_s(char* dest, rsize_t destsz, const char* src)
{
  if (strlen(src) >= destsz)
  {
    if (dest && destsz != 0)
      *dest = '\0';
    return 1;
  }

  strcat(dest, src);
  return 0;
}

errno_t wcscat_s(wchar_t* dest, rsize_t destsz, const wchar_t* src)
{
  if (wcslen(src) >= destsz)
  {
    if (dest && destsz != 0)
      *dest = '\0';
    return 1;
  }

  wcscat(dest, src);
  return 0;
}

errno_t strcpy_s(char* dest, rsize_t destsz, const char* src)
{
  if (strlen(src) >= destsz)
  {
    if (dest && destsz != 0)
      *dest = '\0';
    return 1;
  }
    
  strcpy(dest, src);
  return 0;
}

errno_t wcscpy_s(wchar_t* dest, rsize_t destsz, const wchar_t* src)
{
  if (wcslen(src) >= destsz)
  {
    if (dest && destsz != 0)
      *dest = '\0';
    return 1;
  }

  wcscpy(dest, src);
  return 0;
}

errno_t strncpy_s(char *dest, rsize_t destsz, const char *src, rsize_t count)
{
  if (count > destsz)
  {
    if (dest && destsz != 0)
      *dest = '\0';
    return 1;
  }

  strncpy(dest, src, count);
  return 0;
}

void glDrawBuffer(GLenum buf)
{
  GLenum drawBuffers[8] = { GL_NONE };
  drawBuffers[0] = buf;
  glDrawBuffers(1, drawBuffers);
}

void glBlendEquationSeparatei(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
  Error("This function should not be called when running the Emscripten ZeroEditor, WebGL functioanlity was not properly queried");
};

void glBlendEquationi(GLuint buf, GLenum mode)
{
  Error("This function should not be called when running the Emscripten ZeroEditor, WebGL functioanlity was not properly queried");
};

void glBlendFuncSeparatei(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
  Error("This function should not be called when running the Emscripten ZeroEditor, WebGL functioanlity was not properly queried");
};

void glBlendFunci(GLuint buf, GLenum sfactor, GLenum dfactor)
{
  Error("This function should not be called when running the Emscripten ZeroEditor, WebGL functioanlity was not properly queried");
};

void glEnablei(GLenum cap, GLuint index)
{
  Error("This function should not be called when running the Emscripten ZeroEditor, WebGL functioanlity was not properly queried");
};

void glDisablei(GLenum cap, GLuint index)
{
  Error("This function should not be called when running the Emscripten ZeroEditor, WebGL functioanlity was not properly queried");
};

bool Zero::SetMainLoopFunction(int fps, Zero::MainLoopFn callback, void* userData, bool stopExecutionHere)
{
  // We can only have one main loop set, so cancel it here.
  static bool sIsMainLoopSet = false;
  if (sIsMainLoopSet)
    emscripten_cancel_main_loop();
  
  // This MUST be before because we could be stopping execution at 'emscripten_set_main_loop_arg'.
  sIsMainLoopSet = true;
  emscripten_set_main_loop_arg(callback, userData, 0, (int)stopExecutionHere);
  return true;
}
