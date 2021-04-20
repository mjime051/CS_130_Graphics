#include <GL/glew.h>
#include "application.h"
#include <iostream>
#include <cassert>
#include <cstdio>

using namespace std;

#include "obj.h"

void draw_obj(obj *o);

string getTextFile(const char* name)
{
    string ret;
    int c = 0;
    FILE* file = fopen(name, "rb");
    while(c != EOF)
    {
        c = fgetc(file);
        if(c != EOF)
            ret += c;
    }
    fclose(file);
    return ret;
}

// Call this to debug your shader. This will give you error messages when there is something wrong
void debugShader(GLuint vertexShader, GLuint fragmentShader, GLuint shaderProgram)
{
    char vertexInfoLog[1024];
    char fragmentInfoLog[1024];
    char programInfoLog[1024];
    glGetShaderInfoLog(vertexShader, 1024, NULL, vertexInfoLog);
    glGetShaderInfoLog(fragmentShader, 1024, NULL, fragmentInfoLog);
    glGetProgramInfoLog(shaderProgram, 1024, NULL, programInfoLog);
    cout << vertexInfoLog << endl;
    cout << fragmentInfoLog << endl;
    cout << programInfoLog << endl;
}

GLuint loadShader(const char* vertexfilename, const char* fragmentfilename)
{
    string vertex_file = getTextFile(vertexfilename);
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertex_file_list = vertex_file.c_str();
    glShaderSource(vertex_shader, 1, &vertex_file_list, NULL);
    glCompileShader(vertex_shader);

    string fragment_file = getTextFile(fragmentfilename);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragment_file_list = fragment_file.c_str();
    glShaderSource(fragment_shader, 1, &fragment_file_list, NULL);
    glCompileShader(fragment_shader);

    GLuint program = glCreateProgram();
    glAttachShader(program,vertex_shader);
    glAttachShader(program,fragment_shader);
    glLinkProgram(program);
    debugShader(vertex_shader, fragment_shader, program);
    return program;
}

