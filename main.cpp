#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#pragma comment(lib, "glew32.lib")

#include "objects/gameobject.h"
#include "components/followcomponent.h"
#include "components/modelcomponent.h"
#include "components/primitivebuildcomponent.h"
#include "components/primitivedrawcomponent.h"
#include "components/smoothcomponent.h"
#include "components/spincomponent.h"
#include "objects/cube.h"
#include "objects/pane.h"
#include "scenes/gamescene.h"
#include "shaders/fbo.h"
#include "shaders/texture.h"

std::map<std::string, gamo::Texture*> gamo::Texture::cache;

// Scene + objects.
gamo::Fbo* fbo;
gamo::GameScene* scene;

// Group for colored objects (P3 N3 C4)
gamo::ShaderObjectPair<gamo::VertexP3N3C4>* colored;
gamo::GameObject<gamo::VertexP3N3C4>* camera;
gamo::GameObject<gamo::VertexP3N3C4>* cube1;

// Group for models (textured + bump: P3 N3 T2 B3)
gamo::ShaderObjectPair<gamo::VertexP3N3T2B3>* modeled;

// Group for textured objects (P3 N3 T2)
gamo::ShaderObjectPair<gamo::VertexP3N3T2>* textured;
gamo::GameObject<gamo::VertexP3N3T2>* cube2;

// Group for shadertoy textured objects (P3 N3 T2)
gamo::ShaderObjectPair<gamo::VertexP3N3T2>* toyed;
gamo::GameObject<gamo::VertexP3N3T2>* cube3;
gamo::GameObject<gamo::VertexP3N3T2>* player;

gamo::GameObject<gamo::VertexP3N3T2>* postProcessingPane;

