#include <Windows.h>
#include <tchar.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <vector>

#include "font.h"
#include "SDL_opengl_util.h"
#include "Logger.h"
#include "vector2d.h"

#define WIDTH 768
#define HEIGHT 768
#define BPP 4
#define DEPTH 32

#define PASSABLE_STATE 0x1

typedef struct maploc_s {
    int x;
    int y;
} maploc_t;

const int MAX_ACTORS = 4;

const int MAP_MAXX=12;
const int MAP_MAXY=12;

const int MAP_TILE_LENGTH=64; // play area is 768x768

SDL_Window *window;
SDL_GLContext ctx;

Uint32 gtime;

GLuint font_texture;
GLuint dungeon_sprite_map;

#define DUNGEON_TILE_LEN 32.0f
#define DUNGEON_TILE_WIDTH 2048.0f
#define DUNGEON_TILE_HEIGHT 1536.0f

// w - water
// f - floor
// l - wall
// x - exit
// e - entry
// d - dirt
// r - door
// s - skeletons

#define FL 1 
#define WL 1 << 1 
#define EN 1 << 2 
#define EX 1 << 3 
#define WT 1 << 4 
#define DR 1 << 5 
#define DT 1 << 6
#define SK 1 << 7

#define HAS_TREASURE 1 << 8 

int MAP[MAP_MAXX][MAP_MAXY] = {
    {FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,EX},
    {WL,WL,FL,WL,FL,WL,FL,WL,FL,WL,FL,WL},
    {WL,WL,FL,WL,FL,WL,FL,WL,FL,WL,FL,WL},
    {WL,WL,FL | HAS_TREASURE,WL,FL,WL,FL,WL,FL,WL,FL,WL},
    {WL,WL,WL,WL,SK,WL,DT,WL,DR,WL,WT,WL},
    {FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL},
    {WT,WL,DR,WL,DT,WL,SK,WL,WL,WL,WL,WL},
    {FL,WL,FL,WL,FL,WL,FL,WL,FL,WL,FL,WL},
    {FL,WL,FL,WL,FL,WL,FL,WL,FL,WL,FL,WL},
    {FL,WL,FL,WL,FL,WL,FL,WL,FL,WL,FL,WL},
    {FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL},
    {EN,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL}
};

char MAPSTATE[MAP_MAXX][MAP_MAXY] = { 0 };

class Actor {
    public:
        Actor() { 
            pos[2] = 0.1;
            radius = 16;
        }
        float pos[3];
        float vel[3];	
        float radius;
        bool active = false;
        Uint32 time_initialized;
        Uint32 last_updated;
        maploc_t maploc;
        char  *job_type;

