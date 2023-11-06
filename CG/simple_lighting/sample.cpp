#include <vector>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <glm/ext/scalar_constants.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef F_PI
#define F_PI ((float)(M_PI))
#define F_2_PI ((float)(2.f * F_PI))
#define F_PI_2 ((float)(F_PI / 2.f))
#endif

#ifdef WIN32
#include <windows.h>
#pragma warning(disable : 4996)
#endif

// clang-format off
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
// clang-format on

#include "horse.h"

//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Joe Graphics

// title of these windows:

const char* WINDOWTITLE = "OpenGL / GLUT Sample -- Joe Graphics";
const char* GLUITITLE = "User Interface Window";

// what the glui package defines as true and false:

const int GLUITRUE = true;
const int GLUIFALSE = false;

// the escape key:

const int ESCAPE = 0x1b;

// initial window size:

const int INIT_WINDOW_SIZE = 600;

// size of the 3d box to be drawn:

const float BOXSIZE = 2.f;

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:

const float MINSCALE = 0.05f;

// scroll wheel button values:

const int SCROLL_WHEEL_UP = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):

const int LEFT = 4;
const int MIDDLE = 2;
const int RIGHT = 1;

// which projection:

enum Projections { ORTHO, PERSP };

// which button:

enum ButtonVals { RESET, QUIT };

// window background color (rgba):

const GLfloat BACKCOLOR[] = {0., 0., 0., 1.};

// line width for the axes:

const GLfloat AXES_WIDTH = 3.;

// the color numbers:
// this order must match the radio button order, which must match the order of
// the color names, 	which must match the order of the color RGB values

enum Colors { RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA, WHITE_C };

enum LightType { POINT_LIGHT, SPOT_LIGHT };

char* ColorNames[] = {(char*)"Red", (char*)"Yellow", (char*)"Green", (char*)"Cyan", (char*)"Blue", (char*)"Magenta"};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[][3] = {
    {1., 0., 0.},  // red
    {1., 1., 0.},  // yellow
    {0., 1., 0.},  // green
    {0., 1., 1.},  // cyan
    {0., 0., 1.},  // blue
    {1., 0., 1.},  // magenta
};

// fog parameters:

const GLfloat FOGCOLOR[4] = {.0f, .0f, .0f, 1.f};
const GLenum FOGMODE = GL_LINEAR;
const GLfloat FOGDENSITY = 0.30f;
const GLfloat FOGSTART = 1.5f;
const GLfloat FOGEND = 4.f;

// for lighting:

const float WHITE[] = {1., 1., 1., 1.};

// for animation:

const int MS_PER_CYCLE = 10000;  // 10000 milliseconds = 10 seconds

// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong
// #define DEMO_Z_FIGHTING
// #define DEMO_DEPTH_BUFFER

// non-constant global variables:

int ActiveButton;   // current button that is down
GLuint AxesList;    // list to hold the axes
int AxesOn;         // != 0 means to draw the axes
GLuint HorseList;   // object display list
GLuint DinoList;    // object display list
GLuint DeerList;    // object display list
GLuint TeapotList;  // object display list
GLuint SphereList;
GLuint GridDL;        // grid
int DebugOn;          // != 0 means to print debugging info
int DepthCueOn;       // != 0 means to use intensity depth cueing
int DepthBufferOn;    // != 0 means to use the z-buffer
int DepthFightingOn;  // != 0 means to force the creation of z-fighting
int MainWindow;       // window id for main graphics window
int NowColor;         // index into Colors[ ]
int NowProjection;    // ORTHO or PERSP
float Scale;          // scaling factor
int ShadowsOn;        // != 0 means to turn shadows on
float Time;           // used for animation, this has a value between 0. and 1.
int Xmouse, Ymouse;   // mouse values
float Xrot, Yrot;     // rotation angles in degrees
int light_source_type;
int light_source_color;
bool Frozen;

// function prototypes:

void Animate();
void Display();
void DoViewMenu(int);
void DoAxesMenu(int);
void DoColorMenu(int);
void DoDepthBufferMenu(int);
void DoDepthFightingMenu(int);
void DoDepthMenu(int);
void DoDebugMenu(int);
void DoMainMenu(int);
void DoProjectMenu(int);
void DoRasterString(float, float, float, char*);
void DoStrokeString(float, float, float, float, char*);
float ElapsedSeconds();
void InitGraphics();
void InitLists();
void InitMenus();
void Keyboard(unsigned char, int, int);
void MouseButton(int, int, int, int);
void MouseMotion(int, int);
void Reset();
void Resize(int, int);
void Visibility(int);

