//sources:
//	camera movement
//	https://www.youtube.com/watch?v=AWM4CUfffos&ab_channel=BrianWill
//	


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stack>
#include <glm\glm.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <SOIL2/soil2.h>

#include "Utils.h"
#include "Pyramid.h"
#include "GroundPlane.h"
#include "CubeMap.h"
#include "Torus.h"
#include "Sphere.h"
#include "ImportedModel.h"

using namespace std;

#define numVAOs 1
#define numVBOs 15


//*********************
//settings, inputs, etc
//screen
int screenWidth = 800;
int screenHeight = 800;
float aspect;
//camera
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);	//relative to cam position, which direction is camera facing
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);		//relative to cam position, which way is up
float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;
float cameraSpd = 5.0f;
bool isSprinting = false;
bool hasBeenToggled = false;
float actualCamSpd;
//mouse
float mouseSensitivity = 0.1f;
bool firstMouse = true;
float lastX = (float)screenWidth / 2;
float lastY = (float)screenHeight / 2;


//**************************
//objects & their properties (material, positional, etc)
//pyramid
Pyramid pyramid(2.0f, glm::vec3(0.0f, 3.5f, 0.0f));
float* pyrMatAmb = Utils::silverAmbient();
float* pyrMatDif = Utils::silverDiffuse();
float* pyrMatSpe = Utils::silverSpecular();
float pyrMatShi = Utils::silverShininess();
//ground plane
GroundPlane groundplane(20.0f, glm::vec3(0.0f, 0.0f, 0.0f));
float* gpMatAmb = Utils::bronzeAmbient();
float* gpMatDif = Utils::bronzeDiffuse();
float* gpMatSpe = Utils::bronzeSpecular();
float gpMatShi = Utils::bronzeShininess();
//torus
Torus torus(1.0f, 0.6f, 48);
glm::vec3 torusPos = glm::vec3(-4.0f, 3.0f, -1.0f);
float* torusMatAmb = Utils::jadeAmbient();
float* torusMatDif = Utils::jadeDiffuse();
float* torusMatSpe = Utils::jadeSpecular();
float torusMatShi = Utils::jadeShininess();
float rotAmt_torus = 0.0f;
//sphere
Sphere sphere(48);
glm::vec3 spherePos = glm::vec3(2.0f, 3.0f, 3.0f);
float* sphereMatAmb = Utils::goldAmbient();
float* sphereMatDif = Utils::goldDiffuse();
float* sphereMatSpe = Utils::goldSpecular();
float sphereMatShi = Utils::goldShininess();
float rotAmt_sphere = 0.0f;
//shuttle
ImportedModel spaceship("shuttle.obj");
glm::vec3 spaceshipPos = glm::vec3(spherePos.x + 2.0f, spherePos.y, spherePos.z + 2.0f);
float* shipMatAmb = Utils::silverAmbient();
float* shipMatDif = Utils::silverDiffuse();
float* shipMatSpe = Utils::silverSpecular();
float shipMatShi = Utils::silverShininess();
float rotAmt_ship = 0.0f;

//cube map
CubeMap cubemap(cameraPos);
//textures
GLuint spaceshipTexture;
GLuint skyboxTexture;
GLuint cobbleTexture;
GLuint diamTexture;
GLuint grassTexture;
GLuint classyTexture;
//for light reflectivity
GLuint mambLoc, mdifLoc, mspeLoc, mshiLoc;		//material uniforms
GLuint matType;			//uniform for telling how much to factor in texture vs light reflection. 0 (all texture), 1 (50/50), 2 (all light), 3 (50/50) cubemap/texture, 4 (all cubemap)
float thisAmb[4], thisDif[4], thisSpe[4], thisShi;	//for passing into uniforms from installLights

