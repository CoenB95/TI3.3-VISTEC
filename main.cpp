#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#pragma comment(lib, "glew32.lib")

#include "objects/gameobject.h"
#include "objects/cube.h"
#include "shaders/texture.h"

#include "Shader.h"

std::map<std::string, gamo::Texture*> gamo::Texture::cache;

gamo::GameObject<gamo::VertexP3C4>* cube;
gamo::GameObject<gamo::VertexP3N3T2>* cube2;

gamo::Shader<gamo::VertexP3C4>* shader1;
gamo::Shader<gamo::VertexP3N3T2>* shader2;
gamo::Shader<gamo::VertexP3C4>* shader3;
glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;

glm::ivec2 screenSize;
float rotation;
int lastTimeMillis;

#ifdef WIN32
void GLAPIENTRY onDebug(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
#else
void onDebug(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
#endif
{
	std::cout << message << std::endl;
}

void init()
{
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glClearColor(1, 0.7f, 0.3f, 1.0f);

	cube = gamo::Cubes::colored();
	cube2 = gamo::Cubes::textured();

	shader1 = new gamo::Shader<gamo::VertexP3C4>();
	shader1->initFromFiles("simple.vs", "simple.fs", gamo::AttribArrays::p3c4("a_position", "a_color"), {
		new gamo::Matrix4Uniform("modelViewProjectionMatrix", []() { return projectionMatrix * viewMatrix * shader1->modelMatrix; }),
		new gamo::FloatUniform("time", []() { return lastTimeMillis / 1000.0f; })
	});

	shader2 = new gamo::Shader<gamo::VertexP3N3T2>();
	shader2->initFromFiles("res/shaders/textureanim.vs", "res/shaders/textureanim.fs", gamo::AttribArrays::p3n3t2("a_position", "a_normal", "a_texcoord"), {
		new gamo::Matrix4Uniform("modelMatrix", []() { return shader2->modelMatrix; }),
		new gamo::Matrix4Uniform("viewMatrix", []() { return viewMatrix; }),
		new gamo::Matrix4Uniform("projectionMatrix", []() { return projectionMatrix; }),
		new gamo::IntegerUniform("s_texture", []() { return 0; }),
		new gamo::FloatUniform("time", []() { return lastTimeMillis / 1000.0f; })
	});

	if (glDebugMessageCallback)
	{
		glDebugMessageCallback(&onDebug, NULL);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glEnable(GL_DEBUG_OUTPUT);
	}

	rotation = 0;
	lastTimeMillis = glutGet(GLUT_ELAPSED_TIME);
	cube->build();
	cube2->build();
	cube2->position = glm::vec3(0.1, 0, 0);
}

void display() {
	glViewport(0, 0, screenSize.x, screenSize.y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	projectionMatrix = glm::perspective(80.0f, screenSize.x / (float)screenSize.y, 0.01f, 100.0f);
	viewMatrix = glm::lookAt(glm::vec3(0, 0, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	shader1->use();
	cube->draw(shader1);

	shader2->use();
	cube2->draw(shader2);

	glutSwapBuffers();
}

void reshape(int newWidth, int newHeight)
{
	screenSize.x = newWidth;
	screenSize.y = newHeight;
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == VK_ESCAPE)
		glutLeaveMainLoop();
}

void update()
{
	int timeMillis = glutGet(GLUT_ELAPSED_TIME);
	int elapsedMillis = timeMillis - lastTimeMillis;
	lastTimeMillis = timeMillis;
	
	//rotation += elapsedMillis / 1000.0f;
	cube->update(elapsedMillis / 1000.0f);
	cube2->update(elapsedMillis / 1000.0f);

	glutPostRedisplay();
}




int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(800, 450);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Visualisatietechnieken");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(update);

	init();
	
	
	glutMainLoop();

}