#include <GL/glew.h>
#include "application.h"
#include <fstream>
#include <sstream>

using namespace std;

void draw_obj(obj *o, int material_index);

string getTextFile(const char *name) {
  string ret;
  int c = 0;
  FILE *file = fopen(name, "rb");
  while (c != EOF) {
    c = fgetc(file);
    if (c != EOF)
      ret += c;
  }
  fclose(file);
  return ret;
}

// Call this to debug your shader. This will give you error messages when there is something wrong
void debugShader(GLuint vertexShader, GLuint fragmentShader, GLuint shaderProgram) {
  std::cout << "SHADER DEBUG (ignore if empty):" << std::endl;
  if (!shaderProgram || !fragmentShader || !shaderProgram) {
    std::cerr << "  ERROR: Shaders are not properly loaded." << std::endl;
    return;
  }
  char vertexInfoLog[1024];
  char fragmentInfoLog[1024];
  char programInfoLog[1024];
  glGetShaderInfoLog(vertexShader, 1024, NULL, vertexInfoLog);
  glGetShaderInfoLog(fragmentShader, 1024, NULL, fragmentInfoLog);
  glGetProgramInfoLog(shaderProgram, 1024, NULL, programInfoLog);
  cout << "  " << vertexInfoLog << endl;
  cout << "  " << fragmentInfoLog << endl;
  cout << "  " << programInfoLog << endl;
}

GLuint loadShader(const char *vertexfilename, const char *fragmentfilename) {
  GLuint program = 0, vertex_shader_id = 0, fragment_shader_id = 0;

  // TODO: Functions you should know to implement this
  // glCreateShader
  // glShaderSource
  // glCompileShader
  // glAttachShader
  // glCreateProgram
  // glLinkProgram
  // http:// www.opengl.org/sdk/docs/man/
  // http:// www.lighthouse3d.com/tutorials/glsl-tutorial/creating-a-shader/
  // You are provided with the function getTextFile to load a string from a text file
  program = glCreateProgram();
  
  vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  const GLchar * temp1 = getTextFile(vertexfilename).c_str();
  glShaderSource(vertex_shader_id, 1, &temp1, NULL);
  glCompileShader(vertex_shader_id);
  glAttachShader(program, vertex_shader_id);
  
  fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  const GLchar * temp2 = getTextFile(fragmentfilename).c_str();
  glShaderSource(fragment_shader_id, 1, &temp2, NULL);
  glCompileShader(fragment_shader_id);
  glAttachShader(program, fragment_shader_id);
  
  glLinkProgram(program);

  debugShader(vertex_shader_id, fragment_shader_id, program);
  return program;
}

application::application()
    : solid(true), lightAngle(0), program(0), right_button_down(false), initial_x(0), material_index(0) {
}

application::~application() {
}

// triggered once after the OpenGL context is initialized
void application::init_event() {

  cout << "KEYBOARD CONTROLS: \n";
  cout << "  '=': Toggle wireframe mode\n";
  cout << "  '0': White Monkey\n";
  cout << "  '1': Ambient Monkey\n";
  cout << "  '2': Diffuse Monkey\n";
  cout << "  '3': Specular Monkey\n";
  cout << "  '4': Fancy Monkey\n" << endl;
  cout << "MOUSE CONTROLS: \n";
  cout << "  RIGHT : Rotate Light\n";
  cout << "  LEFT  : Rotate Camera\n";
  cout << "  MIDDLE: Zoom\n" << std::endl;

  const GLfloat ambient[] = {0.50, 0.50, 0.50, 1.0};
  const GLfloat diffuse[] = {1.0, 1.0, 1.0, 1.0};
  const GLfloat specular[] = {1.0, 1.0, 1.0, 1.0};

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

  set_camera_for_box(vec3(-1, -1, -1), vec3(1, 1, 1));

  o.load("monkey.obj");
  o.add_material("monkey.1.mtl");
  o.add_material("monkey.2.mtl");
  o.add_material("monkey.3.mtl");
  o.add_material("monkey.4.mtl");

  glewExperimental = GL_TRUE;
  glewInit();

  program = loadShader("vertex.glsl", "fragment.glsl");
  glUseProgram(program);
}

// triggered each time the application needs to redraw
void application::draw_event() {
  // apply our camera transformation
  apply_gl_transform();

  // Enable the light after the camera
  glPushMatrix();
  glRotatef(lightAngle, 0, 1, 0);
  float position[] = {0, 0.5, 5, 1.0};
  glLightfv(GL_LIGHT0, GL_POSITION, position);
  glPopMatrix();

  // draw the model
  draw_obj(&o, material_index);
}

// triggered when mouse is clicked
void application::mouse_click_event(int button, int button_state, int x, int y) {
  if (button_state == GLUT_DOWN && button == GLUT_RIGHT_BUTTON) {
    initial_x = x;
    right_button_down = true;
  }
  if (button_state == GLUT_UP && button == GLUT_RIGHT_BUTTON)
    right_button_down = false;
}

// triggered when mouse button is held down and the mouse is
// moved
void application::mouse_move_event(int x, int y) {
  const float sensitivity = 0.25;
  if (right_button_down) {
    lightAngle += (x - initial_x) * sensitivity;
    initial_x = x;
  }
}

// triggered when a key is pressed on the keyboard
void application::keyboard_event(unsigned char key, int x, int y) {
  if (key == '=') {
    solid = !solid;
    if (solid) {
      glPolygonMode(GL_FRONT, GL_FILL);
      glPolygonMode(GL_BACK, GL_FILL);
    } else {
      glPolygonMode(GL_FRONT, GL_LINE);
      glPolygonMode(GL_BACK, GL_LINE);
    }
  }
  int m_index = (int) key - 48;
  if (o.valid_material(m_index))
    material_index = m_index;

}

void draw_obj(obj *o, int material_index) {
  glDisable(GL_COLOR_MATERIAL);
  size_t nfaces = o->get_face_count();
  for (size_t i = 0; i < nfaces; ++i) {
    const obj::face &f = o->get_face(i);

    glPushMatrix();
    if (f.mat != "none") {

      const obj::material &mat = o->get_material(f.mat, material_index);

      GLfloat mat_amb[] = {mat.ka[0], mat.ka[1], mat.ka[2], 1};
      GLfloat mat_dif[] = {mat.kd[0], mat.kd[1], mat.kd[2], 1};
      GLfloat mat_spec[] = {mat.ks[0], mat.ks[1], mat.ks[2], 1};
      glMaterialfv(GL_FRONT, GL_AMBIENT, mat_amb);
      glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_dif);
      glMaterialfv(GL_FRONT, GL_SPECULAR, mat_spec);
      glMaterialf(GL_FRONT, GL_SHININESS, mat.ns);
    }


    glBegin(GL_POLYGON);
    for (size_t j = 0; j < f.vind.size(); ++j) {
      if (f.nind.size() == f.vind.size()) {
        const float *norm = o->get_normal(f.nind[j]);
        glNormal3fv(norm);
      }

      const float *vert = o->get_vertex(f.vind[j]);
      glVertex3fv(vert);
    }
    glEnd();
    glPopMatrix();
  }

  glEnable(GL_COLOR_MATERIAL);
}