//*************************
//lights & their properties
//global ambient
GLuint globalAmbLoc;	//location for shader uniform variable
float globalAmbient[4] = { 0.7f, 0.7f, 0.7f, 1.0f };		// white light
//positional light a, green
GLuint ambLoc_pa, diffLoc_pa, specLoc_pa, posLoc_pa;		//locations for shader uniforms
glm::vec3 currentLightPos_pa, transformed_pa;
float lightAmbient_pa[4] = { 0.0f, 0.0f, 0.2f, 1.0f };
float lightDiffuse_pa[4] = { 0.2f, 0.2f, 1.0f, 1.0f };
float lightSpecular_pa[4] = { 0.2f, 0.2f, 1.0f, 1.0f };
glm::vec3 lightLoc_pa = glm::vec3(-7.0f, 5.0f, 0.0f);
float lightPos_pa[3];
bool lightIsOn_pa = true;
//positional light b, red
GLuint ambLoc_pb, diffLoc_pb, specLoc_pb, posLoc_pb;
glm::vec3 currentLightPos_pb, transformed_pb;
float lightAmbient_pb[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuse_pb[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
float lightSpecular_pb[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
glm::vec3 lightLoc_pb = glm::vec3(1.0f, 3.0f, 0.0f);
float lightPos_pb[3];
bool lightIsOn_pb = true;

//********
//shadows
int scSizeX, scSizeY;
//positional light a, green
GLuint shadowTex_pa, shadowBuffer_pa;	//in setupShadowBuffers
GLuint sLoc_pa;		//in passOne
glm::mat4 lightVMat_pa;
glm::mat4 lightPMat_pa;
glm::mat4 shadowMVP_pa;
//positional light b, red
GLuint shadowTex_pb, shadowBuffer_pb;	//in setupShadowBuffers
GLuint sLoc_pb;		//in passOne
glm::mat4 lightVMat_pb;
glm::mat4 lightPMat_pb;
glm::mat4 shadowMVP_pb;
//general use
glm::mat4 b;
glm::vec3 lightOrigin(0.0f, 0.0f, 0.0f);	//what is the origin relative to a light
glm::vec3 lightUp(0.0f, 1.0f, 0.0f);	//what is "up" relative to light

//*************
//display stuff
//drawing
GLuint renderingProgram1, renderingProgram2, renderingProgramCubeMap;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];
//timing
double deltaFrameTime = 0.0f;
double lastFrame = 0.0f;
//other variables used in display()
GLuint vLoc, mvLoc, projLoc, nLoc;						
glm::mat4 pMat, vMat, mMat, mvMat, invTrMat, rMat;
GLuint screenWidthLoc, screenHeightLoc;	//for the shadow fragment shader lookup function



//setupVertices
//setupShadowBuffers
//init
//installLights
//updateCamera
//drawGroundPlane
//drawPyramid
//drawTorus
//drawsphere
//passone
//passSkybox
//passtwo
//updatelightlocations
//display
//processinput
//callbacks
//main


void setupVertices(void) {
	//pyramid & ground plane & cube map already declared beforehand; just needs to be drawn here

	
	glGenVertexArrays(1, vao); //create Vertex Array Object from vao, store the id in vao
	glBindVertexArray(vao[0]); //make VAO "active", generated buffers hereafter will be associated with it
	glGenBuffers(numVBOs, vbo); //create Vertex Buffer Objects, store ids in vbo respectively

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid.vertices), pyramid.vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);	
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid.texCoords), pyramid.texCoords, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(groundplane.vertices), groundplane.vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(groundplane.texCoords), groundplane.texCoords, GL_STATIC_DRAW);


	//torus
	std::vector<int> tor_ind = torus.getIndices();
	std::vector<glm::vec3> tor_vert = torus.getVertices();
	std::vector<glm::vec2> tor_tex = torus.getTexCoords();
	std::vector<glm::vec3> tor_norm = torus.getNormals();

	std::vector<float> tor_pvals;
	std::vector<float> tor_tvals;
	std::vector<float> tor_nvals;

	for (int i = 0; i < torus.getNumVertices(); i++) {
		tor_pvals.push_back(tor_vert[i].x);
		tor_pvals.push_back(tor_vert[i].y);
		tor_pvals.push_back(tor_vert[i].z);
		tor_tvals.push_back(tor_tex[i].s);
		tor_tvals.push_back(tor_tex[i].t);
		tor_nvals.push_back(tor_norm[i].x);
		tor_nvals.push_back(tor_norm[i].y);
		tor_nvals.push_back(tor_norm[i].z);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, tor_pvals.size() * 4, &tor_pvals[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, tor_tvals.size() * 4, &tor_tvals[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glBufferData(GL_ARRAY_BUFFER, tor_nvals.size() * 4, &tor_nvals[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[7]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, tor_ind.size() * 4, &tor_ind[0], GL_STATIC_DRAW);


	//cube map
	glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubemap.vertices), cubemap.vertices, GL_STATIC_DRAW);

	
	//sphere
	std::vector<int> sph_ind = sphere.getIndices();
	std::vector<glm::vec3> sph_vert = sphere.getVertices();
	std::vector<glm::vec2> sph_tex = sphere.getTexCoords();
	std::vector<glm::vec3> sph_norm = sphere.getNormals();
	std::vector<float> sph_pvals;
	std::vector<float> sph_tvals;
	std::vector<float> sph_nvals;
	int numIndices = sphere.getNumIndices();
	for (int i = 0; i < numIndices; i++) {
		sph_pvals.push_back((sph_vert[sph_ind[i]]).x);
		sph_pvals.push_back((sph_vert[sph_ind[i]]).y);
		sph_pvals.push_back((sph_vert[sph_ind[i]]).z);
		sph_tvals.push_back((sph_tex[sph_ind[i]]).s);
		sph_tvals.push_back((sph_tex[sph_ind[i]]).t);
		sph_nvals.push_back((sph_norm[sph_ind[i]]).x);
		sph_nvals.push_back((sph_norm[sph_ind[i]]).y);
		sph_nvals.push_back((sph_norm[sph_ind[i]]).z);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);
	glBufferData(GL_ARRAY_BUFFER, sph_pvals.size() * 4, &sph_pvals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[10]);
	glBufferData(GL_ARRAY_BUFFER, sph_tvals.size() * 4, &sph_tvals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[11]);
	glBufferData(GL_ARRAY_BUFFER, sph_nvals.size() * 4, &sph_nvals[0], GL_STATIC_DRAW);


	//spaceship
	//spaceship
	std::vector<glm::vec3> ship_vert = spaceship.getVertices();
	std::vector<glm::vec2> ship_tex = spaceship.getTexCoords();
	std::vector<glm::vec3> ship_norm = spaceship.getNormals();
	std::vector<float> ship_pvals;
	std::vector<float> ship_tvals;
	std::vector<float> ship_nvals;
	for (int i = 0; i < spaceship.getNumVertices(); i++) {
		ship_pvals.push_back((ship_vert[i]).x);
		ship_pvals.push_back((ship_vert[i]).y);
		ship_pvals.push_back((ship_vert[i]).z);
		ship_tvals.push_back((ship_tex[i]).s);
		ship_tvals.push_back((ship_tex[i]).t);
		ship_nvals.push_back((ship_norm[i]).x);
		ship_nvals.push_back((ship_norm[i]).y);
		ship_nvals.push_back((ship_norm[i]).z);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo[12]);
	glBufferData(GL_ARRAY_BUFFER, ship_pvals.size() * 4, &ship_pvals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[13]);
	glBufferData(GL_ARRAY_BUFFER, ship_tvals.size() * 4, &ship_tvals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[14]);
	glBufferData(GL_ARRAY_BUFFER, ship_nvals.size() * 4, &ship_nvals[0], GL_STATIC_DRAW);
}

void setupShadowBuffers(GLFWwindow *window) {
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
	scSizeX = screenWidth;
	scSizeY = screenHeight;

	glGenFramebuffers(1, &shadowBuffer_pa);
	glGenFramebuffers(1, &shadowBuffer_pb);

	glGenTextures(1, &shadowTex_pa);
	glGenTextures(1, &shadowTex_pb);

	glBindTexture(GL_TEXTURE_2D, shadowTex_pa);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
		scSizeX, scSizeY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	//not sure if i need to repeat all of these parameters, but I will for safety!
	glBindTexture(GL_TEXTURE_2D, shadowTex_pb);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
		scSizeX, scSizeY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	// may reduce shadow border artifacts
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void init(GLFWwindow* window) {
	renderingProgram1 = Utils::createShaderProgram("vShader_p1.vert", "fShader_p1.frag");
	renderingProgram2 = Utils::createShaderProgram("vShader_p2_blinnphong.vert", "fShader_p2_blinnphong.frag");
	renderingProgramCubeMap = Utils::createShaderProgram("vShader_cm.vert", "fShader_cm.frag");

	//perspective matrix
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
	aspect = (float)screenWidth / (float)screenWidth;
	pMat = glm::perspective(glm::radians(fov), aspect, 0.1f, 1000.0f);

	//objects
	setupVertices();

	//shadows
	setupShadowBuffers(window);
	//b matrix (translates shadow texture to view matrix)
	b = glm::mat4(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f);

	//texture loading & settings
	diamTexture = Utils::loadTexture("image.jpg");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	grassTexture = Utils::loadTexture("grass.jpg");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	cobbleTexture = Utils::loadTexture("cobblestone.jpg");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	classyTexture = Utils::loadTexture("amazing_art.png");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	spaceshipTexture = Utils::loadTexture("spstob_1.jpg");

	skyboxTexture = Utils::loadCubeMap("cubeMap");	//expects a folder name
}

//todo change to gluint maybe
void installLights(int renderingProgram, glm::mat4 vMatrix) {
	if (lightIsOn_pa) {
		transformed_pa = glm::vec3(vMatrix * glm::vec4(currentLightPos_pa, 1.0));
		lightPos_pa[0] = transformed_pa.x;
		lightPos_pa[1] = transformed_pa.y;
		lightPos_pa[2] = transformed_pa.z;
	}
	
	if (lightIsOn_pb) {
		transformed_pb = glm::vec3(vMatrix * glm::vec4(currentLightPos_pb, 1.0));
		lightPos_pb[0] = transformed_pb.x;
		lightPos_pb[1] = transformed_pb.y;
		lightPos_pb[2] = transformed_pb.z;
	}

	// get the locations of the light and material fields in the shader
	globalAmbLoc = glGetUniformLocation(renderingProgram, "globalAmbient");

	//light a
	if (lightIsOn_pa) {
		ambLoc_pa = glGetUniformLocation(renderingProgram, "light_pa.ambient");
		diffLoc_pa = glGetUniformLocation(renderingProgram, "light_pa.diffuse");
		specLoc_pa = glGetUniformLocation(renderingProgram, "light_pa.specular");
		posLoc_pa = glGetUniformLocation(renderingProgram, "light_pa.position");
	}
	//power_pa = glGetUniformLocation(renderingProgram, "light_paPower");

	//light b
	if (lightIsOn_pb) {
		ambLoc_pb = glGetUniformLocation(renderingProgram, "light_pb.ambient");
		diffLoc_pb = glGetUniformLocation(renderingProgram, "light_pb.diffuse");
		specLoc_pb = glGetUniformLocation(renderingProgram, "light_pb.specular");
		posLoc_pb = glGetUniformLocation(renderingProgram, "light_pb.position");
	}

	//material
	mambLoc = glGetUniformLocation(renderingProgram, "material.ambient");
	mdifLoc = glGetUniformLocation(renderingProgram, "material.diffuse");
	mspeLoc = glGetUniformLocation(renderingProgram, "material.specular");
	mshiLoc = glGetUniformLocation(renderingProgram, "material.shininess");

	//set the uniform light and material values in the shader
	//global
	glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, globalAmbient);

	//light A
	if (lightIsOn_pa) {
		glProgramUniform4fv(renderingProgram, ambLoc_pa, 1, lightAmbient_pa);
		glProgramUniform4fv(renderingProgram, diffLoc_pa, 1, lightDiffuse_pa);
		glProgramUniform4fv(renderingProgram, specLoc_pa, 1, lightSpecular_pa);
		glProgramUniform3fv(renderingProgram, posLoc_pa, 1, lightPos_pa);
	}
	//glProgramUniform1f(renderingProgram, power_pa, lightAPower);

	//light B
	if (lightIsOn_pb) {
		glProgramUniform4fv(renderingProgram, ambLoc_pb, 1, lightAmbient_pb);
		glProgramUniform4fv(renderingProgram, diffLoc_pb, 1, lightDiffuse_pb);
		glProgramUniform4fv(renderingProgram, specLoc_pb, 1, lightSpecular_pb);
		glProgramUniform3fv(renderingProgram, posLoc_pb, 1, lightPos_pb);
	}

	//materials
	//not sure what precedence this needs, so passing these values first
	glProgramUniform4fv(renderingProgram2, mambLoc, 1, thisAmb);
	glProgramUniform4fv(renderingProgram2, mdifLoc, 1, thisDif);
	glProgramUniform4fv(renderingProgram2, mspeLoc, 1, thisSpe);
	glProgramUniform1f(renderingProgram2, mshiLoc, thisShi);
}

void updateCamera() {
	//lookat: 1st vec is cam position, 2nd vec is target position, 3rd vec is "up"
	vMat = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}
void updateLightLocations(double currentTime) {
	lightLoc_pb.y += 0.01f;

	currentLightPos_pa = lightLoc_pa;
	currentLightPos_pb = lightLoc_pb;
}





void drawGroundPlane_p1(GLuint sLoc, glm::mat4 &shadowMVP, const glm::mat4 &lightPMat, const glm::mat4 &lightVMat) {
	mMat = glm::translate(glm::mat4(1.0f), groundplane.pos);

	shadowMVP = lightPMat * lightVMat * mMat;
	sLoc = glGetUniformLocation(renderingProgram1, "shadow_mvpmatrix");
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}
void drawGroundPlane_p2() {
	//set reflectivity/texture
	thisAmb[0] = gpMatAmb[0]; thisAmb[1] = gpMatAmb[1]; thisAmb[2] = gpMatAmb[2]; thisAmb[3] = gpMatAmb[3];
	thisDif[0] = gpMatDif[0]; thisDif[1] = gpMatDif[1]; thisDif[2] = gpMatDif[2]; thisDif[3] = gpMatDif[3];
	thisSpe[0] = gpMatSpe[0]; thisSpe[1] = gpMatSpe[1]; thisSpe[2] = gpMatSpe[2]; thisSpe[3] = gpMatSpe[3];
	thisShi = gpMatShi;
	glUniform1i(matType, 1);	//matType tells precedence of texture vs. material properties
	installLights(renderingProgram2, vMat);

	//model-view matrix
	mMat = glm::translate(glm::mat4(1.0f), groundplane.pos);
	mvMat = vMat * mMat;
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	//pass vertices through
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	//pass texture buffer through
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, grassTexture);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}