void Axes(float);
void HsvRgb(float[3], float[3]);
void Cross(float[3], float[3], float[3]);
float Dot(float[3], float[3]);
float Unit(float[3], float[3]);
float Unit(float[3]);

// utility to create an array from 3 separate values:

float* Array3(float a, float b, float c) {
    static float array[4];

    array[0] = a;
    array[1] = b;
    array[2] = c;
    array[3] = 1.;
    return array;
}

// utility to create an array from a multiplier and an array:

float* MulArray3(float factor, float array0[]) {
    static float array[4];

    array[0] = factor * array0[0];
    array[1] = factor * array0[1];
    array[2] = factor * array0[2];
    array[3] = 1.;
    return array;
}

float* MulArray3(float factor, float a, float b, float c) {
    static float array[4];

    float* abc = Array3(a, b, c);
    array[0] = factor * abc[0];
    array[1] = factor * abc[1];
    array[2] = factor * abc[2];
    array[3] = 1.;
    return array;
}

void SetPointLight(int ilight, float x, float y, float z, float r, float g, float b) {
    glLightfv(ilight, GL_POSITION, Array3(x, y, z));
    glLightf(ilight, GL_SPOT_CUTOFF, 180.f);
    glLightfv(ilight, GL_AMBIENT, MulArray3(0.1f, 1.f, 1.f, 1.f));
    glLightfv(ilight, GL_DIFFUSE, MulArray3(0.6f, r, g, b));
    glLightfv(ilight, GL_SPECULAR, MulArray3(0.4f, 1.f, 1.f, 1.f));
    glLightf(ilight, GL_CONSTANT_ATTENUATION, 1.);
    glLightf(ilight, GL_LINEAR_ATTENUATION, 0.);
    glLightf(ilight, GL_QUADRATIC_ATTENUATION, 0.);
    glEnable(ilight);
}

void SetSpotLight(int ilight, float x, float y, float z, float xdir, float ydir, float zdir, float r, float g,
                  float b) {
    glLightfv(ilight, GL_POSITION, Array3(x, y, z));
    glLightfv(ilight, GL_SPOT_DIRECTION, Array3(xdir, ydir, zdir));
    glLightf(ilight, GL_SPOT_EXPONENT, 1.f);
    glLightf(ilight, GL_SPOT_CUTOFF, 45.f);
    glLightfv(ilight, GL_AMBIENT, Array3(0., 0., 0.));
    glLightfv(ilight, GL_DIFFUSE, Array3(r, g, b));
    glLightfv(ilight, GL_SPECULAR, Array3(r, g, b));
    glLightf(ilight, GL_CONSTANT_ATTENUATION, 1.);
    glLightf(ilight, GL_LINEAR_ATTENUATION, 0.);
    glLightf(ilight, GL_QUADRATIC_ATTENUATION, 0.);
    glEnable(ilight);
}

