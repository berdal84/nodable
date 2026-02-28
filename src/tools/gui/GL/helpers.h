#pragma once

#ifdef NDBL_DESKTOP
    #include <GL/gl3w.h>
    #include <GL/glcorearb.h>
#elif NDBL_WEB
    #include <emscripten.h>
    #include <SDL_opengl.h>
    #include <SDL_opengl_glext.h>
#endif