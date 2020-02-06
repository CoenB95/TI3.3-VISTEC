#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#pragma comment(lib, "glew32.lib")

#include "objects/gameobject.h"
#include "objects/cube.h"

#include "Shader.h"

gamo::GameObject<gamo::VertexP3C4>* cube = gamo::Cubes::colored();

Shader* simpleShader2;
gamo::Shader<gamo::VertexP3C4>* simpleShader;
glm::mat4 mvp;

glm::ivec2 screenSize;
float rotation;
int lastTime;

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

	simpleShader2 = new Shader("simple");

	simpleShader = new gamo::Shader<gamo::VertexP3C4>();
	simpleShader->initFromFiles("simple.vs", "simple.fs", gamo::AttribArrays::p3c4("a_position", "a_color"), {
		new gamo::Matrix4Uniform("modelViewProjectionMatrix", []() {
			return mvp * simpleShader->modelMatrix; }),
		new gamo::FloatUniform("time", []() { return lastTime / 1000.0f; })
	});

	glEnableVertexAttribArray(0);							// we gebruiken vertex attribute 0
	glEnableVertexAttribArray(1);							// en vertex attribute 1

	if (glDebugMessageCallback)
	{
		glDebugMessageCallback(&onDebug, NULL);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glEnable(GL_DEBUG_OUTPUT);
	}

	rotation = 0;
	lastTime = glutGet(GLUT_ELAPSED_TIME);
	cube->build();
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mvp = glm::perspective(80.0f, screenSize.x / (float)screenSize.y, 0.01f, 100.0f);		//begin met een perspective matrix
	mvp *= glm::lookAt(glm::vec3(0, 0, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));					//vermenigvuldig met een lookat
	//mvp = glm::translate(mvp, glm::vec3(0, 0, 1));													//verplaats de camera gewoon naar achter
	//mvp = glm::rotate(mvp, rotation, glm::vec3(0, 1, 0));											//roteer het object een beetje

	simpleShader->use();
	cube->draw(simpleShader);

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
	int time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = time - lastTime;
	
	
	rotation += elapsed / 1000.0f;
	cube->update(0.013);


	glutPostRedisplay();
	lastTime = time;
}




int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(1900, 1000);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Visualisatietechnieken");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(update);

	init();
	
	
	glutMainLoop();

}