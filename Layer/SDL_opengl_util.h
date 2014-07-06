#ifndef SDL_OPENGL_UTIL_H
#define SDL_OPENGL_UTIL_H

#include <SDL_image.h>
#include <SDL_opengl.h>
#include "Logger.h"

static bool SDL_Load_GL_Image(const char *path, GLuint &textureId) 
{
  GLint  numColors;
  GLenum texture_format;
  SDL_Surface *image = IMG_Load (path);
  if (image) 
    {
        glEnable(GL_TEXTURE_2D);
        glGenTextures( 1, &textureId );
 
        glBindTexture( GL_TEXTURE_2D, textureId );
 
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
 
        numColors = image->format->BytesPerPixel;
        if (numColors == 4)     // contains an alpha channel
          {
            if (image->format->Rmask == 0x000000ff)
              texture_format = GL_RGBA;
            else
              texture_format = GL_BGRA;
          } 
        else if (numColors == 3)     // no alpha channel
          {
            if (image->format->Rmask == 0x000000ff)
              texture_format = GL_RGB;
            else
              texture_format = GL_BGR;
          }
        else 
          {
            goto error;
          }

        Logger::InstanceOf().Info("SDL loaded GL image %s", path);
        glTexImage2D( GL_TEXTURE_2D, 0, numColors, image->w, image->h, 0,
                      texture_format, GL_UNSIGNED_BYTE, image->pixels );

        SDL_FreeSurface(image);

        return true;
    }

 error:
  Logger::InstanceOf().Error("SDL failed to GL image %s", path);
  if(image) 
    {
      SDL_FreeSurface(image);
      glDeleteTextures(1, &textureId);
    }

  return false;

}




#endif