void drawPyramid_p1(GLuint sLoc, glm::mat4& shadowMVP, const glm::mat4& lightPMat, const glm::mat4& lightVMat) {
	mMat = glm::translate(glm::mat4(1.0f), pyramid.pos);

	shadowMVP = lightPMat * lightVMat * mMat;
	sLoc = glGetUniformLocation(renderingProgram1, "shadow_mvpmatrix");
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 18);
}
//todo mess with setting cull face with just 3ds
void drawPyramid_p2() {
	//set reflectivity/texture
	thisAmb[0] = pyrMatAmb[0]; thisAmb[1] = pyrMatAmb[1]; thisAmb[2] = pyrMatAmb[2]; thisAmb[3] = pyrMatAmb[3];
	thisDif[0] = pyrMatDif[0]; thisDif[1] = pyrMatDif[1]; thisDif[2] = pyrMatDif[2]; thisDif[3] = pyrMatDif[3];
	thisSpe[0] = pyrMatSpe[0]; thisSpe[1] = pyrMatSpe[1]; thisSpe[2] = pyrMatSpe[2]; thisSpe[3] = pyrMatSpe[3];
	thisShi = pyrMatShi;
	glUniform1i(matType, 2);	//matType tells precedence of texture vs. material properties
	installLights(renderingProgram2, vMat);

	//model-view matrix
	mMat = glm::translate(glm::mat4(1.0f), pyramid.pos);
	mvMat = vMat * mMat;
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

	//pass vertices through
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	//pass texture buffer through
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diamTexture);


	glDrawArrays(GL_TRIANGLES, 0, 18);
}




