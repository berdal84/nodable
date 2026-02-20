#pragma once

#if PLATFORM_DESKTOP
    #include <GL/gl3w.h>
    #include <GL/glcorearb.h>
#elif PLATFORM_WEB
    #include <emscripten.h>
    #include <SDL_opengl.h>
    #include <SDL_opengl_glext.h>
#endif