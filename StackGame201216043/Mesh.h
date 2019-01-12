#pragma once

#include <GL\glew.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

class Mesh
{
public:
	Mesh();
	

	enum Block_Color {
		RED,
		GREEN,
		BLUE,
		RAINBOW
	};

	void CreateMesh(GLfloat *vertices, unsigned int *indices, unsigned int numOfVertices, unsigned int numOfIndices);
	void RenderMesh();
	void ClearMesh();

	void SetColor(Block_Color color);
	glm::vec3 GetColor();

	~Mesh();

private:
	GLuint VAO, VBO, IBO;
	GLsizei indexCount;
	glm::vec3 blockColor;
};