        void Init(Uint32 now) {
            time_initialized = now;
            last_updated = now;
            active = true;
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
Actor *theplayer = &actors[0];

void FindEntryPoint(maploc_t &loc)
{
    loc.x = -1;
    loc.y = -1;
    for(int i=0; i < MAP_MAXY; i++) 
    {
        for(int j=0; j < MAP_MAXX; j++) 
        {
            if(MAP[i][j] & EN) {
                loc.y = i;
                loc.x = j;
                return;
            }                
        }
    }
}

void InitEntities() {
    for(int i=0; i < MAP_MAXY; i++) 
    {
        for(int j=0; j < MAP_MAXX; j++) 
        {
            char typ = MAP[i][j];
            if(typ & FL || typ & EN) {
                MAPSTATE[i][j] |= PASSABLE_STATE;
            }
        }
    }

    maploc_t loc;
    FindEntryPoint(loc);
    for(int j=0; j < MAX_ACTORS; j++) {
        actors[j].maploc.x = loc.x;
        actors[j].maploc.y = loc.y;
    }
    actors[0].job_type = "builder";
    actors[1].job_type = "warrior";
    actors[2].job_type = "keymaster";
    actors[3].job_type = "digger";
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
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glPointSize(radius*2);

    glBegin(GL_POINTS);
    glVertex3f(x, y, z);
    glEnd();
}

float get_sprite_coord(float i, float len, float max) {
    return (i * len) / max;
}

void DrawMap() 
{
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, dungeon_sprite_map);
    glColor3f(1.0, 1.0, 1.0);

    for(int i=MAP_MAXY-1; i >= 0; i--) 
    {
        int cury = i * MAP_TILE_LENGTH;
        for(int j=0; j < MAP_MAXX; j++) 
        {
            int typ = MAP[i][j];
            int xoffset = 0; 
            int yoffset = 0;
            if(typ & FL) {
                yoffset = 13; xoffset = 22;
            } else if(typ & WT) {
                yoffset = 19; xoffset = 19;
            } else if(typ & DT) {
                yoffset = 14; xoffset = 25;
            } else if(typ & DR) {
                yoffset = 11; xoffset = 24;
            } else if(typ & SK) {
                yoffset = 3; xoffset = 2;
            } else if(typ & EX) {
                yoffset = 45; xoffset = 4;
            } else if(typ & EN) {
                yoffset = 45; xoffset = 5;
            } else if(typ & WL) {
                yoffset = 17; xoffset= 39;
            }
            int curx = j * MAP_TILE_LENGTH;

	    float minx = get_sprite_coord(xoffset, DUNGEON_TILE_LEN, DUNGEON_TILE_WIDTH) + (1.0f / DUNGEON_TILE_WIDTH);
            float maxx = get_sprite_coord(xoffset+1, DUNGEON_TILE_LEN, DUNGEON_TILE_WIDTH) - (1.0f / DUNGEON_TILE_WIDTH);

            float miny = get_sprite_coord(yoffset, DUNGEON_TILE_LEN, DUNGEON_TILE_HEIGHT) + (1.0f / DUNGEON_TILE_HEIGHT);
	    float maxy = get_sprite_coord(yoffset + 1, DUNGEON_TILE_LEN, DUNGEON_TILE_HEIGHT) - (1.0f / DUNGEON_TILE_HEIGHT);


            glBegin(GL_QUADS);

            glTexCoord2f(minx, maxy);
            glVertex3f(curx, cury, 0.0);

            glTexCoord2f(maxx, maxy);
            glVertex3f(curx+MAP_TILE_LENGTH, cury, 0.0);

            glTexCoord2f(maxx, miny);
            glVertex3f(curx+MAP_TILE_LENGTH, cury+MAP_TILE_LENGTH, 0.0);

            glTexCoord2f(minx, miny);
            glVertex3f(curx, cury+MAP_TILE_LENGTH, 0.0);

            glEnd();

            if(typ & HAS_TREASURE) {
              glEnable( GL_BLEND );
              glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );  

              float minx = get_sprite_coord(0, DUNGEON_TILE_LEN, DUNGEON_TILE_WIDTH) + (1.0f / DUNGEON_TILE_WIDTH);
              float maxx = get_sprite_coord(1, DUNGEON_TILE_LEN, DUNGEON_TILE_WIDTH) - (1.0f / DUNGEON_TILE_WIDTH);

              float miny = get_sprite_coord(12, DUNGEON_TILE_LEN, DUNGEON_TILE_HEIGHT) + (1.0f / DUNGEON_TILE_HEIGHT);
              float maxy = get_sprite_coord(13, DUNGEON_TILE_LEN, DUNGEON_TILE_HEIGHT) - (1.0f / DUNGEON_TILE_HEIGHT);

              glBegin(GL_QUADS);

              glTexCoord2f(minx, maxy);
              glVertex3f(curx, cury, 0.1);

              glTexCoord2f(maxx, maxy);
              glVertex3f(curx+MAP_TILE_LENGTH, cury, 0.1);

              glTexCoord2f(maxx, miny);
              glVertex3f(curx+MAP_TILE_LENGTH, cury+MAP_TILE_LENGTH, 0.1);

              glTexCoord2f(minx, miny);
              glVertex3f(curx, cury+MAP_TILE_LENGTH, 0.1);

              glEnd();

              glDisable( GL_BLEND );
            }
        }
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

    DrawMap();
    for(int j=0; j < MAX_ACTORS; j++) {
        Actor *actor = &actors[j];
        int xoffset = 0;
        int yoffset = 0;
        if(actor->active) 
        {
            glColor3f(1.0, 1.0, 0.0);

            if(strcmp(actor->job_type, "builder") == 0) { glColor3f(0.0, 0.0, 1.0); }
            if(strcmp(actor->job_type, "digger") == 0)  { xoffset = MAP_TILE_LENGTH * 0.5; glColor3f(1.0, 1.0, 0.8); }
            if(strcmp(actor->job_type, "keymaster") == 0) { yoffset = MAP_TILE_LENGTH * 0.5; glColor3f(0.75, 0.75, 0.37); }
            if(strcmp(actor->job_type, "warrior") == 0) { xoffset = MAP_TILE_LENGTH * 0.5; yoffset = MAP_TILE_LENGTH * 0.5; glColor3f(0.9, 0.9, 0.9); }

            float x = (actor->maploc.x * MAP_TILE_LENGTH) + actor->radius + xoffset;
            float y = (actor->maploc.y * MAP_TILE_LENGTH) + actor->radius + yoffset;

            DrawCircle(actor->radius, x, y, actor->pos[2]);
        }
    }


    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, font_texture);

