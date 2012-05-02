
/**

  Last change 30.04.24 by Felix Weißig

  Email: thesleeper@gmx.net
  skype: otacocato
  URL:   http://www.otaco.de

**/

/**
  Step 00: glut template

  This is a simple glut template we will use in the tutorials.

**/


////////////////////////////////////////////////////////////////////////////////


/// c++ includes
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <cmath>
#include <stdlib.h>


////////////////////////////////////////////////////////////////////////////////


/// general setup
static unsigned int g_screenWidth  = 1280;
static unsigned int g_screenHeight = 640;

/// frameCounter
float g_frameCounter = 0;


/// includes and globals for this demo /////////////////////////////////////////


/// this header includes many nice shapes to draw and helpers

#include <gloost/gloostRenderGoodies.h>

void init();
void cleanup();
void draw3d(void);
void draw2d();
void resize(int width, int height);
void key(unsigned char key, int x, int y);
void motionFunc(int mouse_h, int mouse_v);
void mouseFunc(int button, int state, int mouse_h, int mouse_v);
void idle(void);


#include <gloost/Vbo4.h>
gloost::Vbo4* g_vbo4 = 0;

#include <gloost/Ibo.h>
gloost::Ibo* g_ibo = 0;

#include <gloost/Mesh.h>
#include <gloost/InterleavedAttributes.h>
#include <gloost/PlyLoader.h>
#include <gloost/GbmLoader.h>
#include <gloost/ObjLoader.h>

#include <gloost/ShaderProgram.h>
gloost::ShaderProgram* g_shader = 0;

#include <gloost/MatrixStack.h>


#include <gloost/gloostGlUtil.h>
#include <gloost/UniformSet.h>
#include <gloost/serializers.h>

#include <chrono>
#include <thread>

#include <SvoLoader.h>


////////////////////////////////////////////////////////////////////////////////


void draw();
void draw3d();
void draw2d();


////////////////////////////////////////////////////////////////////////////////

  /// load and initialize resources for our demo

void init()
{
  gloost::PlyLoader loader1("../data/meshes/frog2_seperated.ply");
  gloost::PlyLoader loader2("../data/meshes/frog2_seperated_ao.ply");

  for (unsigned i=0; i!=loader1.getMesh()->getVertices().size(); ++i)
  {
    loader1.getMesh()->getColors()[i].r = loader1.getMesh()->getColors()[i].r * loader2.getMesh()->getColors()[i].r* loader2.getMesh()->getColors()[i].r;
    loader1.getMesh()->getColors()[i].g = loader1.getMesh()->getColors()[i].g * loader2.getMesh()->getColors()[i].r* loader2.getMesh()->getColors()[i].r;
    loader1.getMesh()->getColors()[i].b = loader1.getMesh()->getColors()[i].b * loader2.getMesh()->getColors()[i].r* loader2.getMesh()->getColors()[i].r;
  }

  gloost::Mesh* mesh = loader1.getMesh();
  mesh->takeReference();
  mesh->center();
  mesh->scaleToSize(2.0);

  g_vbo4 = new gloost::Vbo4(new gloost::InterleavedAttributes(mesh));
  g_ibo  = new gloost::Ibo(mesh);


  // shader
  g_shader = new gloost::ShaderProgram();
  g_shader->attachShader(GLOOST_SHADERPROGRAM_VERTEX_SHADER,   "../data/shaders/simpleVertexShader330.vs");
  g_shader->attachShader(GLOOST_SHADERPROGRAM_FRAGMENT_SHADER, "../data/shaders/simpleFragmentShader330.fs");


  //
  int tb_size;
  glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE_ARB, &tb_size);
  std::cerr << std::endl << "GL_MAX_TEXTURE_BUFFER_SIZE_ARB: " << tb_size;



  svo::SvoLoader cpuSvo("/home/otaco/Desktop/SVO_DATA/frog2_seperated_ao_8");


}

////////////////////////////////////////////////////////////////////////////////


  /// free all resources we created

void cleanup()
{
  g_ibo->dropReference();
  g_vbo4->dropReference();
}


//////////////////////////////////////////////////////////////////////////////////////////