void drawTorus_p1(GLuint sLoc, glm::mat4& shadowMVP, const glm::mat4& lightPMat, const glm::mat4& lightVMat) {
	rotAmt_torus += 0.01f;
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(torusPos));
	mMat = glm::rotate(mMat, rotAmt_torus, glm::vec3(1.0f, 0.0f, 0.0f));

	shadowMVP = lightPMat * lightVMat * mMat;
	sLoc = glGetUniformLocation(renderingProgram1, "shadow_mvpmatrix");
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[7]);
	glDrawElements(GL_TRIANGLES, torus.getNumIndices(), GL_UNSIGNED_INT, 0);
}
void drawTorus_p2() {
	glUniform1i(matType, 3);	//matType tells precedence of texture vs. material properties
	//set reflectivity/texture 
	thisAmb[0] = torusMatAmb[0]; thisAmb[1] = torusMatAmb[1]; thisAmb[2] = torusMatAmb[2]; thisAmb[3] = torusMatAmb[3];
	thisDif[0] = torusMatDif[0]; thisDif[1] = torusMatDif[1]; thisDif[2] = torusMatDif[2]; thisDif[3] = torusMatDif[3];
	thisSpe[0] = torusMatSpe[0]; thisSpe[1] = torusMatSpe[1]; thisSpe[2] = torusMatSpe[2]; thisSpe[3] = torusMatSpe[3];
	thisShi = torusMatShi;
	

	installLights(renderingProgram2, vMat);

	//model-view matrix, normals
	//rotAmt_torus += 0.01f; executes in pass one
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(torusPos));
	mMat = glm::rotate(mMat, rotAmt_torus, glm::vec3(1.0f, 0.0f, 0.0f));

	mvMat = vMat * mMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);	//positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);	//textures
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cobbleTexture);


	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);


	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);	//normals
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[7]);	//indices
	glDrawElements(GL_TRIANGLES, torus.getNumIndices(), GL_UNSIGNED_INT, 0);
}




