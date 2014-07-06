#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include "font.h"

void MakeFontMap(GLint row, GLint col);
void DrawChar(GLshort, GLshort, char);
void RenderString(GLshort, GLshort, char *);

static const int FONT_BLOCK_COL = 16, FONT_BLOCK_ROW = 16;
static GLfloat tPoints[257][2]; // font map
static GLfloat tIncX = 0.0f;    // used for texture coords, x axis amount to move
static GLfloat tIncY = 0.0f;    // used for texture coords, y axis amount to move
static int     region = 0, font_size = 14;
static int     region_dims[] = { 0, 0, 0, 0 };
static GLfloat font_color[] = { 1.0, 0.0, 0.0 }; // default is white

void LoadFontMap()
{
    MakeFontMap(FONT_BLOCK_ROW, FONT_BLOCK_COL);
}

void MakeFontMap(GLint row, GLint col)
{
  GLint i = 0;
  GLfloat x, y;

  tIncX = (GLfloat)pow((float)col, -1);
  tIncY = (GLfloat)pow((float)row, -1);
	
  for (y = 0.0; y <= 1.0 - tIncY; y += tIncY){
    for (x = 0.0; x <= 1 - tIncX; x += tIncX, i++){
      tPoints[i][0] = x;
      tPoints[i][1] = y + tIncY;
    }
  }
}

void DrawChar(GLshort x, GLshort y, char c){
  GLint ci = c;
  
  short x_maxc = x + (font_size);
  short y_maxc = y + (font_size);

  float x_maxt = tPoints[ci][0] + tIncX;
  float y_maxt = tPoints[ci][1] - tIncY;

	const GLshort KVertices []=
	{
		x, y,
		x, y_maxc,
		x_maxc, y,
		x_maxc, y_maxc
	};
	
	const GLfloat kTexCoords[] =
	{
		tPoints[ci][0], tPoints[ci][1],
		tPoints[ci][0], y_maxt,		
		x_maxt, tPoints[ci][1],
		x_maxt, y_maxt,		
	};
			
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glVertexPointer (2, GL_SHORT, 0, KVertices);
	glEnableClientState (GL_VERTEX_ARRAY);

	glTexCoordPointer(2, GL_FLOAT, 0, kTexCoords);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);	
}

void RenderString(GLshort xpos, GLshort ypos, char *buffer){
  GLint px = region ? region_dims[0] : xpos;
  GLint py = region ? region_dims[1] - font_size : ypos;

  for (char *where = buffer; *where; where++){
    if (*where == '\t'){
      px += font_size * 5; // five spaces
    }
    else if (*where == '\n'){
      py -= font_size;
      px = region ? region_dims[0] : xpos;
    }
    else if (*where == '\\'){

    }
    else{
      DrawChar(px, py, *where);
      px += font_size;
    }
  }
}

void DrawFontString(GLshort xpos, GLshort ypos, char *msg, ...){
  va_list argptr;
  char buf[32768];	
  va_start(argptr, msg);
  vsprintf(buf, msg, argptr);
  va_end(argptr);

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);

  glDisable(GL_DEPTH_TEST);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   // use texture 
  glColor4f(font_color[0], font_color[1], font_color[2], 1.0);	
  RenderString(xpos, ypos, buf);

  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);

  glEnable(GL_DEPTH_TEST);
}

void SetFontSize(unsigned int s){
  font_size = s;
}

void SetFontColor(float r, float g, float b){
  font_color[0] = r;
  font_color[1] = g;
  font_color[2] = b;
}