void
draw()
{
  /// increment the frame counter
  ++g_frameCounter;

  /// clear the frame buffer and the depth buffer
  glClearColor(0.25, 0.25, 0.25, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /// set the viewport
  glViewport(0, 0, g_screenWidth, g_screenHeight);

  draw3d();
  draw2d();
}


//////////////////////////////////////////////////////////////////////////////////////////


  /// main loop function, render with 3D setup

void draw3d(void)
{
  // uniform set
  static gloost::UniformSet uniforms;
  uniforms.set_float("time", g_frameCounter);


  /// projection
  gloost::Matrix projectionMatrix;
  projectionMatrix.setIdentity();

  gloost::gloostPerspective( projectionMatrix,
                             46.0f,                                  /// camera view angle
                             (g_screenWidth)/(float)(g_screenHeight),/// aspect ratio
                             0.1f,                                   /// dist. to near plane
                             120.0f);                                /// dist. to far plane

  uniforms.set_mat4("ProjectionMatrix", projectionMatrix);


  // view matrix
  gloost::Matrix viewMatrix;
  viewMatrix.setIdentity();

  gloost::gloostLookAt(viewMatrix,
                       gloost::Point3(0.0,0.0, 3.0),
                       gloost::Point3(0.0,0.0,0.0),
                       gloost::Vector3(0.0,1.0,0.0));


  // Matrix stack to accumulate the modelViewMatrix
  static gloost::MatrixStack matrixStack;
  matrixStack.loadMatrix(viewMatrix);


  /// set the shader
  g_shader->use();

  matrixStack.push();
  {
    matrixStack.rotate(g_frameCounter*0.01, 0.0, 1.0, 0.0);

    uniforms.set_mat4("ModelViewMatrix", matrixStack.top());
    uniforms.set_mat4("NormalMatrix", gloost::modelViewMatrixToNormalMatrix(matrixStack.top()));

    uniforms.applyToShader(g_shader->getHandle());

    g_vbo4->bind();
    g_ibo->bind();
      g_ibo->draw();
    g_ibo->unbind();
    g_vbo4->unbind();
  }
  matrixStack.pop();

  g_shader->disable();


  // draw stuff with 2d camera setup
//  draw2d();

  /// swap buffers
  glutSwapBuffers();
}


//////////////////////////////////////////////////////////////////////////////////////////


  ///

void draw2d()
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  {

    glViewport(0, 0, g_screenWidth, g_screenHeight);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_LIGHTING);


    // uniform set
    static gloost::UniformSet uniforms;
    uniforms.set_float("time", g_frameCounter);

    /// projection
    gloost::Matrix projectionMatrix;
    projectionMatrix.setIdentity();

    gloost::gloostOrtho(projectionMatrix,
                        0, g_screenWidth,
                        0, g_screenHeight,
                        -100.0, 100.0);

    uniforms.set_mat4("ProjectionMatrix", projectionMatrix);


    // view matrix
//    gloost::Matrix viewMatrix;
//    viewMatrix.setIdentity();
//
//    gloost::gloostLookAt(viewMatrix,
//                         gloost::Point3(0.0,0.0, 10.0),
//                         gloost::Point3(0.0,0.0,0.0),
//                         gloost::Vector3(0.0,1.0,0.0));

    // Matrix stack to accumulate the modelViewMatrix
    static gloost::MatrixStack matrixStack;
//    matrixStack.loadMatrix(viewMatrix);

    /// set the shader
    g_shader->use();

    matrixStack.push();
    {
      matrixStack.translate(100.0, 100.0, 0.0);
      matrixStack.rotate(g_frameCounter*0.01, 0.0, 1.0, 0.0);
      matrixStack.scale(100.0);

      uniforms.set_mat4("ModelViewMatrix", matrixStack.top());
      uniforms.set_mat4("NormalMatrix", gloost::modelViewMatrixToNormalMatrix(matrixStack.top()));
      uniforms.applyToShader(g_shader->getHandle());

      g_vbo4->bind();
      g_ibo->bind();
        g_ibo->draw();
      g_ibo->unbind();
      g_vbo4->unbind();
    }
    matrixStack.pop();


    g_shader->disable();

  }
  glPopAttrib();
}


////////////////////////////////////////////////////////////////////////////////


  /// this function is triggered when the screen is resized

void resize(int width, int height)
{
  /// update the global screen size
  g_screenWidth  = width;
  g_screenHeight = height;

  /// recalc the frustum
  glViewport(0, 0, width, height);
}


//////////////////////////////////////////////////////////////////////////////////////////


  /// this function is called by glut when a key press occured

void key(unsigned char key, int x, int y)
{

  switch (key)
  {
    /// press ESC or q to quit
    case 27 :
    case 'q':
        cleanup();
        exit(0);
        break;

    case 'r':
        g_shader->reloadShaders();
        break;
    case 'a':
        g_vbo4->removeFromContext();
        break;

    /// press f for fullscreen mode
    case 'f':
        glutFullScreen();
        break;
  }
}


//////////////////////////////////////////////////////////////////////////////////////////


void motionFunc(int mouse_h, int mouse_v)
{



}


//////////////////////////////////////////////////////////////////////////////////////////


void mouseFunc(int button, int state, int mouse_h, int mouse_v)
{



}


//////////////////////////////////////////////////////////////////////////////////////////


  /// this function is triggered by glut,
  /// when nothing is left to do for this frame

void idle(void)
{
    glutPostRedisplay();
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char *argv[])
{

  /// enable sync to vblank on linux to control the demo fps
  #ifdef GLOOST_GNULINUX
    setenv("__GL_SYNC_TO_VBLANK","1",true);
  #endif


  /// initialize glut
  glutInit(&argc, argv);
  glutInitWindowSize(g_screenWidth, g_screenHeight);
  glutInitWindowPosition(400,10);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);


  /// open the window
  glutCreateWindow("gloost_vbo4");


  /// set glut callbacks for resizing, frameloop, keyboard input and idle
  glutReshapeFunc(resize);
  glutDisplayFunc(draw);
  glutKeyboardFunc(key);
  glutIdleFunc(idle);
  glutMotionFunc(motionFunc);
  glutPassiveMotionFunc(motionFunc);
  glutMouseFunc(mouseFunc);


  /// initialize GLEW
	if (GLEW_OK != glewInit())
	{
	  /// ... or die trying
	  std::cout << "'glewInit()' failed." << std::endl;
	  exit(0);
  }


  /// set some gl states
  glEnable(GL_TEXTURE_2D);   /// enable texture support
  glEnable(GL_DEPTH_TEST);   /// enable z-buffering
  glDepthFunc(GL_LESS);      /// set a depth function (draw only if a new fragment is closer to the eye as the pixel allready in the buffer)


  /// load and intialize stuff for our demo
  init();


  /// start the loop (this will call display() every frame)
  glutMainLoop();


  /// cleanup all resources we created for the demo
  cleanup();


  return EXIT_SUCCESS;
}