void drawSphere_p1(GLuint sLoc, glm::mat4& shadowMVP, const glm::mat4& lightPMat, const glm::mat4& lightVMat) {
	//model-view matrix, normals
	rotAmt_sphere += 0.01f;
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(spherePos));
	mMat = glm::translate(mMat,
		glm::vec3(sin(rotAmt_sphere * 10.0), 0.0f, cos(rotAmt_sphere * 6.0))); 
	mMat = glm::rotate(mMat, rotAmt_sphere, glm::vec3(1.0f, 0.0f, 0.0f));

	shadowMVP = lightPMat * lightVMat * mMat;
	sLoc = glGetUniformLocation(renderingProgram1, "shadow_mvpmatrix");
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, sphere.getNumIndices());
}
void drawSphere_p2() {
	glUniform1i(matType, 1);	//matType tells precedence of texture vs. material properties
	//set reflectivity/texture 
	thisAmb[0] = sphereMatAmb[0]; thisAmb[1] = sphereMatAmb[1]; thisAmb[2] = sphereMatAmb[2]; thisAmb[3] = sphereMatAmb[3];
	thisDif[0] = sphereMatDif[0]; thisDif[1] = sphereMatDif[1]; thisDif[2] = sphereMatDif[2]; thisDif[3] = sphereMatDif[3];
	thisSpe[0] = sphereMatSpe[0]; thisSpe[1] = sphereMatSpe[1]; thisSpe[2] = sphereMatSpe[2]; thisSpe[3] = sphereMatSpe[3];
	thisShi = sphereMatShi;


	installLights(renderingProgram2, vMat);

	//model-view matrix, normals
	//rotAmt_sphere += 0.02f; executes in pass 1

	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(spherePos));
	mMat = glm::translate(mMat,
		glm::vec3(10 * sin(rotAmt_sphere), 0.0f, 10 * cos(rotAmt_sphere)));
	mMat = glm::rotate(mMat, rotAmt_sphere, glm::vec3(1.0f, 0.0f, 0.0f));

	mvMat = vMat * mMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);	//positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[10]);	//textures
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, classyTexture);


	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);


	glBindBuffer(GL_ARRAY_BUFFER, vbo[11]);	//normals
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);


	glDrawArrays(GL_TRIANGLES, 0, sphere.getNumIndices());
}

