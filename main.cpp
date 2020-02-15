#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#pragma comment(lib, "glew32.lib")

#include "components/primitivedrawcomponent.h"
#include "objects/gameobject.h"
#include "objects/cube.h"
#include "objects/pane.h"
#include "scenes/gamescene.h"
#include "shaders/texture.h"

#include "Shader.h"

std::map<std::string, gamo::Texture*> gamo::Texture::cache;

// Scene + objects.
gamo::GameScene scene;
gamo::GameObject<gamo::VertexP3C4>* cube1;
gamo::GameObject<gamo::VertexP3N3T2>* cube2;
gamo::GameObject<gamo::VertexP3N3T2>* pane1;

// Color shaders.
std::vector<gamo::Shader<gamo::VertexP3C4>*> colorShaders;
int colorShaderIndex = 0;
std::vector<std::string> colorShaderNames = {
	"simple"
};

// Texture shaders.
std::vector<gamo::Shader<gamo::VertexP3N3T2>*> textureShaders;
int textureShaderIndex = 0;

// Wireframe (bool)
bool wireFrame = false;

std::vector<std::string> textureShaderNames = {
	"res/shaders/texture",
	"res/shaders/simple",
	"res/shaders/textureanim",
	"res/shaders/vertexanim",
	"res/shaders/multitex",
	"res/shaders/thunder"
};

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

	scene = gamo::GameScene();
	cube1 = gamo::Cubes::colored();
	cube2 = gamo::Cubes::mcGrass();
	pane1 = gamo::Panes::mcAll();
	scene.colored->addChildren({ cube1 });
	scene.textured->addChildren({ cube2, pane1 });

	for (std::string shaderName : colorShaderNames) {
		gamo::Shader<gamo::VertexP3C4>* shap = new gamo::Shader<gamo::VertexP3C4>();
		shap->initFromFiles(shaderName + ".vs", shaderName + ".fs", gamo::AttribArrays::p3c4("a_position", "a_color"), {
			new gamo::Matrix4Uniform("modelViewProjectionMatrix", [shap]() { return projectionMatrix * viewMatrix * shap->modelMatrix; }),
			new gamo::FloatUniform("time", []() { return lastTimeMillis / 1000.0f; })
			});
		colorShaders.push_back(shap);
	}

	for (std::string shaderName : textureShaderNames) {
		gamo::Shader<gamo::VertexP3N3T2>* shap = new gamo::Shader<gamo::VertexP3N3T2>();
		shap->initFromFiles(shaderName + ".vs", shaderName + ".fs", gamo::AttribArrays::p3n3t2("a_position", "a_normal", "a_texcoord"), {
			new gamo::Matrix4Uniform("modelMatrix", [shap]() { return shap->modelMatrix; }),
			new gamo::Matrix4Uniform("viewMatrix", []() { return viewMatrix; }),
			new gamo::Matrix4Uniform("projectionMatrix", []() { return projectionMatrix; }),
			new gamo::Matrix3Uniform("normalMatrix", [shap]() { return glm::transpose(glm::inverse(glm::mat3(viewMatrix * shap->modelMatrix))); }),
			new gamo::IntegerUniform("s_texture", []() { return 0; }),
			new gamo::FloatUniform("time", []() { return lastTimeMillis / 1000.0f; })
			});
		textureShaders.push_back(shap);
	}

	if (glDebugMessageCallback) {
		glDebugMessageCallback(&onDebug, NULL);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glEnable(GL_DEBUG_OUTPUT);
	}

	rotation = 0;
	lastTimeMillis = glutGet(GLUT_ELAPSED_TIME);
	
	cube2->position = glm::vec3(1.5, 0, 0);
	pane1->position = glm::vec3(-1.5, 0, 0);
}

void display() {
	glViewport(0, 0, screenSize.x, screenSize.y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	projectionMatrix = glm::perspective(80.0f, screenSize.x / (float)screenSize.y, 0.01f, 100.0f);
	viewMatrix = glm::lookAt(glm::vec3(0, 0, 2), glm::vec3(0, 0, 0), glm::vec3(0, -1, 0));

	scene.coloredShader = colorShaders[colorShaderIndex];
	scene.coloredShader->wireframe = wireFrame;
	scene.texturedShader = textureShaders[textureShaderIndex];
	scene.texturedShader->wireframe = wireFrame;
	scene.draw();

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

	if (key == 'c')
		colorShaderIndex = (colorShaderIndex + 1) % colorShaders.size();

	if (key == 't')
		textureShaderIndex = (textureShaderIndex + 1) % textureShaders.size();

	if (key == 'w')
		wireFrame = !wireFrame;
}

void update()
{
	int timeMillis = glutGet(GLUT_ELAPSED_TIME);
	int elapsedMillis = timeMillis - lastTimeMillis;
	lastTimeMillis = timeMillis;
	
	scene.update(elapsedMillis / 1000.0f);

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