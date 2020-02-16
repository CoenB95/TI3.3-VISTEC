#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#pragma comment(lib, "glew32.lib")

#include "components/modelcomponent.h"
#include "components/primitivedrawcomponent.h"
#include "components/spincomponent.h"
#include "objects/gameobject.h"
#include "objects/cube.h"
#include "objects/pane.h"
#include "scenes/gamescene.h"
#include "shaders/texture.h"

std::map<std::string, gamo::Texture*> gamo::Texture::cache;

// Scene + objects.
gamo::GameScene scene;
gamo::ShaderObjectPair<gamo::VertexP3N3C4>* colored;
gamo::GameObject<gamo::VertexP3N3C4>* cube1;

gamo::ShaderObjectPair<gamo::VertexP3N3T2>* textured;
gamo::GameObject<gamo::VertexP3N3T2>* cube2;

gamo::ShaderObjectPair<gamo::VertexP3N3T2>* toyed;
gamo::GameObject<gamo::VertexP3N3T2>* cube3;

// Models.
std::vector<gamo::GameObject<gamo::VertexP3N3T2>*> models;
int modelIndex = 0;
std::vector<std::pair<std::string, double>> modelInfos = {
	{ "res/models/car/honda_jazz.obj", 0.01 },
	{ "res/models/ship/shipA_OBJ.obj", 0.02 },
	{ "res/models/normalstuff/normaltest.obj", 0.4 },
	{ "res/models/normalstuff/normaltest2.obj", 0.4 },
	{ "res/models/bloemetje/PrimroseP.obj", 1.0 }
};

// Color shaders.
std::vector<gamo::Shader<gamo::VertexP3N3C4>*> colorShaders;
int colorShaderIndex = 0;
std::vector<std::string> colorShaderNames = {
	"res/shaders/p3n3c4-simple",
	"res/shaders/p3n3c4-specular",
	"res/shaders/p3n3c4-specularcolor",
	"res/shaders/p3n3c4-gray"
};

// Texture shaders.
std::vector<gamo::Shader<gamo::VertexP3N3T2>*> textureShaders;
int textureShaderIndex = 0;
std::vector<std::string> textureShaderNames = {
	"res/shaders/p3n3t2-simple",
	"res/shaders/p3n3t2-specular",
	"res/shaders/p3n3t2-speculartex",
	"res/shaders/p3n3t2-gray",
	"res/shaders/p3n3t2-textureanim",
	"res/shaders/p3n3t2-vertexanim",
	"res/shaders/p3n3t2-multitex"
};

// Shadertoy shaders (texture 0,0 - 1,1).
std::vector<gamo::Shader<gamo::VertexP3N3T2>*> toyShaders;
int toyShaderIndex = 0;
std::vector<std::string> toyShaderNames = {
	"res/shaders/p3n3t2-simple",
	"res/shaders/p3n3c4-specular",
	"res/shaders/p3n3t2-thunder"
};

// Wireframe (bool)
bool wireFrame = false;

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

