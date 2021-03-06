#include "stdafx.h"
#include "string.h"
#include <iostream>
#include <cmath>
#include <vector> //array lists

#include <GL\glew.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <gl\GLU.h>

#include<glm\glm.hpp>
#include<glm\gtc\matrix_transform.hpp>
#include<glm\gtc\type_ptr.hpp>

#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"



bool init();
bool initGL();
void render();
void close();
void CreateShaders();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//OpenGL context
SDL_GLContext gContext;

std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

bool direction = true;
float triOffset = 0.0f;
float triMaxOffset = 5.0f;
float triIncrement = 0.08f;

float triSetStill = 0.0f;
bool cubeShouldMove = true;
bool shouldSpawnNewCube = false;

float cubePositions[20] = {};

float width = 800.0;
float height = 600.0;

int cubeNumber = 1;
float prevXPosition = 0.0f;


//Vertex Shader
static const char* vShader = "Shaders/shader.vert";

// Fragment shader
static const char* fShader = "Shaders/shader.frag";

void CreateObjects();

// camera
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f), glm::vec3(0.0f,1.0f,0.0f), -90.0f, 0.0f,100.0f);
float lastX = -1;
float lastY = -1;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//event handlers
void HandleKeyUp(const SDL_KeyboardEvent& key);

int main(int argc, char* args[])
{
	init();

	SDL_Event e;
	//While application is running
	bool quit = false;
	CreateObjects();
	CreateShaders();
	while (!quit)
	{
		// per-frame time logic
		// -------------------
		float currentFrame = SDL_GetTicks() / 1000.0f;
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		if (direction)
		{
			triOffset += triIncrement;
		}
		else {
			triOffset -= triIncrement;
		}

		if (abs(triOffset) >= triMaxOffset) {
			direction = !direction;
		}

		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{

			//User requests quit
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			switch (e.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				if (e.key.keysym.sym == SDLK_ESCAPE)
				{
					quit = true;
				}
				else {
					HandleKeyUp(e.key);
				}
				break;

			}

			
		}

		if (cubeShouldMove) {
			cubePositions[cubeNumber] = triOffset;
		}
		
		//Render quad
		render();

		//Update screen
		SDL_GL_SwapWindow(gWindow);

	}

	close();

	return 0;
}

void HandleKeyUp(const SDL_KeyboardEvent& key)
{
	switch (key.keysym.sym)
	{
	case SDLK_w:
		camera.ProcessKeyboard(camera.FORWARD, deltaTime);
		break;
	case SDLK_s:
		camera.ProcessKeyboard(camera.BACKWARD, deltaTime);
		break;
	case SDLK_a:
		camera.ProcessKeyboard(camera.LEFT, deltaTime);
		break;
	case SDLK_d:
		camera.ProcessKeyboard(camera.RIGHT, deltaTime);
		break;
	case SDLK_q:
		camera.ProcessKeyboard(camera.UP, deltaTime);
		break;
	case SDLK_e:
		camera.ProcessKeyboard(camera.DOWN, deltaTime);
		break;
	case SDLK_SPACE:
		if (cubeNumber <= 21) {
			if (cubePositions[cubeNumber] > cubePositions[cubeNumber - 1] + 2.0 || cubePositions[cubeNumber] < cubePositions[cubeNumber - 1] - 2.0) {
				printf("gubish");
				cubeShouldMove = false;
			}
			else {
				cubeNumber++;
				triIncrement += 0.03;
				camera.MoveUp();

			}
		}
		
		break;
	}
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Use OpenGL 3.3
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);


		//Create window
		gWindow = SDL_CreateWindow("Stack Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN*/);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create context
			gContext = SDL_GL_CreateContext(gWindow);
			if (gContext == NULL)
			{
				printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Use Vsync
				if (SDL_GL_SetSwapInterval(1) < 0)
				{
					printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
				}

				//Initialize OpenGL
				if (!initGL())
				{
					printf("Unable to initialize OpenGL!\n");
					success = false;
				}
			}
		}
	}

	return success;
}