void drawSpaceship_p1(GLuint sLoc, glm::mat4& shadowMVP, const glm::mat4& lightPMat, const glm::mat4& lightVMat) {
	rotAmt_ship += 0.01f;
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(spherePos.x + 2.0f, spherePos.y, spherePos.z + 2.0f));
	mMat = glm::translate(mMat,
		glm::vec3(2 * cos(rotAmt_ship), 0.0f, 2 * sin(rotAmt_ship)));

	shadowMVP = lightPMat * lightVMat * mMat;
	sLoc = glGetUniformLocation(renderingProgram1, "shadow_mvpmatrix");
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[12]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, spaceship.getNumVertices());
}


void drawSpaceship_p2() {
	glUniform1i(matType, 0);	//matType tells precedence of texture vs. material properties
	//set reflectivity/texture 
	thisAmb[0] = shipMatAmb[0]; thisAmb[1] = shipMatAmb[1]; thisAmb[2] = shipMatAmb[2]; thisAmb[3] = shipMatAmb[3];
	thisDif[0] = shipMatDif[0]; thisDif[1] = shipMatDif[1]; thisDif[2] = shipMatDif[2]; thisDif[3] = shipMatDif[3];
	thisSpe[0] = shipMatSpe[0]; thisSpe[1] = shipMatSpe[1]; thisSpe[2] = shipMatSpe[2]; thisSpe[3] = shipMatSpe[3];
	thisShi = shipMatShi;


	installLights(renderingProgram2, vMat);

	//model-view matrix, normals
	//rotAmt += 0.01f; executes in pass 1
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(spherePos.x + 2.0f, spherePos.y, spherePos.z + 2.0f));
	mMat = glm::translate(mMat,
		glm::vec3(10 * cos(rotAmt_ship), 0.0f, 10 * sin(rotAmt_ship)));

	mvMat = vMat * mMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[12]);	//positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[13]);	//textures
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, spaceshipTexture);

	/*
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	*/


	glBindBuffer(GL_ARRAY_BUFFER, vbo[14]);	//normals
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);


	glDrawArrays(GL_TRIANGLES, 0, spaceship.getNumVertices());

}



