#ifndef FONT_H
#define FONT_H

#ifdef IPHONE
#include <OpenGLES/ES1/glext.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif

#ifdef WIN32
#include <Windows.h>		
#include <gl/gl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Load the texture you want for the font. This font should be 8-bit
void LoadFontMap();

//   Set the current font size for the system
void SetFontSize(unsigned int);

// set the font color for the system
void SetFontColor(float, float, float);

// Draw a string (similiar to printf)
void DrawFontString(GLshort xpos, GLshort ypos, char *msg, ...);

#ifdef __cplusplus
}
#endif
		
	
#endif