// Models.
std::vector<gamo::GameObject<gamo::VertexP3N3T2B3>*> models;
int modelIndex = 0;
std::vector<std::pair<std::string, float>> modelInfos = {
	{ "res/models/car/honda_jazz.obj", 0.01f },
	{ "res/models/ship/shipA_OBJ.obj", 0.02f },
	{ "res/models/normalstuff/normaltest.obj", 0.4f },
	{ "res/models/normalstuff/normaltest2.obj", 0.4f },
	{ "res/models/bloemetje/PrimroseP.obj", 1.0f }
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
std::vector<gamo::Shader<gamo::VertexP3N3T2B3>*> modelShaders;
std::vector<gamo::Shader<gamo::VertexP3N3T2>*> textureShaders;
int textureShaderIndex = 0;
std::vector<std::string> textureShaderNames = {
	"res/shaders/p3n3t2-brick",
	"res/shaders/p3n3t2b3-bump",
	"res/shaders/p3n3t2-toon",
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

// Shadertoy shaders (texture 0,0 - 1,1).
std::vector<gamo::Shader<gamo::VertexP3N3T2>*> postShaders;
int postShaderIndex = 0;
std::vector<std::string> postShaderNames = {
	"res/shaders/p3n3t2-simple",
	"res/shaders/p3n3t2-texturewobble",
	"res/shaders/p3n3t2-blur",
	"res/shaders/p3n3t2-edges",
	"res/shaders/p3n3t2-pixels"
};

// Wireframe (bool)
bool wireFrame = false;

glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;

std::map<char, bool> keys;
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

template<class T>
std::vector<gamo::Uniform*> defaultUniforms(gamo::Shader<T>* shap) {
	return {
		new gamo::Matrix4Uniform("modelViewProjectionMatrix", [shap]() { return projectionMatrix * viewMatrix * shap->modelMatrix; }),
		new gamo::Matrix4Uniform("modelViewMatrix", [shap]() { return viewMatrix * shap->modelMatrix; }),
		new gamo::Matrix4Uniform("projectionMatrix", [shap]() { return projectionMatrix; }),
		new gamo::Matrix3Uniform("normalMatrix", [shap]() { return glm::transpose(glm::inverse(glm::mat3(shap->modelMatrix))); }),
		new gamo::IntegerUniform("s_texture", []() { return 0; }),
		new gamo::IntegerUniform("s_bumpmap", []() { return 1; }),
		new gamo::FloatUniform("time", []() { return lastTimeMillis / 1000.0f; })
	};
}

void init() {
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);

	fbo = new gamo::Fbo(4096, 4096);
	scene = new gamo::GameScene();
	colored = new gamo::ShaderObjectPair<gamo::VertexP3N3C4>(new gamo::GameObject<gamo::VertexP3N3C4>("coloredGroup"), nullptr);
	modeled = new gamo::ShaderObjectPair<gamo::VertexP3N3T2B3>(new gamo::GameObject<gamo::VertexP3N3T2B3>("modeledGroup"), nullptr);
	textured = new gamo::ShaderObjectPair<gamo::VertexP3N3T2>(new gamo::GameObject<gamo::VertexP3N3T2>("texturedGroup"), nullptr);
	toyed = new gamo::ShaderObjectPair<gamo::VertexP3N3T2>(new gamo::GameObject<gamo::VertexP3N3T2>("shadertoyGroup"), nullptr);
	scene->pairs.push_back(colored);
	scene->pairs.push_back(modeled);
	scene->pairs.push_back(textured);
	scene->pairs.push_back(toyed);
	
	player = new gamo::GameObject<gamo::VertexP3N3T2>();
	camera = new gamo::GameObject<gamo::VertexP3N3C4>();
	camera->addComponent(new gamo::FollowComponent(player, true, false, false));
	camera->addComponent(new gamo::SmoothComponent(0.9f, false, false, true));

	cube1 = gamo::Cubes::colored();
	cube1->position = glm::vec3(0, -1.5, 0);
	colored->group->addChildren({ cube1, camera });
	cube2 = gamo::Cubes::mcGrass();
	cube2->position = glm::vec3(-1.5, -1.5, 0);
	textured->group->addChildren({ cube2, player });
	cube3 = gamo::Cubes::mcTotal();
	cube3->position = glm::vec3(1.5, -1.5, 0);
	toyed->group->addChild(cube3);

	postProcessingPane = new gamo::GameObject<gamo::VertexP3N3T2>();
	postProcessingPane->addComponent(new gamo::TexturedPaneBuildComponent(glm::vec2(1, 1), glm::vec2(3, 3)));
	postProcessingPane->addComponent(new gamo::TextureDrawComponent(fbo->texture));
	postProcessingPane->build();

	for (std::pair<std::string, float> modelInfo : modelInfos) {
		gamo::GameObject<gamo::VertexP3N3T2B3>* mod = new gamo::GameObject<gamo::VertexP3N3T2B3>();
		mod->addComponent(new gamo::ModelComponent(modelInfo.first, modelInfo.second));
		mod->addComponent(new gamo::SpinComponent(glm::vec3(0, 10, 0)));
		mod->position = glm::vec3(0, -0.5, 0);
		models.push_back(mod);
	}
	modeled->group->addChild(models[modelIndex]);

	for (std::string shaderName : colorShaderNames) {
		gamo::Shader<gamo::VertexP3N3C4>* shap = new gamo::Shader<gamo::VertexP3N3C4>();
		shap->initFromFiles(shaderName + ".vs", shaderName + ".fs",
			gamo::AttribArrays::p3n3c4("a_position", "a_normal", "a_color"), defaultUniforms(shap));
		colorShaders.push_back(shap);
	}

	for (std::string shaderName : textureShaderNames) {
		gamo::Shader<gamo::VertexP3N3T2>* shap = new gamo::Shader<gamo::VertexP3N3T2>();
		shap->initFromFiles(shaderName + ".vs", shaderName + ".fs",
			gamo::AttribArrays::p3n3t2("a_position", "a_normal", "a_texcoord"), defaultUniforms(shap));
		textureShaders.push_back(shap);
	}

	for (std::string shaderName : textureShaderNames) {
		gamo::Shader<gamo::VertexP3N3T2B3>* shap = new gamo::Shader<gamo::VertexP3N3T2B3>();
		shap->initFromFiles(shaderName + ".vs", shaderName + ".fs",
			gamo::AttribArrays::p3n3t2b3("a_position", "a_normal", "a_texcoord", "a_tangent"), defaultUniforms(shap));
		modelShaders.push_back(shap);
	}

	for (std::string shaderName : toyShaderNames) {
		gamo::Shader<gamo::VertexP3N3T2>* shap = new gamo::Shader<gamo::VertexP3N3T2>();
		shap->initFromFiles(shaderName + ".vs", shaderName + ".fs",
			gamo::AttribArrays::p3n3t2("a_position", "a_normal", "a_texcoord"), defaultUniforms(shap));
		toyShaders.push_back(shap);
	}

	for (std::string shaderName : postShaderNames) {
		gamo::Shader<gamo::VertexP3N3T2>* shap = new gamo::Shader<gamo::VertexP3N3T2>();
		shap->initFromFiles(shaderName + ".vs", shaderName + ".fs",
			gamo::AttribArrays::p3n3t2("a_position", "a_normal", "a_texcoord"), defaultUniforms(shap));
		postShaders.push_back(shap);
	}

	if (glDebugMessageCallback) {
		glDebugMessageCallback(&onDebug, NULL);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glEnable(GL_DEBUG_OUTPUT);
	}
}

void build() {
	while (true) {
		if (scene->shouldRebuild()) {
			std::cout << "Building started.." << std::endl;
			scene->build();
			std::cout << "Done building!" << std::endl;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void display() {
	colored->shader = colorShaders[colorShaderIndex];
	colored->shader->wireframe = wireFrame;
	modeled->shader = modelShaders[textureShaderIndex];
	modeled->shader->wireframe = wireFrame;
	textured->shader = textureShaders[textureShaderIndex];
	textured->shader->wireframe = wireFrame;
	toyed->shader = toyShaders[toyShaderIndex];
	toyed->shader->wireframe = wireFrame;

	// Draw scene to texture.
	fbo->bind();
	glViewport(0, 0, fbo->texture->getWidth(), fbo->texture->getHeight());
	glClearColor(0.3f, 0.7f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	projectionMatrix = glm::perspective(80.0f, screenSize.x / (float)screenSize.y, 0.01f, 100.0f);
	viewMatrix = glm::lookAt(camera->position, camera->position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));

	scene->draw();
	fbo->unbind();

	// Draw scene-texture to quad.
	glViewport(0, 0, screenSize.x, screenSize.y);
	glClearColor(1.0f, 0.7f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	projectionMatrix = glm::identity<glm::mat4>();
	viewMatrix = glm::identity<glm::mat4>();

	postShaders[postShaderIndex]->use();
	postProcessingPane->draw(postShaders[postShaderIndex]);

	glutSwapBuffers();
}

void reshape(int newWidth, int newHeight) {
	screenSize.x = newWidth;
	screenSize.y = newHeight;
	glutPostRedisplay();
}

void keyboardUp(unsigned char key, int x, int y) {
	keys[key] = false;
}

void keyboard(unsigned char key, int x, int y) {
	keys[key] = true;

	if (key == VK_ESCAPE)
		glutLeaveMainLoop();

	if (key == 'm') {
		modeled->group->removeChild(models[modelIndex]);
		modelIndex = (modelIndex + 1) % models.size();
		modeled->group->addChild(models[modelIndex]);
	}

	if (key == 'u')
		colorShaderIndex = (colorShaderIndex + 1) % colorShaders.size();

	if (key == 'i')
		textureShaderIndex = (textureShaderIndex + 1) % textureShaders.size();

	if (key == 'o')
		toyShaderIndex = (toyShaderIndex + 1) % toyShaders.size();

	if (key == 'p')
		postShaderIndex = (postShaderIndex + 1) % postShaders.size();

	if (key == 'k')
		wireFrame = !wireFrame;
}

void update() {
	int timeMillis = glutGet(GLUT_ELAPSED_TIME);
	int elapsedMillis = timeMillis - lastTimeMillis;
	lastTimeMillis = timeMillis;
	
	glm::vec3 veloc(0, 0, 0);
	if (keys['w'])
		veloc += glm::vec3(0, 0, -1);

	if (keys['a'])
		veloc += glm::vec3(-1, 0, 0);

	if (keys['s'])
		veloc += glm::vec3(0, 0, 1);

	if (keys['d'])
		veloc += glm::vec3(1, 0, 0);

	if (keys['z'])
		veloc += glm::vec3(0, -1, 0);

	if (keys[VK_SPACE])
		veloc += glm::vec3(0, 1, 0);

	glm::normalize(veloc);
	veloc *= 0.01;
	player->position += veloc;

	scene->update(elapsedMillis / 1000.0f);

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
	glutKeyboardUpFunc(keyboardUp);
	glutIdleFunc(update);

	init();

	std::thread buildThread(build);
	lastTimeMillis = glutGet(GLUT_ELAPSED_TIME);
	
	glutMainLoop();

}