void SetMaterial(float r, float g, float b, float shininess) {
    glMaterialfv(GL_BACK, GL_EMISSION, Array3(0., 0., 0.));
    glMaterialfv(GL_BACK, GL_AMBIENT, MulArray3(.4f, (float*)WHITE));
    glMaterialfv(GL_BACK, GL_DIFFUSE, MulArray3(1., (float*)WHITE));
    glMaterialfv(GL_BACK, GL_SPECULAR, Array3(0., 0., 0.));
    glMaterialf(GL_BACK, GL_SHININESS, 2.f);

    glMaterialfv(GL_FRONT, GL_EMISSION, Array3(0., 0., 0.));
    glMaterialfv(GL_FRONT, GL_AMBIENT, Array3(r, g, b));
    glMaterialfv(GL_FRONT, GL_DIFFUSE, Array3(r, g, b));
    glMaterialfv(GL_FRONT, GL_SPECULAR, MulArray3(.8f, (float*)WHITE));
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

// these are here for when you need them -- just uncomment the ones you need:

// #include "setmaterial.cpp"
#include "osusphere.cpp"
// #include "osucone.cpp"
// #include "osutorus.cpp"
// #include "bmptotexture.cpp"
#include "loadobjfile.cpp"
// #include "keytime.cpp"
// #include "glslprogram.cpp"

// main program:

int main(int argc, char* argv[]) {
    // turn on the glut package:
    // (do this before checking argc and argv since glutInit might
    // pull some command line arguments out)

    glutInit(&argc, argv);

    // setup all the graphics stuff:

    InitGraphics();

    // create the display lists that **will not change**:

    InitLists();

    // init all the global variables used by Display( ):
    // this will also post a redisplay

    Reset();

    // setup all the user interface stuff:

    InitMenus();

    // draw the scene once and wait for some interaction:
    // (this will never return)

    glutSetWindow(MainWindow);
    glutMainLoop();

    // glutMainLoop( ) never actually returns
    // the following line is here to make the compiler happy:

    return 0;
}

// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it

void Animate() {
    // put animation stuff in here -- change some global variables for Display( )
    // to find:

    int ms = glutGet(GLUT_ELAPSED_TIME);
    ms %= MS_PER_CYCLE;                      // makes the value of ms between 0 and MS_PER_CYCLE-1
    Time = (float)ms / (float)MS_PER_CYCLE;  // makes the value of Time between 0.
                                             // and slightly less than 1.

    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DrawCircle(double radius, size_t steps = 15) {
    glBegin(GL_LINE_LOOP);
    constexpr auto pi = glm::pi<double>();
    const auto step = 2. * pi / steps;
    for (size_t i = 0; i <= steps; i++) {
        double angle = i * step;
        glVertex3d(radius * cos(angle), 0., radius * sin(angle));
    }
    glEnd();
}

// draw the complete scene:

void Display() {
    if (DebugOn != 0)
        fprintf(stderr, "Starting Display.\n");

    // set which window we want to do the graphics into:
    glutSetWindow(MainWindow);

    // erase the background:
    glDrawBuffer(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
#ifdef DEMO_DEPTH_BUFFER
    if (DepthBufferOn == 0)
        glDisable(GL_DEPTH_TEST);
#endif

    // specify shading to be flat:

    glShadeModel(GL_FLAT);

    // set the viewport to be a square centered in the window:

    GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
    GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
    GLsizei v = vx < vy ? vx : vy;  // minimum dimension
    GLint xl = (vx - v) / 2;
    GLint yb = (vy - v) / 2;
    glViewport(xl, yb, v, v);

    // set the viewing volume:
    // remember that the Z clipping  values are given as DISTANCES IN FRONT OF THE
    // EYE USE gluOrtho2D( ) IF YOU ARE DOING 2D !

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (NowProjection == ORTHO)
        glOrtho(-2.f, 2.f, -2.f, 2.f, 0.1f, 1000.f);
    else
        gluPerspective(70.f, 1.f, 0.1f, 1000.f);

    // place the objects into the scene:

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // set the eye position, look-at position, and up-vector:
    gluLookAt(7.f, 5.f, 7.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);

    // rotate the scene:

    glRotatef((GLfloat)Yrot, 0.f, 1.f, 0.f);
    glRotatef((GLfloat)Xrot, 1.f, 0.f, 0.f);

    // uniformly scale the scene:

    if (Scale < MINSCALE)
        Scale = MINSCALE;
    glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);

    // set the fog parameters:

    if (DepthCueOn != 0) {
        glFogi(GL_FOG_MODE, FOGMODE);
        glFogfv(GL_FOG_COLOR, FOGCOLOR);
        glFogf(GL_FOG_DENSITY, FOGDENSITY);
        glFogf(GL_FOG_START, FOGSTART);
        glFogf(GL_FOG_END, FOGEND);
        glEnable(GL_FOG);
    } else {
        glDisable(GL_FOG);
    }

    // possibly draw the axes:

    if (AxesOn != 0) {
        glColor3fv(&Colors[NowColor][0]);
        glCallList(AxesList);
    }

    // since we are using glScalef( ), be sure the normals get unitized:

    glEnable(GL_NORMALIZE);

    constexpr double carousel_radius = 3.;
    constexpr double carousel_freq = 2.;
    constexpr double carousel_y = 5.;
    constexpr double pi = glm::pi<double>();

    glm::vec3 light_color;
    switch (light_source_color) {
        case YELLOW:
            light_color = glm::vec3(1, 1, 0);
            break;
        case WHITE_C:
            light_color = glm::vec3(1, 1, 1);
            break;
        case RED:
            light_color = glm::vec3(1, 0, 0);
            break;
        case GREEN:
            light_color = glm::vec3(0, 1, 0);
            break;
        case BLUE:
            light_color = glm::vec3(0, 0, 1);
            break;
    }

    glPushMatrix();
    glColor3f(light_color.r, light_color.g, light_color.b);
    glTranslated(carousel_radius * sin(Time * 2 * pi * carousel_freq), carousel_y,
                 carousel_radius * cos(Time * 2 * pi * carousel_freq));
    // glScaled(0.5, 0.5, 0.5);
    glCallList(SphereList);
    glPopMatrix();

    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, MulArray3(.3f, (float*)WHITE));

    glEnable(GL_LIGHT0);
    // glDisable(GL_LIGHT0);

    if (light_source_type == POINT_LIGHT) {
        SetPointLight(GL_LIGHT0, carousel_radius * sin(Time * 2 * pi * 2.), 2,
                      carousel_radius * cos(Time * 2 * pi * 2.), light_color.r, light_color.g, light_color.b);
    } else {
        SetSpotLight(GL_LIGHT0, carousel_radius * sin(Time * 2 * pi * carousel_freq), carousel_y,
                     carousel_radius * cos(Time * 2 * pi * carousel_freq), 0, -1, 0, light_color.r, light_color.g,
                     light_color.b);
    }

    glColor3f(.5, .5, .5);
    glCallList(GridDL);

    // draw the box object by calling up its display list:
    glColor3f(1., 1., 0.);
    SetMaterial(1., 1., 0., 0.);
    glPushMatrix();
    glTranslated(-2., 1., 0);
    glScaled(0.5, 0.5, 0.5);
    glCallList(DeerList);
    glPopMatrix();

    glColor3f(1., 0., 0.);
    SetMaterial(1., 0., 0., 128.);
    glPushMatrix();
    glTranslated(2., 1.2, -1.5);
    glRotated(30, 0, 1, 0);
    glScaled(0.35, .35, .35);
    glCallList(TeapotList);
    glPopMatrix();

    glColor3f(0., 0., 1.);
    SetMaterial(0., 0., 1., 10);
    glPushMatrix();
    glTranslated(2, 1., 2);
    glRotated(-45, 0, 1, 0);
    glScaled(0.2, 0.2, 0.2);
    glCallList(DinoList);
    glPopMatrix();

    glDisable(GL_LIGHTING);

#ifdef DEMO_Z_FIGHTING
    if (DepthFightingOn != 0) {
        glPushMatrix();
        glRotatef(90.f, 0.f, 1.f, 0.f);
        glCallList(BoxList);
        glPopMatrix();
    }
#endif

    // draw some gratuitous text that just rotates on top of the scene:
    // i commented out the actual text-drawing calls -- put them back in if you
    // have a use for them a good use for thefirst one might be to have your name
    // on the screen a good use for the second one might be to have vertex numbers
    // on the screen alongside each vertex

    glDisable(GL_DEPTH_TEST);
    glColor3f(0.f, 1.f, 1.f);
    // DoRasterString( 0.f, 1.f, 0.f, (char *)"Text That Moves" );

    // draw some gratuitous text that is fixed on the screen:
    //
    // the projection matrix is reset to define a scene whose
    // world coordinate system goes from 0-100 in each axis
    //
    // this is called "percent units", and is just a convenience
    //
    // the modelview matrix is reset to identity as we don't
    // want to transform these coordinates

    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.f, 100.f, 0.f, 100.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(1.f, 1.f, 1.f);
    // DoRasterString( 5.f, 5.f, 0.f, (char *)"Text That Doesn't" );

    // swap the double-buffered framebuffers:

    glutSwapBuffers();

    // be sure the graphics buffer has been sent:
    // note: be sure to use glFlush( ) here, not glFinish( ) !

    glFlush();
}

void DoAxesMenu(int id) {
    AxesOn = id;

    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoColorMenu(int id) {
    NowColor = id - RED;

    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoDebugMenu(int id) {
    DebugOn = id;

    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoDepthBufferMenu(int id) {
    DepthBufferOn = id;

    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoDepthFightingMenu(int id) {
    DepthFightingOn = id;

    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoDepthMenu(int id) {
    DepthCueOn = id;

    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

// main menu callback:

void DoMainMenu(int id) {
    switch (id) {
        case RESET:
            Reset();
            break;

        case QUIT:
            // gracefully close out the graphics:
            // gracefully close the graphics window:
            // gracefully exit the program:
            glutSetWindow(MainWindow);
            glFinish();
            glutDestroyWindow(MainWindow);
            exit(0);
            break;

        default:
            fprintf(stderr, "Don't know what to do with Main Menu ID %d\n", id);
    }

    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void UpdateCurrentProjection(int proj) { NowProjection = proj; }

void DoProjectMenu(int id) {
    UpdateCurrentProjection(id);

    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

// use glut to display a string of characters using a raster font:

void DoRasterString(float x, float y, float z, char* s) {
    glRasterPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);

    char c;  // one character to print
    for (; (c = *s) != '\0'; s++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
    }
}

// use glut to display a string of characters using a stroke font:

void DoStrokeString(float x, float y, float z, float ht, char* s) {
    glPushMatrix();
    glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
    float sf = ht / (119.05f + 33.33f);
    glScalef((GLfloat)sf, (GLfloat)sf, (GLfloat)sf);
    char c;  // one character to print
    for (; (c = *s) != '\0'; s++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
    }
    glPopMatrix();
}

// return the number of seconds since the start of the program:

float ElapsedSeconds() {
    // get # of milliseconds since the start of the program:

    int ms = glutGet(GLUT_ELAPSED_TIME);

    // convert it to seconds:

    return (float)ms / 1000.f;
}

// initialize the glui window:

void InitMenus() {
    if (DebugOn != 0)
        fprintf(stderr, "Starting InitMenus.\n");

    glutSetWindow(MainWindow);

    int numColors = sizeof(Colors) / (3 * sizeof(float));
    int colormenu = glutCreateMenu(DoColorMenu);
    for (int i = 0; i < numColors; i++) {
        glutAddMenuEntry(ColorNames[i], i);
    }

    int axesmenu = glutCreateMenu(DoAxesMenu);
    glutAddMenuEntry("Off", 0);
    glutAddMenuEntry("On", 1);

    int depthcuemenu = glutCreateMenu(DoDepthMenu);
    glutAddMenuEntry("Off", 0);
    glutAddMenuEntry("On", 1);

    int depthbuffermenu = glutCreateMenu(DoDepthBufferMenu);
    glutAddMenuEntry("Off", 0);
    glutAddMenuEntry("On", 1);

    int depthfightingmenu = glutCreateMenu(DoDepthFightingMenu);
    glutAddMenuEntry("Off", 0);
    glutAddMenuEntry("On", 1);

    int debugmenu = glutCreateMenu(DoDebugMenu);
    glutAddMenuEntry("Off", 0);
    glutAddMenuEntry("On", 1);

    int projmenu = glutCreateMenu(DoProjectMenu);
    glutAddMenuEntry("Orthographic", ORTHO);
    glutAddMenuEntry("Perspective", PERSP);

    int mainmenu = glutCreateMenu(DoMainMenu);
    // glutAddSubMenu("View", viewmenu);
    glutAddSubMenu("Axes", axesmenu);
    glutAddSubMenu("Axis Colors", colormenu);

#ifdef DEMO_DEPTH_BUFFER
    glutAddSubMenu("Depth Buffer", depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
    glutAddSubMenu("Depth Fighting", depthfightingmenu);
#endif

    glutAddSubMenu("Depth Cue", depthcuemenu);
    // glutAddSubMenu("Projection", projmenu);
    glutAddMenuEntry("Reset", RESET);
    glutAddSubMenu("Debug", debugmenu);
    glutAddMenuEntry("Quit", QUIT);

    // attach the pop-up menu to the right mouse button:

    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// initialize the glut and OpenGL libraries:
//	also setup callback functions

void InitGraphics() {
    if (DebugOn != 0)
        fprintf(stderr, "Starting InitGraphics.\n");

    // request the display modes:
    // ask for red-green-blue-alpha color, double-buffering, and z-buffering:

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    // set the initial window configuration:

    glutInitWindowPosition(0, 0);
    glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);

    // open the window and set its title:

    MainWindow = glutCreateWindow(WINDOWTITLE);
    glutSetWindowTitle(WINDOWTITLE);

    // set the framebuffer clear values:

    glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);

    // setup the callback functions:
    // DisplayFunc -- redraw the window
    // ReshapeFunc -- handle the user resizing the window
    // KeyboardFunc -- handle a keyboard input
    // MouseFunc -- handle the mouse button going down or up
    // MotionFunc -- handle the mouse moving with a button down
    // PassiveMotionFunc -- handle the mouse moving with a button up
    // VisibilityFunc -- handle a change in window visibility
    // EntryFunc	-- handle the cursor entering or leaving the window
    // SpecialFunc -- handle special keys on the keyboard
    // SpaceballMotionFunc -- handle spaceball translation
    // SpaceballRotateFunc -- handle spaceball rotation
    // SpaceballButtonFunc -- handle spaceball button hits
    // ButtonBoxFunc -- handle button box hits
    // DialsFunc -- handle dial rotations
    // TabletMotionFunc -- handle digitizing tablet motion
    // TabletButtonFunc -- handle digitizing tablet button hits
    // MenuStateFunc -- declare when a pop-up menu is in use
    // TimerFunc -- trigger something to happen a certain time from now
    // IdleFunc -- what to do when nothing else is going on

    glutSetWindow(MainWindow);
    glutDisplayFunc(Display);
    glutReshapeFunc(Resize);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(MouseButton);
    glutMotionFunc(MouseMotion);
    glutPassiveMotionFunc(MouseMotion);
    // glutPassiveMotionFunc( NULL );
    glutVisibilityFunc(Visibility);
    glutEntryFunc(NULL);
    glutSpecialFunc(NULL);
    glutSpaceballMotionFunc(NULL);
    glutSpaceballRotateFunc(NULL);
    glutSpaceballButtonFunc(NULL);
    glutButtonBoxFunc(NULL);
    glutDialsFunc(NULL);
    glutTabletMotionFunc(NULL);
    glutTabletButtonFunc(NULL);
    glutMenuStateFunc(NULL);
    glutTimerFunc(-1, NULL, 0);

    // setup glut to call Animate( ) every time it has
    // 	nothing it needs to respond to (which is most of the time)
    // we don't need to do this for this program, and really should set the
    // argument to NULL but, this sets us up nicely for doing animation

    glutIdleFunc(Animate);

    // init the glew package (a window must be open to do this):

#ifdef WIN32
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "glewInit Error\n");
    } else
        fprintf(stderr, "GLEW initialized OK\n");
    fprintf(stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

    // all other setups go here, such as GLSLProgram and KeyTime setups:
}

// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void InitLists() {
    if (DebugOn != 0)
        fprintf(stderr, "Starting InitLists.\n");

    glutSetWindow(MainWindow);

    // create the object:

    HorseList = glGenLists(1);
    glNewList(HorseList, GL_COMPILE);
    glPushMatrix();
    glRotatef(90.f, 0., 1., 0.);
    glTranslatef(0., -1.1f, 0.f);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < HORSEntris; i++) {
        struct point p0 = HORSEpoints[HORSEtris[i].p0];
        struct point p1 = HORSEpoints[HORSEtris[i].p1];
        struct point p2 = HORSEpoints[HORSEtris[i].p2];

        // fake "lighting" from above:

        float p01[3], p02[3], n[3];
        p01[0] = p1.x - p0.x;
        p01[1] = p1.y - p0.y;
        p01[2] = p1.z - p0.z;
        p02[0] = p2.x - p0.x;
        p02[1] = p2.y - p0.y;
        p02[2] = p2.z - p0.z;
        Cross(p01, p02, n);
        Unit(n, n);
        n[1] = (float)fabs(n[1]);
        // simulating a glColor3f( 1., 1., 0. ) = yellow:
        glColor3f(1.f * n[1], 1.f * n[1], 0.f * n[1]);

        glVertex3f(p0.x, p0.y, p0.z);
        glVertex3f(p1.x, p1.y, p1.z);
        glVertex3f(p2.x, p2.y, p2.z);
    }
    glEnd();
    glPopMatrix();
    glEndList();

#define XSIDE 10                // length of the x side of the grid
#define X0 (-XSIDE / 2.)        // where one side starts
#define NX 100                  // how many points in x
#define DX (XSIDE / (float)NX)  // change in x between the points

#define YGRID 0.f

#define ZSIDE 10                // length of the z side of the grid
#define Z0 (-ZSIDE / 2.)        // where one side starts
#define NZ 100                  // how many points in z
#define DZ (ZSIDE / (float)NZ)  // change in z between the points

    GridDL = glGenLists(1);
    glNewList(GridDL, GL_COMPILE);
    glTranslated(0, -0.2, 0);
    SetMaterial(0.6f, 0.6f, 0.6f, 30.f);
    glNormal3f(0., 1., 0.);
    for (int i = 0; i < NZ; i++) {
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j < NX; j++) {
            glVertex3f(X0 + DX * (float)j, YGRID, Z0 + DZ * (float)(i + 0));
            glVertex3f(X0 + DX * (float)j, YGRID, Z0 + DZ * (float)(i + 1));
        }
        glEnd();
    }
    glEndList();

    DeerList = glGenLists(1);
    glNewList(DeerList, GL_COMPILE);
    LoadObjFile("deer.obj");
    glEndList();

    TeapotList = glGenLists(1);
    glNewList(TeapotList, GL_COMPILE);
    LoadObjFile("cow.obj");
    glEndList();

    DinoList = glGenLists(1);
    glNewList(DinoList, GL_COMPILE);
    LoadObjFile("dino.obj");
    glEndList();

    SphereList = glGenLists(1);
    glNewList(SphereList, GL_COMPILE);
    OsuSphere(0.2, 8, 8);
    glEndList();

    // create the axes:

    AxesList = glGenLists(1);
    glNewList(AxesList, GL_COMPILE);
    glLineWidth(AXES_WIDTH);
    Axes(4.);
    glLineWidth(1.);
    glEndList();
}

void UpdateLightSourceType(int light_type) {
    light_source_type = light_type;

    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void UpdateLightSourceColor(int light_color) {
    light_source_color = light_color;

    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

// the keyboard callback:

void Keyboard(unsigned char c, int x, int y) {
    if (DebugOn != 0)
        fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c);

    switch (c) {
            // case 'o':
            // case 'O':
            //     UpdateCurrentProjection(ORTHO);
            //     break;

            // case 'p':
            // case 'P':
            //     UpdateCurrentProjection(PERSP);
            //     break;

        case 'q':
        case 'Q':
        case ESCAPE:
            DoMainMenu(QUIT);  // will not return here
            break;             // happy compiler

        case 'p':
        case 'P':
            UpdateLightSourceType(POINT_LIGHT);
            break;

        case 's':
        case 'S':
            UpdateLightSourceType(SPOT_LIGHT);
            break;

        case 'w':
        case 'W':
            UpdateLightSourceColor(WHITE_C);
            break;

        case 'r':
        case 'R':
            UpdateLightSourceColor(RED);
            break;

        case 'b':
        case 'B':
            UpdateLightSourceColor(BLUE);
            break;

        case 'g':
        case 'G':
            UpdateLightSourceColor(GREEN);
            break;

        case 'y':
        case 'Y':
            UpdateLightSourceColor(YELLOW);
            break;

        case 'f':
        case 'F':
            Frozen = !Frozen;
            if (Frozen)
                glutIdleFunc(NULL);
            else
                glutIdleFunc(Animate);
            break;

        default:
            fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
    }

    // force a call to Display( ):

    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

// called when the mouse button transitions down or up:

void MouseButton(int button, int state, int x, int y) {
    int b = 0;  // LEFT, MIDDLE, or RIGHT

    if (DebugOn != 0)
        fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);

    // get the proper button bit mask:

    switch (button) {
        case GLUT_LEFT_BUTTON:
            b = LEFT;
            break;

        case GLUT_MIDDLE_BUTTON:
            b = MIDDLE;
            break;

        case GLUT_RIGHT_BUTTON:
            b = RIGHT;
            break;

        case SCROLL_WHEEL_UP:
            Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
            // keep object from turning inside-out or disappearing:
            if (Scale < MINSCALE)
                Scale = MINSCALE;
            break;

        case SCROLL_WHEEL_DOWN:
            Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
            // keep object from turning inside-out or disappearing:
            if (Scale < MINSCALE)
                Scale = MINSCALE;
            break;

        default:
            b = 0;
            fprintf(stderr, "Unknown mouse button: %d\n", button);
    }

    // button down sets the bit, up clears the bit:

    if (state == GLUT_DOWN) {
        Xmouse = x;
        Ymouse = y;
        ActiveButton |= b;  // set the proper bit
    } else {
        ActiveButton &= ~b;  // clear the proper bit
    }

    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

// called when the mouse moves while a button is down:

void MouseMotion(int x, int y) {
    int dx = x - Xmouse;  // change in mouse coords
    int dy = y - Ymouse;

    if ((ActiveButton & LEFT) != 0) {
        Xrot += (ANGFACT * dy);
        Yrot += (ANGFACT * dx);
    }

    if ((ActiveButton & MIDDLE) != 0) {
        Scale += SCLFACT * (float)(dx - dy);

        // keep object from turning inside-out or disappearing:

        if (Scale < MINSCALE)
            Scale = MINSCALE;
    }

    Xmouse = x;  // new current position
    Ymouse = y;

    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void Reset() {
    ActiveButton = 0;
    AxesOn = 1;
    DebugOn = 0;
    DepthBufferOn = 1;
    DepthFightingOn = 0;
    DepthCueOn = 0;
    Scale = 1.0;
    ShadowsOn = 0;
    NowColor = YELLOW;
    NowProjection = PERSP;
    Xrot = Yrot = 0.;
    light_source_type = POINT_LIGHT;
    light_source_color = YELLOW;
    Frozen = false;
}

// called when user resizes the window:

void Resize(int width, int height) {
    // don't really need to do anything since window size is
    // checked each time in Display( ):

    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

// handle a change to the window's visibility:

void Visibility(int state) {
    if (DebugOn != 0)
        fprintf(stderr, "Visibility: %d\n", state);

    if (state == GLUT_VISIBLE) {
        glutSetWindow(MainWindow);
        glutPostRedisplay();
    } else {
        // could optimize by keeping track of the fact
        // that the window is not visible and avoid
        // animating or redrawing it ...
    }
}

///////////////////////////////////////   HANDY UTILITIES:
/////////////////////////////

// the stroke characters 'X' 'Y' 'Z' :

static float xx[] = {0.f, 1.f, 0.f, 1.f};

static float xy[] = {-.5f, .5f, .5f, -.5f};

static int xorder[] = {1, 2, -3, 4};

static float yx[] = {0.f, 0.f, -.5f, .5f};

static float yy[] = {0.f, .6f, 1.f, 1.f};

static int yorder[] = {1, 2, 3, -2, 4};

static float zx[] = {1.f, 0.f, 1.f, 0.f, .25f, .75f};

static float zy[] = {.5f, .5f, -.5f, -.5f, 0.f, 0.f};

static int zorder[] = {1, 2, 3, 4, -5, 6};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void Axes(float length) {
    glBegin(GL_LINE_STRIP);
    glVertex3f(length, 0., 0.);
    glVertex3f(0., 0., 0.);
    glVertex3f(0., length, 0.);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glVertex3f(0., 0., 0.);
    glVertex3f(0., 0., length);
    glEnd();

    float fact = LENFRAC * length;
    float base = BASEFRAC * length;

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < 4; i++) {
        int j = xorder[i];
        if (j < 0) {
            glEnd();
            glBegin(GL_LINE_STRIP);
            j = -j;
        }
        j--;
        glVertex3f(base + fact * xx[j], fact * xy[j], 0.0);
    }
    glEnd();

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < 5; i++) {
        int j = yorder[i];
        if (j < 0) {
            glEnd();
            glBegin(GL_LINE_STRIP);
            j = -j;
        }
        j--;
        glVertex3f(fact * yx[j], base + fact * yy[j], 0.0);
    }
    glEnd();

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < 6; i++) {
        int j = zorder[i];
        if (j < 0) {
            glEnd();
            glBegin(GL_LINE_STRIP);
            j = -j;
        }
        j--;
        glVertex3f(0.0, fact * zy[j], base + fact * zx[j]);
    }
    glEnd();
}

// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void HsvRgb(float hsv[3], float rgb[3]) {
    // guarantee valid input:

    float h = hsv[0] / 60.f;
    while (h >= 6.)
        h -= 6.;
    while (h < 0.)
        h += 6.;

    float s = hsv[1];
    if (s < 0.)
        s = 0.;
    if (s > 1.)
        s = 1.;

    float v = hsv[2];
    if (v < 0.)
        v = 0.;
    if (v > 1.)
        v = 1.;

    // if sat==0, then is a gray:

    if (s == 0.0) {
        rgb[0] = rgb[1] = rgb[2] = v;
        return;
    }

    // get an rgb from the hue itself:

    float i = (float)floor(h);
    float f = h - i;
    float p = v * (1.f - s);
    float q = v * (1.f - s * f);
    float t = v * (1.f - (s * (1.f - f)));

    float r = 0., g = 0., b = 0.;  // red, green, blue
    switch ((int)i) {
        case 0:
            r = v;
            g = t;
            b = p;
            break;

        case 1:
            r = q;
            g = v;
            b = p;
            break;

        case 2:
            r = p;
            g = v;
            b = t;
            break;

        case 3:
            r = p;
            g = q;
            b = v;
            break;

        case 4:
            r = t;
            g = p;
            b = v;
            break;

        case 5:
            r = v;
            g = p;
            b = q;
            break;
    }

    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
}

void Cross(float v1[3], float v2[3], float vout[3]) {
    float tmp[3];
    tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
    tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
    tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
    vout[0] = tmp[0];
    vout[1] = tmp[1];
    vout[2] = tmp[2];
}

float Dot(float v1[3], float v2[3]) { return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]; }

float Unit(float vin[3], float vout[3]) {
    float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
    if (dist > 0.0) {
        dist = sqrtf(dist);
        vout[0] = vin[0] / dist;
        vout[1] = vin[1] / dist;
        vout[2] = vin[2] / dist;
    } else {
        vout[0] = vin[0];
        vout[1] = vin[1];
        vout[2] = vin[2];
    }
    return dist;
}

float Unit(float v[3]) {
    float dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    if (dist > 0.0) {
        dist = sqrtf(dist);
        v[0] /= dist;
        v[1] /= dist;
        v[2] /= dist;
    }
    return dist;
}