bool initGL()
{
	bool success = true;
	GLenum error = GL_NO_ERROR;

	glewInit();

	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		success = false;
		printf("Error initializing OpenGL! %s\n", gluErrorString(error));
	}


	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	glEnable(GL_DEPTH_TEST);

	/*glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/


	return success;
}

void close()
{

	//Delete OGL context
	SDL_GL_DeleteContext(gContext);
	//Destroy window	
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

void render()
{
	
	int cubeCounter = 1;

	//Clear color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLuint uniformProjection = 0, uniformModel = 0, uniformView, blockColorLocation;
	

	shaderList[0].UseShader();
	uniformModel = shaderList[0].GetModelLocation();
	uniformProjection = shaderList[0].GetProjectionLocation();
	uniformView = shaderList[0].GetViewLocation();
	blockColorLocation = glGetUniformLocation(shaderList[0].GetShaderID(), "blockColor");

	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));

	//Floor------------------------------------------------------------------------
	glm::mat4 floor = glm::mat4(1.0);
	floor = glm::translate(floor, glm::vec3(0.0f, -2.4f, -7.5f));
	floor = glm::scale(floor, glm::vec3(8.0f, 0.2f, 5.0f));
	//model = glm::rotate(model, glm::radians(currAngle), glm::vec3(0, 1, 0));

	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(floor));
	glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[0]->GetColor()));
	
	if (!meshList.empty()) {
		meshList[0]->RenderMesh();
	}

	//Back wall------------------------------------------------------------------------
	glm::mat4 backWall = glm::mat4(1.0);
	backWall = glm::translate(backWall, glm::vec3(0.0f, 17.0f, -14.5f));
	backWall = glm::scale(backWall, glm::vec3(9.0f, 20.0f, 0.5f));

	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(backWall));
	glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[0]->GetColor()));
	meshList[0]->RenderMesh();

	//First Cube
	glm::mat4 firstQube = glm::mat4(1.0);
	firstQube = glm::translate(firstQube, glm::vec3(0.0f, -1.0f, -7.5f));
	firstQube = glm::scale(firstQube, glm::vec3(1.0f, 1.0f, 1.0f));

	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(firstQube));
	glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[1]->GetColor()));
	meshList[1]->RenderMesh();



	glm::mat4 spawnCube = glm::mat4(1.0);
	
	spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 1.0f, -7.5f));
	spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
	glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[2]->GetColor()));
	meshList[2]->RenderMesh();

	//glUniform1f(uniformModel,triOffset);

	//second spawn
	if (cubeNumber > 1) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 3.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[1]->GetColor()));
		meshList[1]->RenderMesh();
	}
	//third spawn
	if (cubeNumber > 2) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 5.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[2]->GetColor()));
		meshList[2]->RenderMesh();
	}

	//forth spawn
	if (cubeNumber > 3) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 7.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[1]->GetColor()));
		meshList[1]->RenderMesh();
	}

	//fifth spawn cube
	if (cubeNumber > 4) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 9.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[2]->GetColor()));
		meshList[2]->RenderMesh();
	}

	//sixth spawn cube
	if (cubeNumber > 5) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 11.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[1]->GetColor()));
		meshList[1]->RenderMesh();
	}

	//7th spawn cube
	if (cubeNumber > 6) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 13.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[2]->GetColor()));
		meshList[2]->RenderMesh();
	}

	//8th spawn cube
	if (cubeNumber > 7) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 15.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[1]->GetColor()));
		meshList[1]->RenderMesh();
	}

	//9th spawn cube
	if (cubeNumber > 8) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 17.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[2]->GetColor()));
		meshList[2]->RenderMesh();
	}

	//10th spawn cube
	if (cubeNumber > 9) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 19.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[1]->GetColor()));
		meshList[1]->RenderMesh();
	}

	//11th spawn cube
	if (cubeNumber > 10) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 21.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[2]->GetColor()));
		meshList[2]->RenderMesh();
	}

	//12th spawn cube
	if (cubeNumber > 11) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 23.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[1]->GetColor()));
		meshList[1]->RenderMesh();
	}

	//13th spawn cube
	if (cubeNumber > 12) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 25.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[2]->GetColor()));
		meshList[2]->RenderMesh();
	}

	//14th spawn cube
	if (cubeNumber > 13) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 27.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[1]->GetColor()));
		meshList[1]->RenderMesh();
	}

	//15th spawn cube
	if (cubeNumber > 14) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 29.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[2]->GetColor()));
		meshList[2]->RenderMesh();
	}

	//16th spawn cube
	if (cubeNumber > 15) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 31.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[1]->GetColor()));
		meshList[1]->RenderMesh();
	}

	//17th spawn cube
	if (cubeNumber > 16) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 33.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[2]->GetColor()));
		meshList[2]->RenderMesh();
	}

	//18th spawn cube
	if (cubeNumber > 17) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 35.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[1]->GetColor()));
		meshList[1]->RenderMesh();
	}

	//19th spawn cube
	if (cubeNumber > 18) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 37.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[2]->GetColor()));
		meshList[2]->RenderMesh();
	}

	//20th spawn cube
	if (cubeNumber > 19) {

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 39.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[1]->GetColor()));
		meshList[1]->RenderMesh();
	}

	//21th spawn cube
	if (cubeNumber > 20) {
		printf("krai");

		cubeCounter++;
		spawnCube = glm::mat4(1.0);
		spawnCube = glm::translate(spawnCube, glm::vec3(cubePositions[cubeCounter], 41.0f, -7.5f));
		spawnCube = glm::scale(spawnCube, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(spawnCube));
		glUniform3fv(blockColorLocation, 1, glm::value_ptr(meshList[2]->GetColor()));
		meshList[2]->RenderMesh();
	}
	
	
	
	

	glUseProgram(0);
	
}

void CreateObjects()
{
	/*unsigned int indices[] = {
		0,3,1,
		1,3,2,
		2,3,0,
		0,1,2
	};
	GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f,
		0.0f,-1.0f,1.0f,
		1.0f,-1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
		*/
	//};

	unsigned int indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3
	};
	GLfloat vertices[] = {
		// front
		-1.0, -1.0,  1.0,
		1.0, -1.0,  1.0,
		1.0,  1.0,  1.0,
		-1.0,  1.0,  1.0,
		// back
		-1.0, -1.0, -1.0,
		1.0, -1.0, -1.0,
		1.0,  1.0, -1.0,
		-1.0,  1.0, -1.0
	};

	//floor and walls
	Mesh *floorWalls = new Mesh();
	floorWalls->CreateMesh(vertices, indices, 36, 12 * 3);
	floorWalls->SetColor(floorWalls->RAINBOW);
	meshList.push_back(floorWalls);

	Mesh *baseCube = new Mesh();
	baseCube->CreateMesh(vertices, indices, 36, 12 * 3);
	baseCube->SetColor(baseCube->RED);
	meshList.push_back(baseCube);

	Mesh *obj3 = new Mesh();
	obj3->CreateMesh(vertices, indices, 36, 12 * 3);
	obj3->SetColor(obj3->GREEN);
	meshList.push_back(obj3);


}

void CreateShaders() {
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}


