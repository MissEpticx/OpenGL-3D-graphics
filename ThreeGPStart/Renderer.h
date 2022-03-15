#pragma once

#include "ExternalLibraryHeaders.h"
#include "Helper.h"
#include "Mesh.h"
#include "Camera.h"

//Creates a struct to hold specific information
struct Mesh
{
	GLuint txtr;
	GLuint vao{ 0 };
	GLuint numElements{ 0 };
};

struct Model 
{
	std::string modelName;
	std::vector<Mesh> meshVector;
	GLuint numCubeElements = 0;
};

class Renderer
{
private:
	// Program object - to host shaders
	GLuint m_skyProgram{ 0 };
	GLuint m_cubeProgram{ 0 };
	GLuint m_program{ 0 };

	//Create a model vector
	std::vector<Model> modelVector;

	bool m_wireframe{ false };

	//Create a function that allows me to create a program
	GLuint CreateProgram(std::string, std::string);
public:
	Renderer();
	~Renderer();

	// Draw GUI
	void DefineGUI();

	// Create and / or load geometry, this is like 'level load'
	bool InitialiseGeometry();

	// Render the scene
	void Render(const Helpers::Camera& camera, float deltaTime);
}; 