//handle for each light, hence multiple passOne defs
//*****************
	//positional light a
void passOne_pa() {
	glUseProgram(renderingProgram1);
	
	
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer_pa);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTex_pa, 0);

	glDrawBuffer(GL_NONE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL);	// for reducing
	glPolygonOffset(2.0f, 4.0f);		//  shadow artifacts

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	//GLuint sLoc, glm::mat4 &shadowMVP, const glm::mat4 &lightPMat, const glm::mat4 &lightVMat
	
	drawTorus_p1(sLoc_pa, shadowMVP_pa, lightPMat_pa, lightVMat_pa);
	drawPyramid_p1(sLoc_pa, shadowMVP_pa, lightPMat_pa, lightVMat_pa);
	drawSphere_p1(sLoc_pa, shadowMVP_pa, lightPMat_pa, lightVMat_pa);
	drawSpaceship_p1(sLoc_pa, shadowMVP_pa, lightPMat_pa, lightVMat_pa);
	drawGroundPlane_p1(sLoc_pa, shadowMVP_pa, lightPMat_pa, lightVMat_pa);

}
//*****************
	//positional light b
void passOne_pb() {
	glUseProgram(renderingProgram1);

	
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer_pb);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTex_pb, 0);

	glDrawBuffer(GL_NONE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL);	// for reducing
	glPolygonOffset(2.0f, 4.0f);		//  shadow artifacts

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	//GLuint sLoc, glm::mat4 &shadowMVP, const glm::mat4 &lightPMat, const glm::mat4 &lightVMat

	drawTorus_p1(sLoc_pb, shadowMVP_pb, lightPMat_pb, lightVMat_pb);
	drawPyramid_p1(sLoc_pb, shadowMVP_pb, lightPMat_pb, lightVMat_pb);
	drawSphere_p1(sLoc_pb, shadowMVP_pb, lightPMat_pb, lightVMat_pb);
	drawSpaceship_p1(sLoc_pb, shadowMVP_pb, lightPMat_pb, lightVMat_pb);
	drawGroundPlane_p1(sLoc_pb, shadowMVP_pb, lightPMat_pb, lightVMat_pb);
}
//*****************
	//spotlight
void passOne_sptlt() {


	
}