    SetFontSize(HEIGHT / 32);
    SetFontColor(1.0, 1.0, 1.0);
    DrawFontString(10, HEIGHT-(HEIGHT/32), "abc");
    SDL_GL_SwapWindow(window);
}

void TransformMouse(int *x, int *y) {
    *y = HEIGHT - *y;
}

bool collided(const int x, const int y, const Actor &actor) {
    return pow(x-actor.pos[0], 2.0f) + pow(y-actor.pos[1], 2.0f) < pow(actor.radius, 2.0f); 
}

bool CanMove(Actor *actor, int xoffset, int yoffset) {
    if(actor->maploc.x + xoffset < 0) { return false; }
    if(actor->maploc.y + yoffset < 0) { return false; }

    if(actor->maploc.x + xoffset >= MAP_MAXX) { return false; } 
    if(actor->maploc.y + yoffset >= MAP_MAXY) { return false; }

    int xidx = actor->maploc.x + xoffset;
    int yidx = actor->maploc.y + yoffset;

    if(MAP[yidx][xidx] & WL) {
        return false;
    }

    if(MAPSTATE[yidx][xidx] & PASSABLE_STATE) {
       return true;   
    }

    return false;
}

void Work(Actor *actor, int xoffset, int yoffset) {
    int xidx = actor->maploc.x + xoffset;
    int yidx = actor->maploc.y + yoffset;

    if(strcmp(actor->job_type, "builder") == 0 && MAP[yidx][xidx] & WT) {
       MAPSTATE[yidx][xidx] |= PASSABLE_STATE; 
    }
    if(strcmp(actor->job_type, "keymaster") == 0 && MAP[yidx][xidx] & DR) {
       MAPSTATE[yidx][xidx] |= PASSABLE_STATE; 
    }
    if(strcmp(actor->job_type, "warrior") == 0 && MAP[yidx][xidx] & SK) {
       MAPSTATE[yidx][xidx] |= PASSABLE_STATE; 
    }
    if(strcmp(actor->job_type, "digger") == 0 && MAP[yidx][xidx] & DT ) {
       MAPSTATE[yidx][xidx] |= PASSABLE_STATE; 
    }
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
    SDL_Load_GL_Image("images/font-8bit.png", font_texture);
    SDL_Load_GL_Image("images/dungeon_tileset.png", dungeon_sprite_map);

    InitEntities();
    bool quit = false;
    SDL_Event e;
    int mx, my;

    gtime = SDL_GetTicks();
    for(int j=0; j < MAX_ACTORS; j++) 
    {
        actors[j].Init(gtime);
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
                        if(CanMove(theplayer, -1, 0)) 
                        {
                            theplayer->maploc.x -= 1;
                        } else {
                            Work(theplayer, -1, 0);
                        }
                    } 
                    else if(e.key.keysym.sym == SDLK_RIGHT) 
                    {
                        if(CanMove(theplayer, 1, 0)) 
                        {
                            theplayer->maploc.x += 1;
                        } else {
                            Work(theplayer, 1, 0);
                        }
                    }
                    else if(e.key.keysym.sym == SDLK_UP) 
                    {
                        if(CanMove(theplayer, 0, 1)) 
                        {
                            theplayer->maploc.y += 1;
                        } else {
                            Work(theplayer, 0, 1);
                        }
                    }
                    else if(e.key.keysym.sym == SDLK_DOWN) 
                    {
                        if(CanMove(theplayer, 0, -1)) 
                        {
                            theplayer->maploc.y -= 1;
                        } else {
                            Work(theplayer, 0, -1);
                        }
                    } 
                    else if(e.key.keysym.sym == SDLK_1) {
                        theplayer = &actors[0];
                    }
                    else if(e.key.keysym.sym == SDLK_2) {
                        theplayer = &actors[1];
                    }
                    else if(e.key.keysym.sym == SDLK_3) {
                        theplayer = &actors[2];
                    }
                    else if(e.key.keysym.sym == SDLK_4) {
                        theplayer = &actors[3];
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