void init() {
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glClearColor(1, 0.7f, 0.3f, 1.0f);

	scene = gamo::GameScene();
	colored = new gamo::ShaderObjectPair<gamo::VertexP3N3C4>(new gamo::GameObject<gamo::VertexP3N3C4>("coloredGroup"), nullptr);
	textured = new gamo::ShaderObjectPair<gamo::VertexP3N3T2>(new gamo::GameObject<gamo::VertexP3N3T2>("texturedGroup"), nullptr);
	toyed = new gamo::ShaderObjectPair<gamo::VertexP3N3T2>(new gamo::GameObject<gamo::VertexP3N3T2>("shadertoyGroup"), nullptr);
	scene.pairs.push_back(colored);
	scene.pairs.push_back(textured);
	scene.pairs.push_back(toyed);
	cube1 = gamo::Cubes::colored();
	cube1->position = glm::vec3(0, -1.5, 0);
	colored->group->addChild(cube1);
	cube2 = gamo::Cubes::mcGrass();
	cube2->position = glm::vec3(-1.5, -1.5, 0);
	textured->group->addChild(cube2);
	cube3 = gamo::Cubes::mcTotal();
	cube3->position = glm::vec3(1.5, -1.5, 0);
	toyed->group->addChild(cube3);

	for (std::pair<std::string, double> modelInfo : modelInfos) {
		gamo::GameObject<gamo::VertexP3N3T2>* mod = new gamo::GameObject<gamo::VertexP3N3T2>();
		mod->addComponent(new gamo::ModelComponent(modelInfo.first, modelInfo.second));
		mod->addComponent(new gamo::SpinComponent<gamo::VertexP3N3T2>(glm::vec3(0, 10, 0)));
		mod->position = glm::vec3(0, -0.5, 0);
		models.push_back(mod);
	}
	textured->group->addChild(models[modelIndex]);

	for (std::string shaderName : colorShaderNames) {
		gamo::Shader<gamo::VertexP3N3C4>* shap = new gamo::Shader<gamo::VertexP3N3C4>();
		shap->initFromFiles(shaderName + ".vs", shaderName + ".fs", gamo::AttribArrays::p3n3c4("a_position", "a_normal", "a_color"), {
			new gamo::Matrix4Uniform("modelViewProjectionMatrix", [shap]() { return projectionMatrix * viewMatrix * shap->modelMatrix; }),
			new gamo::Matrix3Uniform("normalMatrix", [shap]() { return glm::transpose(glm::inverse(glm::mat3(viewMatrix * shap->modelMatrix))); }),
			new gamo::FloatUniform("time", []() { return lastTimeMillis / 1000.0f; })
			});
		colorShaders.push_back(shap);
	}

	for (std::string shaderName : textureShaderNames) {
		gamo::Shader<gamo::VertexP3N3T2>* shap = new gamo::Shader<gamo::VertexP3N3T2>();
		shap->initFromFiles(shaderName + ".vs", shaderName + ".fs", gamo::AttribArrays::p3n3t2("a_position", "a_normal", "a_texcoord"), {
			new gamo::Matrix4Uniform("modelViewProjectionMatrix", [shap]() { return projectionMatrix * viewMatrix * shap->modelMatrix; }),
			new gamo::Matrix3Uniform("normalMatrix", [shap]() { return glm::transpose(glm::inverse(glm::mat3(viewMatrix * shap->modelMatrix))); }),
			new gamo::IntegerUniform("s_texture", []() { return 0; }),
			new gamo::FloatUniform("time", []() { return lastTimeMillis / 1000.0f; })
			});
		textureShaders.push_back(shap);
	}

	for (std::string shaderName : toyShaderNames) {
		gamo::Shader<gamo::VertexP3N3T2>* shap = new gamo::Shader<gamo::VertexP3N3T2>();
		shap->initFromFiles(shaderName + ".vs", shaderName + ".fs", gamo::AttribArrays::p3n3t2("a_position", "a_normal", "a_texcoord"), {
			new gamo::Matrix4Uniform("modelViewProjectionMatrix", [shap]() { return projectionMatrix * viewMatrix * shap->modelMatrix; }),
			new gamo::Matrix3Uniform("normalMatrix", [shap]() { return glm::transpose(glm::inverse(glm::mat3(viewMatrix * shap->modelMatrix))); }),
			new gamo::IntegerUniform("s_texture", []() { return 0; }),
			new gamo::FloatUniform("time", []() { return lastTimeMillis / 1000.0f; })
			});
		toyShaders.push_back(shap);
	}

	if (glDebugMessageCallback) {
		glDebugMessageCallback(&onDebug, NULL);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glEnable(GL_DEBUG_OUTPUT);
	}
}

void build() {
	while (true) {
		if (scene.shouldRebuild()) {
			std::cout << "Building started.." << std::endl;
			scene.build();
			std::cout << "Done building!" << std::endl;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void display() {
	glViewport(0, 0, screenSize.x, screenSize.y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	projectionMatrix = glm::perspective(80.0f, screenSize.x / (float)screenSize.y, 0.01f, 100.0f);
	viewMatrix = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, -1, 0));

	colored->shader = colorShaders[colorShaderIndex];
	colored->shader->wireframe = wireFrame;
	textured->shader = textureShaders[textureShaderIndex];
	textured->shader->wireframe = wireFrame;
	toyed->shader = toyShaders[toyShaderIndex];
	toyed->shader->wireframe = wireFrame;
	scene.draw();

	glutSwapBuffers();
}

void reshape(int newWidth, int newHeight) {
	screenSize.x = newWidth;
	screenSize.y = newHeight;
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	if (key == VK_ESCAPE)
		glutLeaveMainLoop();

	if (key == 'm') {
		textured->group->removeChild(models[modelIndex]);
		modelIndex = (modelIndex + 1) % models.size();
		textured->group->addChild(models[modelIndex]);
	}

	if (key == 'c')
		colorShaderIndex = (colorShaderIndex + 1) % colorShaders.size();

	if (key == 't')
		textureShaderIndex = (textureShaderIndex + 1) % textureShaders.size();

	if (key == 's')
		toyShaderIndex = (toyShaderIndex + 1) % toyShaders.size();

	if (key == 'w')
		wireFrame = !wireFrame;
}

void update() {
	int timeMillis = glutGet(GLUT_ELAPSED_TIME);
	int elapsedMillis = timeMillis - lastTimeMillis;
	lastTimeMillis = timeMillis;
	
	scene.update(elapsedMillis / 1000.0f);

	glutPostRedisplay();
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitWindowSize(800, 450);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Visualisatietechnieken");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(update);

	init();

	std::thread buildThread(build);
	lastTimeMillis = glutGet(GLUT_ELAPSED_TIME);
	
	glutMainLoop();

}