//Loads a targa file
//Does not load with RLE compression
//be sure to save targa to be loaded with origin at bottom left
GLuint loadTarga(const char* filename)
{
    FILE* file = fopen(filename, "rb");
    char targetheader[] = {0,0,2,0,0,0,0,0,0,0,0,0};

    char header[12];
    char info[6];

    //Read the header(some magic number ensures its a targa file)
    size_t num_read=fread(header, 1, 12, file);
    assert(num_read==12);
    for(int i = 0; i < 12; ++i) if(header[i] != targetheader[i]) return 0;

    //Read the width height and bytes per pixel
    num_read=fread(info, 1, 6, file);
    assert(num_read==6);
    unsigned int width = info[0] + info[1]*256;
    unsigned int height = info[2] + info[3]*256;
    unsigned int bytesPerPixel = info[4]/8;

    unsigned int size = width*height*bytesPerPixel;

    //read the data
    unsigned char* data = new unsigned char[size];
    num_read=fread(data, 1, size, file);
    assert(num_read==size);
    fclose(file);

    GLuint texture_id = 0;
    
    // TODO: THIS IS THE PART YOU SHOULD CHANGE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Ha!!!!!!!!!
    //glGenTextures
    //glBindTexture
    //glTexParameteri
    //glTexImage2D
    glGenTextures(1,&texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    return texture_id;
}

application::application()
    : solid(true),lightAngle(0),program(0),texture(0),right_button_down(false),initial_x(0)
{
}

application::~application()
{
}

// triggered once after the OpenGL context is initialized
void application::init_event()
{

    cout << "CAMERA CONTROLS: \n  LMB: Rotate \n  MMB: Move \n  RMB: Zoom" << endl;
    cout << "KEYBOARD CONTROLS: \n  '=': Toggle wireframe mode" << endl;

    const GLfloat ambient[] = { 0.50, 0.50, 0.50, 1.0 };
    const GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    const GLfloat specular[] = { 1.0, 1.0, 1.0, 1.0 };

    // enable a light
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
    glEnable(GL_LIGHT1);

    // enable depth-testing, colored materials, and lighting
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    // normalize normals so lighting calculations are correct
    // when using GLUT primitives
    glEnable(GL_NORMALIZE);

    // enable smooth shading
    glShadeModel(GL_SMOOTH);

    glClearColor(0.9, 0.9, 1.0, 0.0);

    set_camera_for_box(vec3(-1.5,-1.5,-1.5),vec3(1.5,1.5,1.5));

    o.load("monkey.obj");

    glewExperimental = GL_TRUE;
    glewInit();

    program = loadShader("vertex.glsl", "fragment.glsl");
    texture = loadTarga("monkey.tga");
    glUseProgram(program);
}

// triggered each time the application needs to redraw
void application::draw_event()
{
    // apply our camera transformation
    apply_gl_transform();

    // Enable the light after the camera
    glPushMatrix();
    glRotatef(lightAngle, 0,1,0);
    float position[] = {0, 0.5, 5, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glPopMatrix();

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    glDisable(GL_LIGHTING);
    glColor3f(0,0.5,0);
    glBegin(GL_LINES);
    for(float x = -5; x < 5.01; x += 0.1)
    {
        glVertex3f(x,-5,-2);
        glVertex3f(x,5,-2);
    }
    for(float y = -5; y < 5.01; y += 0.1)
    {
        glVertex3f(-5,y,-2);
        glVertex3f(5,y,-2);
    }
    glEnd();
    glEnable(GL_LIGHTING);

    // draw the model
    glUseProgram(program);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPushMatrix();
    glTranslatef(0,0,1);
    glRotatef(90,1,0,0);
    glTranslatef(-.2,-1,0);
    draw_obj(&o);
    glPopMatrix();
}

// triggered when mouse is clicked
void application::mouse_click_event(int button, int button_state, int x, int y)
{
    if(button_state==GLUT_DOWN && button==GLUT_RIGHT_BUTTON)
    {
        initial_x=x;
        right_button_down=true;
    }
    if(button_state==GLUT_UP && button==GLUT_RIGHT_BUTTON)
        right_button_down=false;
}

// triggered when mouse button is held down and the mouse is
// moved
void application::mouse_move_event(int x, int y)
{
    const float sensitivity = 0.25;
    if(right_button_down)
    {
        lightAngle += (x-initial_x)*sensitivity;
        initial_x=x;
    }
}

// triggered when a key is pressed on the keyboard
void application::keyboard_event(unsigned char key, int x, int y)
{
    if(key == '=')
    {
        if (solid) {
            glPolygonMode(GL_FRONT, GL_FILL);
            glPolygonMode(GL_BACK, GL_FILL);
        } else {
            glPolygonMode(GL_FRONT, GL_LINE);
            glPolygonMode(GL_BACK, GL_LINE);
        }
    }
}

void draw_obj(obj *o)
{
    glDisable(GL_COLOR_MATERIAL);
    size_t nfaces = o->get_face_count();
    for(size_t i = 0; i < nfaces; ++i)
    {
        const obj::face& f = o->get_face(i);

        glPushMatrix();
        if(f.mat != "none") {

            const obj::material& mat = o->get_material(f.mat);

            GLfloat mat_amb[] = { mat.ka[0], mat.ka[1], mat.ka[2], 1 };
            GLfloat mat_dif[] = { mat.kd[0], mat.kd[1], mat.kd[2], 1 };
            GLfloat mat_spec[] = { mat.ks[0], mat.ks[1], mat.ks[2], 1 };
            glMaterialfv(GL_FRONT, GL_AMBIENT, mat_amb);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_dif);
            glMaterialfv(GL_FRONT, GL_SPECULAR, mat_spec);
            glMaterialf(GL_FRONT, GL_SHININESS, mat.ns);
        }


        glBegin(GL_POLYGON);
        for(size_t j = 0; j < f.vind.size(); ++j)
        {
            if(f.nind.size() == f.vind.size()) {
                const float *norm = o->get_normal(f.nind[j]);
                glNormal3fv(norm);
            }
            if (f.tex.size() == f.vind.size()) {
                const float *tex = o->get_texture_indices(f.tex[j]);
                glTexCoord2fv(tex);
            }

            const float *vert = o->get_vertex(f.vind[j]);
            glVertex3fv(vert);
        }
        glEnd();
        glPopMatrix();
    }

    glEnable(GL_COLOR_MATERIAL);
}