void drawSkybox() {
	glUseProgram(renderingProgramCubeMap);

	vLoc = glGetUniformLocation(renderingProgramCubeMap, "v_matrix");
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));

	projLoc = glGetUniformLocation(renderingProgramCubeMap, "p_matrix");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

	glFrontFace(GL_CCW);	// cube is CW, but we are viewing the inside
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glEnable(GL_DEPTH_TEST);
}

void passTwo() {
	glUseProgram(renderingProgram2);

	//get uniform variables for the MV and projection matrices
	mvLoc = glGetUniformLocation(renderingProgram2, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram2, "proj_matrix");
	nLoc = glGetUniformLocation(renderingProgram2, "norm_matrix");
	screenWidthLoc = glGetUniformLocation(renderingProgram2, "screenW");
	screenHeightLoc = glGetUniformLocation(renderingProgram2, "screenH");
	matType = glGetUniformLocation(renderingProgram2, "matType");

	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniform1d(screenWidthLoc, (double)screenWidth);
	glUniform1d(screenHeightLoc, (double)screenHeight);


	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	
	drawGroundPlane_p2();	//might have to do something with thisAmb and bMatAmb etc
	drawPyramid_p2();
	drawSphere_p2();
	drawSpaceship_p2();
	drawTorus_p2();
}

void display(GLFWwindow *window, double currentTime) {
	deltaFrameTime = currentTime - lastFrame;
	lastFrame = currentTime;

	//glUseProgram(renderingProgram2);

	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);

	updateLightLocations(currentTime);
	updateCamera();

	

	passOne_pa();

	glDisable(GL_POLYGON_OFFSET_FILL);	// artifact reduction, continued

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE1);	//goes to layout binding = 1
	glBindTexture(GL_TEXTURE_2D, shadowTex_pa);

	glDrawBuffer(GL_FRONT);

	passOne_pb();

	glDisable(GL_POLYGON_OFFSET_FILL);	// artifact reduction, continued

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE2);	//goes to layout binding = 2
	glBindTexture(GL_TEXTURE_2D, shadowTex_pb);

	glDrawBuffer(GL_FRONT);

	drawSkybox();

	//build view matrix, model matrices, model-view matrices
	passTwo();
	
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && !hasBeenToggled) {
		hasBeenToggled = true;
		if (isSprinting) {
			isSprinting = false;
		}
		else {
			isSprinting = true;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS) {
		hasBeenToggled = false;
	}

	if (isSprinting) {
		actualCamSpd = cameraSpd * deltaFrameTime * 2;
	}
	else {
		actualCamSpd = cameraSpd * deltaFrameTime;
	}


	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cameraPos += actualCamSpd * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		//cameraFront and cameraUp form a plane; their cross product is the perpendicular vector which will run right/left
		cameraPos -= actualCamSpd * glm::normalize(glm::cross(cameraFront, cameraUp));
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cameraPos -= actualCamSpd * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cameraPos += actualCamSpd * glm::normalize(glm::cross(cameraFront, cameraUp));
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		cameraPos += actualCamSpd * glm::vec3(0.0f, 1.0f, 0.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		cameraPos -= actualCamSpd * glm::vec3(0.0f, 1.0f, 0.0f);
	}
}

void window_size_callback(GLFWwindow* window, int newWidth, int newHeight) {
	aspect = (float)newWidth / (float)newHeight;
	screenWidth = newWidth;
	screenHeight = newHeight;
	glViewport(0, 0, newWidth, newHeight);
	pMat = glm::perspective(glm::radians(fov), aspect, 0.1f, 1000.0f);
	setupShadowBuffers(window);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;	//y coordinates are reversed
	lastX = xpos;
	lastY = ypos;

	xoffset *= mouseSensitivity;	//sensitivity set at the top
	yoffset *= mouseSensitivity;

	yaw += xoffset;
	pitch += yoffset;

	//make sure pitch doesn't flip screen at edge of bounds
	if (pitch > 89.0f) {
		pitch = 89.0f;
	} 
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	cameraFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront.y = sin(glm::radians(pitch));
	cameraFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(cameraFront);	//always set camera vector to unit vector
}



//****
//MAIN
//****
int main(void) {
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "final project - PRESS ESC TO CLOSE", NULL, NULL);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		exit(EXIT_FAILURE);
	}

	glfwSwapInterval(1);

	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	//captures mouse for our window without displaying the cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	init(window);

	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}