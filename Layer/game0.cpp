#include <Windows.h>
#include <tchar.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>

#include <stdio.h>
#include <math.h>
#include <time.h>

#include "font.h"
#include "SDL_opengl_util.h"
#include "Logger.h"

#define WIDTH 1024
#define HEIGHT 768
#define BPP 4
#define DEPTH 32

const int MAX_ACTORS = 4;

const int MAP_MAXX=4;
const int MAP_MAXY=4;

const int MAP_TILE_LENGTH=128; // play area is 512x512

SDL_Window *window;
SDL_GLContext ctx;

Uint32 gtime;

GLuint font_texture;
GLuint sprite_map;

char MAP[MAP_MAXX][MAP_MAXY] = {
    { 'x',  'x',  'x',  'x', },
    { 'x',  'x',  'x',  'x', },
    { 'x',  'x',  'x',  'x', },
    { 'x',  'w',  'x',  'x', }
};

class Actor {
    public:
        Actor() { 
            pos[2] = 0.1;
            radius = 25;
        }
        float pos[3];
        float vel[3];	
        float radius;
        bool active = false;
        Uint32 time_initialized;
        Uint32 last_updated;

        void Init(Uint32 now) {
            time_initialized = now;
            last_updated = now;
        }

        void Update(Uint32 now) {
            Uint32 delta = now - last_updated;

            float percms = ((float)delta) / 1000.0f;

            pos[0] += vel[0] * percms;
            pos[1] += vel[1] * percms;

            last_updated = now;
        }

    private:

};

Actor actors[MAX_ACTORS];
Actor &theplayer = actors[0];

void InitActors() {
    for(int j=0; j < MAX_ACTORS; j++) {
        actors[j].pos[0] = 100 + (j * 75);
        actors[j].pos[1] = 100.0;
    }
}

void Display_InitGL()
{
    glShadeModel( GL_SMOOTH );
}

/* function to reset our viewport after a window resize */
int Display_SetViewport( int width, int height )
{
    return 1;
}

void DrawCircle(float radius, float x, float y, float z) 
{

    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glPointSize(radius*2);

    glBegin(GL_POINTS);
    glVertex3f(x, y, z);
    glEnd();
}

void DrawMap() 
{
    int cury = 0;
    for(int i=MAP_MAXY-1; i >= 0; i--) 
    {
        for(int j=0; j < MAP_MAXX; j++) 
        {
            char typ = MAP[i][j];
            if(typ == 'x') {
                glColor3f(0.5, 0.5, 0.5);
            }
            else if(typ == 'w') {
                glColor3f(0.0, 0.0, 1.0);
            }
            glBegin(GL_QUADS);
            glVertex3f(j * MAP_TILE_LENGTH, cury, 0.0);
            glVertex3f((j * MAP_TILE_LENGTH)+MAP_TILE_LENGTH, cury, 0.0);
            glVertex3f((j * MAP_TILE_LENGTH)+MAP_TILE_LENGTH, cury+MAP_TILE_LENGTH, 0.0);
            glVertex3f((j * MAP_TILE_LENGTH), cury+MAP_TILE_LENGTH, 0.0);
            glEnd();
        }
        cury += MAP_TILE_LENGTH;
    }
}

void Display_Render()
{
    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glViewport(0, 0, WIDTH, HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, WIDTH, 0, HEIGHT, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();       

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, font_texture);

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    DrawMap();
    for(int j=0; j < MAX_ACTORS; j++) {
        Actor *actor = &actors[j];
        if(actor->active) 
        {
            glColor3f(1.0, 1.0, 0.0);
            DrawCircle(actor->radius, actor->pos[0], actor->pos[1], actor->pos[2]);
        }
    }
    SetFontSize(HEIGHT / 32);
    SetFontColor(1.0, 1.0, 1.0);
    DrawFontString(10, 10, "abc");
    SDL_GL_SwapWindow(window);
}

void TransformMouse(int *x, int *y) {
    *y = HEIGHT - *y;
}

bool collided(const int x, const int y, const Actor &actor) {
    return pow(x-actor.pos[0], 2.0f) + pow(y-actor.pos[1], 2.0f) < pow(actor.radius, 2.0f); 
}


int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Mongoose", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    ctx = SDL_GL_CreateContext(window);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetSwapInterval(1);

    srand((unsigned)time(NULL));
    Display_InitGL();
    Display_SetViewport(WIDTH, HEIGHT);

    Logger::InstanceOf().SetPrintToStdout(true);
    LoadFontMap();
    const char *fimage = "images/font-8bit.png";
    SDL_Load_GL_Image(fimage, font_texture);

    InitActors();
    bool quit = false;
    SDL_Event e;
    int mx, my;

    gtime = SDL_GetTicks();
    for(int j=0; j < MAX_ACTORS; j++) 
    {
        actors[j].Init(gtime);
        theplayer.active=true;
    }

    while(!quit) 
    {
        gtime = SDL_GetTicks();

        for(int j=0; j < MAX_ACTORS; j++) {
            if(actors[j].active) {
                actors[j].Update(gtime);
            }
        }

        while(SDL_PollEvent(&e)) 
        {      
            switch (e.type) 
            {
                case SDL_KEYDOWN:
                    if(e.key.keysym.sym == SDLK_q) 
                    {
                        quit = true;
                    } 
                    else if(e.key.keysym.sym == SDLK_LEFT) 
                    {
                        theplayer.pos[0] -= 25;
                    } 
                    else if(e.key.keysym.sym == SDLK_RIGHT) 
                    {
                        theplayer.pos[0] += 25;
                    }
                    else if(e.key.keysym.sym == SDLK_UP) 
                    {
                        theplayer.pos[1] += 25;
                    }
                    else if(e.key.keysym.sym == SDLK_DOWN) 
                    {
                        theplayer.pos[1] -= 25;
                    }

                    break;
                case SDL_KEYUP:
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    Uint32 mstate = SDL_GetMouseState(&mx, &my);
                    TransformMouse(&mx, &my);
                    Logger::InstanceOf().Info("x: %d y: %d", mx, my);
                    for(int i=0; i < MAX_ACTORS; i++) 
                    {
                        bool acollided = collided(mx, my, actors[i]);
                        if(acollided) 
                        {
                            Logger::InstanceOf().Info("mouse clicked on actor: %d", i);
                        }
                    }
                    break;
            }
        }
        Display_Render();
    }

    SDL_Quit();

    return 0;
}





