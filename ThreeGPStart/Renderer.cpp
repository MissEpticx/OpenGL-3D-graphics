#include "Renderer.h"
#include "Camera.h"
#include "ImageLoader.h"

Renderer::Renderer() 
{

}

// On exit must clean up any OpenGL resources e.g. the program, the buffers
Renderer::~Renderer()
{
	// TODO: clean up any memory used including OpenGL objects via glDelete* calls
	glDeleteProgram(m_cubeProgram);
	glDeleteProgram(m_skyProgram);
	glDeleteProgram(m_program);
}

// Use IMGUI for a simple on screen GUI
void Renderer::DefineGUI()
{
	// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	ImGui::Begin("3GP");						// Create a window called "3GP" and append into it.

	ImGui::Text("Visibility.");					// Display some text (you can use a format strings too)	

	ImGui::Checkbox("Wireframe", &m_wireframe);	// A checkbox linked to a member variable

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		
	ImGui::End();
}

// Load, compile and link the shaders and create a program object to host them
GLuint Renderer::CreateProgram(std::string vsPath, std::string fsPath)
{
	// Create a new program (returns a unqiue id)
	GLuint program = glCreateProgram();

	// Load and create vertex and fragment shaders
	GLuint vertex_shader{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, vsPath) };
	GLuint fragment_shader{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, fsPath) };
	if (vertex_shader == 0 || fragment_shader == 0)
		return false;

	// Attach the vertex shader to this program (copies it)
	glAttachShader(program, vertex_shader);

	// The attibute location 0 maps to the input stream "vertex_position" in the vertex shader
	// Not needed if you use (location=0) in the vertex shader itself
	//glBindAttribLocation(m_program, 0, "vertex_position");

	// Attach the fragment shader (copies it)
	glAttachShader(program, fragment_shader);

	// Done with the originals of these as we have made copies
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	// Link the shaders, checking for errors
	if (!Helpers::LinkProgramShaders(program))
		return 0;

	return program;
}

// Load / create geometry into OpenGL buffers	
bool Renderer::InitialiseGeometry()
{	
//--SKYBOX----------------------------------------------------------------------------------------------------------------//
	//Create a new program to handle specific vertex and fragment shaders
	m_skyProgram = CreateProgram("Data/Shaders/sky_vertex_shader.vert", "Data/Shaders/sky_fragment_shader.frag");

	//Loads in the model required using the file location
	Helpers::ModelLoader loadSkybox;
	if (!loadSkybox.LoadFromFile("Data/Models/Sky/Mars/skybox.X"))
	{
		//Checks for a fail load, cout to determine what load is having issues
		std::cout << "Failed to Load Skybox Model" << std::endl;
		return false;
	}

	//Loads in the texture required using the file location
	Helpers::ImageLoader loadSkyUp;
	if (!loadSkyUp.Load("Data/Models/Sky/Mars/Mar_U.dds"))
	{
		//Loads an error texture in place when original texture fails to load
		if (!loadSkyUp.Load("Data/Textures/ErrorTexture.png"))
		{
			std::cout << "Failed to Load Skybox Texture - Up" << std::endl;
			return false;
		}
	}

	Helpers::ImageLoader loadSkyDown;
	if (!loadSkyDown.Load("Data/Models/Sky/Mars/Mar_D.dds"))
	{
		if (!loadSkyDown.Load("Data/Textures/ErrorTexture.png"))
		{
			std::cout << "Failed to Load Skybox Texture - Down" << std::endl;
			return false;
		}
	}

	Helpers::ImageLoader loadSkyLeft;
	if (!loadSkyLeft.Load("Data/Models/Sky/Mars/Mar_L.dds"))
	{
		if (!loadSkyLeft.Load("Data/Textures/ErrorTexture.png"))
		{
			std::cout << "Failed to Load Skybox Texture - Left" << std::endl;
			return false;
		}
	}

	Helpers::ImageLoader loadSkyRight;
	if (!loadSkyRight.Load("Data/Models/Sky/Mars/Mar_R.dds"))
	{
		if (!loadSkyRight.Load("Data/Textures/ErrorTexture.png"))
		{
			std::cout << "Failed to Load Skybox Texture - Right" << std::endl;
			return false;
		}
	}

	Helpers::ImageLoader loadSkyFront;
	if (!loadSkyFront.Load("Data/Models/Sky/Mars/Mar_F.dds"))
	{
		if (!loadSkyFront.Load("Data/Textures/ErrorTexture.png"))
		{
			std::cout << "Failed to Load Skybox Texture - Front" << std::endl;
			return false;
		}
	}

	Helpers::ImageLoader loadSkyBack;
	if (!loadSkyBack.Load("Data/Models/Sky/Mars/Mar_B.dds"))
	{
		if (!loadSkyBack.Load("Data/Textures/ErrorTexture.png"))
		{
			std::cout << "Failed to Load Skybox Texture - Back" << std::endl;
			return false;
		}
	}

	//Create an instance using a struct and name it (for later use)
	Model Skybox;
	Skybox.modelName = "Skybox";

	//Create a counter to handle each mesh for the skybox model
	int skyMeshTxtr = 0;

	//A loop that goes through each mesh in the skybox
	for (const Helpers::Mesh& meshSky : loadSkybox.GetMeshVector())
	{
		//Create an instance using a struct
		Mesh skyboxMesh;

		//Creates the texture VBO - vertex buffer object
		GLuint skyTxtrVBO;
		glGenBuffers(1, &skyTxtrVBO);
		glBindBuffer(GL_ARRAY_BUFFER, skyTxtrVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * meshSky.uvCoords.size(), meshSky.uvCoords.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//Sets a specific texture based on what counter number each mesh is to have a complete skybox
		if (skyMeshTxtr == 0)
		{
			//Creates the texture using a pre-loaded texture and applies it to the mesh
			glGenTextures(1, &skyboxMesh.txtr);
			glBindTexture(GL_TEXTURE_2D, skyboxMesh.txtr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, loadSkyDown.Width(), loadSkyDown.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, loadSkyDown.GetData());
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else if (skyMeshTxtr == 1)
		{
			glGenTextures(1, &skyboxMesh.txtr);
			glBindTexture(GL_TEXTURE_2D, skyboxMesh.txtr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, loadSkyRight.Width(), loadSkyRight.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, loadSkyRight.GetData());
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else if (skyMeshTxtr == 2)
		{
			glGenTextures(1, &skyboxMesh.txtr);
			glBindTexture(GL_TEXTURE_2D, skyboxMesh.txtr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, loadSkyFront.Width(), loadSkyFront.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, loadSkyFront.GetData());
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else if (skyMeshTxtr == 3)
		{
			glGenTextures(1, &skyboxMesh.txtr);
			glBindTexture(GL_TEXTURE_2D, skyboxMesh.txtr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, loadSkyUp.Width(), loadSkyUp.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, loadSkyUp.GetData());
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else if (skyMeshTxtr == 4)
		{
			glGenTextures(1, &skyboxMesh.txtr);
			glBindTexture(GL_TEXTURE_2D, skyboxMesh.txtr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, loadSkyLeft.Width(), loadSkyLeft.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, loadSkyLeft.GetData());
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else if (skyMeshTxtr == 5)
		{
			glGenTextures(1, &skyboxMesh.txtr);
			glBindTexture(GL_TEXTURE_2D, skyboxMesh.txtr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, loadSkyBack.Width(), loadSkyBack.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, loadSkyBack.GetData());
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		//Adds 1 to my mesh counter
		skyMeshTxtr++;

		//Creates the normals VBO - vertex buffer object
		GLuint skyNormVBO;
		glGenBuffers(1, &skyNormVBO);
		glBindBuffer(GL_ARRAY_BUFFER, skyNormVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * meshSky.normals.size(), meshSky.normals.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		////Creates the position VBO - vertex buffer object
		GLuint skyPosVBO;
		glGenBuffers(1, &skyPosVBO);
		glBindBuffer(GL_ARRAY_BUFFER, skyPosVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * meshSky.vertices.size(), meshSky.vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		////Creates the Elements EBO - Element Buffer Object
		GLuint skyElemEBO;
		glGenBuffers(1, &skyElemEBO);
		glBindBuffer(GL_ARRAY_BUFFER, skyElemEBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * meshSky.elements.size(), meshSky.elements.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//Sets the number of elements using the mesh size
		skyboxMesh.numElements = meshSky.elements.size();

		//sets the VAO to wrap everything together to allow it to render
		glGenVertexArrays(1, &skyboxMesh.vao);
		glBindVertexArray(skyboxMesh.vao);

		glBindBuffer(GL_ARRAY_BUFFER, skyPosVBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, skyNormVBO);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, skyTxtrVBO);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyElemEBO);

		//Always reset the bind to prevent issues
		glBindVertexArray(0);

		//Pushes the mesh into the mesh vector
		Skybox.meshVector.push_back(skyboxMesh);
	}
	//Pushes the model into the model vector, which now also holds the mesh vector data
	modelVector.push_back(Skybox);

//--CUBE------------------------------------------------------------------------------------------------------------------//
	m_cubeProgram = CreateProgram("Data/Shaders/cube_vertex_shader.vert", "Data/Shaders/cube_fragment_shader.frag");

	Model Cube;
	Cube.modelName = "Cube";
	Mesh CubeMesh;

	//Creating all the necessary vertices to create a 6-sided cube
	glm::vec3 cubeCorners[24] =
	{
		// X,   Y,   Z - Front
		{-10, -10,  10},  //0
		{ 10, -10,  10},  //1
		{-10,  10,  10},  //2
		{ 10,  10,  10},  //3
		// X,   Y,   Z - Back
		{-10, -10, -10},  //4
		{ 10, -10, -10},  //5
		{-10,  10, -10},  //6
		{ 10,  10, -10},  //7
		// X,   Y,   Z - Left
		{-10, -10,  10},  //8
		{-10,  10,  10},  //9
		{-10, -10, -10},  //10
		{-10,  10, -10},  //11
		// X,   Y,   Z - Right
		{ 10, -10,  10},  //12
		{ 10,  10,  10},  //13
		{ 10, -10, -10},  //14
		{ 10,  10, -10},  //15
		// X,   Y,   Z - Top
		{-10,  10,  10},  //16
		{ 10,  10,  10},  //17
		{-10,  10, -10},  //18
		{ 10,  10, -10},  //19
		// X,   Y,   Z - Bottom
		{-10, -10,  10},  //20
		{ 10, -10,  10},  //21
		{-10, -10, -10},  //22
		{ 10, -10, -10}   //23
	};

	//Storing all 24 vertices into a vector
	std::vector<glm::vec3> vertices;

	vertices.push_back(cubeCorners[0]);
	vertices.push_back(cubeCorners[1]);
	vertices.push_back(cubeCorners[2]);
	vertices.push_back(cubeCorners[3]);

	vertices.push_back(cubeCorners[4]);
	vertices.push_back(cubeCorners[5]);
	vertices.push_back(cubeCorners[6]);
	vertices.push_back(cubeCorners[7]);

	vertices.push_back(cubeCorners[8]);
	vertices.push_back(cubeCorners[9]);
	vertices.push_back(cubeCorners[10]);
	vertices.push_back(cubeCorners[11]);

	vertices.push_back(cubeCorners[12]);
	vertices.push_back(cubeCorners[13]);
	vertices.push_back(cubeCorners[14]);
	vertices.push_back(cubeCorners[15]);

	vertices.push_back(cubeCorners[16]);
	vertices.push_back(cubeCorners[17]);
	vertices.push_back(cubeCorners[18]);
	vertices.push_back(cubeCorners[19]);

	vertices.push_back(cubeCorners[20]);
	vertices.push_back(cubeCorners[21]);
	vertices.push_back(cubeCorners[22]);
	vertices.push_back(cubeCorners[23]);

	//Creating all the necessary colours for each vert
	std::vector<GLfloat> cubeColours =
	{
		//Front colours
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		//Back colours
		1.0f, 0.5f, 0.0f,
		1.0f, 0.5f, 0.0f,
		1.0f, 0.5f, 0.0f,
		1.0f, 0.5f, 0.0f,
		//Left colours
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		//Right colours
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		//Top colours
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		//Bottom colours
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};

	/*Storing the vertices into sets of 3 to create triangles
	Ensuring that the order in which the vertices are stored is correct 
	(anti-clockwise or clockwise depending on which way the face looking out)
	2 sets of triangles are required for each square face*/
	std::vector<GLuint> cubeElements;
	//Front
	cubeElements.push_back(0);
	cubeElements.push_back(1);
	cubeElements.push_back(2);

	cubeElements.push_back(1);
	cubeElements.push_back(3);
	cubeElements.push_back(2);
	//Back
	cubeElements.push_back(6);
	cubeElements.push_back(5);
	cubeElements.push_back(4);

	cubeElements.push_back(7);
	cubeElements.push_back(5);
	cubeElements.push_back(6);
	//Left
	cubeElements.push_back(8);
	cubeElements.push_back(9);
	cubeElements.push_back(10);

	cubeElements.push_back(9);
	cubeElements.push_back(11);
	cubeElements.push_back(10);
	//Right
	cubeElements.push_back(12);
	cubeElements.push_back(14);
	cubeElements.push_back(13);

	cubeElements.push_back(13);
	cubeElements.push_back(14);
	cubeElements.push_back(15);
	//Top
	cubeElements.push_back(17);
	cubeElements.push_back(18);
	cubeElements.push_back(16);

	cubeElements.push_back(19);
	cubeElements.push_back(18);
	cubeElements.push_back(17);
	//Bottom
	cubeElements.push_back(22);
	cubeElements.push_back(21);
	cubeElements.push_back(20);

	cubeElements.push_back(23);
	cubeElements.push_back(21);
	cubeElements.push_back(22);

	//Creating a Positional and Colour VBO to tell the program what it needs to draw
	GLuint cubePositionsVBO;
	glGenBuffers(1, &cubePositionsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubePositionsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)* vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint cubeColoursVBO;
	glGenBuffers(1, &cubeColoursVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeColoursVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* cubeColours.size(), cubeColours.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Creating an element buffer to let the program know to use the elements created earlier
	CubeMesh.numElements = cubeElements.size();

	GLuint cubeElementEBO;
	glGenBuffers(1, &cubeElementEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeElementEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)* cubeElements.size(), cubeElements.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//Creating a VAO to wrap all the information gathered, and draw the cube
	glGenVertexArrays(1, &CubeMesh.vao);
	glBindVertexArray(CubeMesh.vao);

	glBindBuffer(GL_ARRAY_BUFFER, cubePositionsVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, cubeColoursVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeElementEBO);

	glBindVertexArray(0);

	Cube.meshVector.push_back(CubeMesh);
	modelVector.push_back(Cube);

//--TERRAIN-------------------------------------------------------------------------------------------------------------------------------------//
	m_program = CreateProgram("Data/Shaders/vertex_shader.vert", "Data/Shaders/fragment_shader.frag");
	
	Model Terrain;
	Terrain.modelName = "Terrain";
	Mesh TerrainMesh;

	Helpers::ImageLoader loadTerrain;
	if (!loadTerrain.Load("Data/Textures/Terrain_Sand.jpg"))
	{
		if (!loadTerrain.Load("Data/Textures/ErrorTexture.png"))
		{
			std::cout << "Failed to Load Terrain Texture" << std::endl;
			return false;
		}
	}

	Helpers::ImageLoader loadHeightMap;
	if (!loadHeightMap.Load("Data\\Heightmaps\\TerrainHeightmap.jpg"))
	{
		std::cout << "Failed to Load Terrain Heightmap" << std::endl;
		return false;
	}

	//Create vectors to handle specific items required for terrain generation
	std::vector<glm::vec3> colours;
	std::vector<glm::vec3> verts;
	std::vector<GLuint> terrainElem;
	std::vector<glm::vec2> uvCoords;
	std::vector<glm::vec3> normals;

	//Sets the number of squares, this can be edited for a bigger or smaller terrain space
	int numSquaresX = 200;
	int numSquaresZ = 200;
	//Sets the number of verts to the number of squares + 1 - this allows us to easily edit the terrain size
	int numVertsX = numSquaresX + 1;
	int numVertsZ = numSquaresZ + 1;
	float imageX = 0;
	float imageZ = 0;

	//Loops through the number of vertices on the x-axis
	for (int i = 0; i < numVertsX; i++)
	{
		//Loops through the number of vertices on the z-axis
		for (int j = 0; j < numVertsZ; j++)
		{
			//Pushes all the necessary information into the correct vectors
			verts.push_back(glm::vec3(i * 8, 0, j * 8));
			colours.push_back(glm::vec3(0.5, 0, 0.8));
			uvCoords.push_back(glm::vec2((j / (float)numSquaresX), (i / (float)numSquaresZ)));
			normals.push_back(glm::vec3(0, 0, 0));
		}
	}

	//Checking the values of the Heightmap to arrange the y values of the vertices based on the heightmap coplour/shade
	float vertXtoImage = (float)loadHeightMap.Width() / numVertsX;
	float vertZtoImage = (float)loadHeightMap.Height() / numVertsZ;

	GLbyte* imageData = (GLbyte*)loadHeightMap.GetData();

	for (int z = 0; z < numVertsZ; z++)
	{
		imageZ = vertZtoImage * z;

		for (int x = 0; x < numVertsX; x++)
		{
			imageX = vertXtoImage * x;
			size_t offset = ((size_t)imageX + (size_t)imageZ * loadHeightMap.Width()) * 4;
			BYTE height = imageData[offset];
			int myvec = (z * numVertsX) + x;
			verts[myvec].y = (float)height / 3;
		}
	}

	//Create a toggle to create a diamond pattern for the generation of terrain vertices
	bool diamondToggle = true;

	//This loop switched the toggle to ensure that the vertices are being generated on a diamond pattern
	for (int Z = 0; Z < numSquaresZ; Z++)
	{
		for (int X = 0; X < numSquaresX; X++)
		{
			int startVertIndex = (Z * numVertsX) + X;

			if (diamondToggle)
			{
				terrainElem.push_back(startVertIndex);
				terrainElem.push_back(startVertIndex + 1);
				terrainElem.push_back(startVertIndex + numVertsX + 1);

				terrainElem.push_back(startVertIndex);
				terrainElem.push_back(startVertIndex + numVertsX + 1);
				terrainElem.push_back(startVertIndex + numVertsX);
			}
			else
			{
				terrainElem.push_back(startVertIndex);
				terrainElem.push_back(startVertIndex + 1);
				terrainElem.push_back(startVertIndex + numVertsX);

				terrainElem.push_back(startVertIndex + 1);
				terrainElem.push_back(startVertIndex + numVertsX + 1);
				terrainElem.push_back(startVertIndex + numVertsX);
			}
			diamondToggle = !diamondToggle;
		}
		diamondToggle = !diamondToggle;
	}

	//Calculations for creating correct normals for the terrain
	for (int e = 0; e < terrainElem.size(); e += 3)
	{
		glm::vec3 element1 = verts[terrainElem[e]];
		glm::vec3 element2 = verts[terrainElem[(size_t)e + 1]];
		glm::vec3 element3 = verts[terrainElem[(size_t)e + 2]];

		glm::vec3 edge1 = element2 - element1;
		glm::vec3 edge2 = element3 - element1;

		normals[terrainElem[e]] += glm::cross(edge1, edge2);
		normals[terrainElem[(size_t)e + 1]] += glm::cross(edge1, edge2);
		normals[terrainElem[(size_t)e + 2]] += glm::cross(edge1, edge2);
	}

	for (int n = 0; n < normals.size(); n++)
	{
		normals[n] = glm::normalize(normals[n]);
	}

	GLuint terrainTxtrVBO;
	glGenBuffers(1, &terrainTxtrVBO);
	glBindBuffer(GL_ARRAY_BUFFER, terrainTxtrVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * uvCoords.size(), uvCoords.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenTextures(1, &TerrainMesh.txtr);
	glBindTexture(GL_TEXTURE_2D, TerrainMesh.txtr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, loadTerrain.Width(), loadTerrain.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, loadTerrain.GetData());
	glGenerateMipmap(GL_TEXTURE_2D);

	GLuint terrainNormVBO;
	glGenBuffers(1, &terrainNormVBO);
	glBindBuffer(GL_ARRAY_BUFFER, terrainNormVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), normals.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint terrainPosVBO;
	glGenBuffers(1, &terrainPosVBO);
	glBindBuffer(GL_ARRAY_BUFFER, terrainPosVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * verts.size(), verts.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint terrainColVBO;
	glGenBuffers(1, &terrainColVBO);
	glBindBuffer(GL_ARRAY_BUFFER, terrainColVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * colours.size(), colours.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	TerrainMesh.numElements = terrainElem.size();

	GLuint terrainElemEBO;
	glGenBuffers(1, &terrainElemEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainElemEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * terrainElem.size(), terrainElem.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &TerrainMesh.vao);
	glBindVertexArray(TerrainMesh.vao);

	glBindBuffer(GL_ARRAY_BUFFER, terrainPosVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, terrainNormVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, terrainTxtrVBO);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainElemEBO);

	glBindVertexArray(0);

	Terrain.meshVector.push_back(TerrainMesh);
	modelVector.push_back(Terrain);

//--Model--------------------------------------------------------------------------------------------------------------------------------------//

	Helpers::ModelLoader loadModel;
	if (!loadModel.LoadFromFile("Data/Models/jeep.obj"))
	{
		std::cout << "Failed to Load Jeep Model" << std::endl;
		return false;
	}

	Helpers::ImageLoader loadModelTxtr;
	if (!loadModelTxtr.Load("Data/Textures/jeep_army.jpg"))
	{
		std::cout << "Failed to Load Jeep Texture" << std::endl;
		return false;
	}

	Model Jeep;
	Jeep.modelName = "Jeep";

	for (const Helpers::Mesh& meshJeep : loadModel.GetMeshVector())
	{
		Mesh jeepMesh;

		GLuint modelTxtrVBO;
		glGenBuffers(1, &modelTxtrVBO);
		glBindBuffer(GL_ARRAY_BUFFER, modelTxtrVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * meshJeep.uvCoords.size(), meshJeep.uvCoords.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenTextures(1, &jeepMesh.txtr);
		glBindTexture(GL_TEXTURE_2D, jeepMesh.txtr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, loadModelTxtr.Width(), loadModelTxtr.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, loadModelTxtr.GetData());
		glGenerateMipmap(GL_TEXTURE_2D);

		GLuint modelNormVBO;
		glGenBuffers(1, &modelNormVBO);
		glBindBuffer(GL_ARRAY_BUFFER, modelNormVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * meshJeep.normals.size(), meshJeep.normals.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLuint modelPosVBO;
		glGenBuffers(1, &modelPosVBO);
		glBindBuffer(GL_ARRAY_BUFFER, modelPosVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)* meshJeep.vertices.size(), meshJeep.vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLuint modelElemEBO;
		glGenBuffers(1, &modelElemEBO);
		glBindBuffer(GL_ARRAY_BUFFER, modelElemEBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * meshJeep.elements.size(), meshJeep.elements.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		jeepMesh.numElements = meshJeep.elements.size();

		glGenVertexArrays(1, &jeepMesh.vao);
		glBindVertexArray(jeepMesh.vao);

		glBindBuffer(GL_ARRAY_BUFFER, modelPosVBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, modelNormVBO);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, modelTxtrVBO);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelElemEBO);

		glBindVertexArray(0);

		Jeep.meshVector.push_back(jeepMesh);
	}
	modelVector.push_back(Jeep);
	return true;
}

// Render the scene. Passed the delta time since last called.
void Renderer::Render(const Helpers::Camera& camera, float deltaTime)
{			
	// Configure pipeline settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Wireframe mode controlled by ImGui
	if (m_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Clear buffers from previous frame
	glClearColor(0.0f, 0.0f, 0.0f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Compute viewport and projection matrix
	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	const float aspect_ratio = viewportSize[2] / (float)viewportSize[3];
	glm::mat4 projection_xform = glm::perspective(glm::radians(45.0f), aspect_ratio, 0.1f, 4000.0f);

	// Compute camera view matrix and combine with projection matrix for passing to shader
	/*glm::mat4 view_xform = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());
	glm::mat4 view_xform2 = glm::mat4((glm::mat3(view_xform)));
	glm::mat4 combined_xform = projection_xform * view_xform;*/

	//Loops through each model in the model vector
	for (Model& model : modelVector)
	{
		//Loops through each mesh in the mesh vector
		for (Mesh& mesh : model.meshVector)
		{
			//sets the model_xform, and view_xform
			glm::mat4 model_xform = glm::mat4(1);
			glm::mat4 view_xform = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());

			//Sets the correct view_xforms, program etc. depending on the model name - This is to make sure each object is using the correct shaders etc.
			if (model.modelName == "Skybox")
			{
				glDepthMask(GL_FALSE);
				glDisable(GL_DEPTH_TEST);

				glm::mat4 view_xform2 = glm::mat4((glm::mat3(view_xform)));
				glm::mat4 combined_xform = projection_xform * view_xform2;

				glUseProgram(m_skyProgram);

				GLuint combined_xform_id = glGetUniformLocation(m_skyProgram, "combined_xform");
				glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(combined_xform));

				glActiveTexture(GL_TEXTURE0);
				glUniform1i(glGetUniformLocation(m_skyProgram, "sampler_tex"), 0);
				glBindTexture(GL_TEXTURE_2D, mesh.txtr);

				// Send the model matrix to the shader in a uniform
				GLuint model_xform_id = glGetUniformLocation(m_skyProgram, "model_xform");
				glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(model_xform));
			}
			else if (model.modelName == "Terrain")
			{
				glDepthMask(GL_TRUE);
				glEnable(GL_DEPTH_TEST);

				glm::mat4 combined_xform = projection_xform * view_xform;

				glUseProgram(m_program);

				GLuint combined_xform_id = glGetUniformLocation(m_program, "combined_xform");
				glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(combined_xform));

				glActiveTexture(GL_TEXTURE0);
				glUniform1i(glGetUniformLocation(m_program, "sampler_tex"), 0);
				glBindTexture(GL_TEXTURE_2D, mesh.txtr);

				// Send the model matrix to the shader in a uniform
				GLuint model_xform_id = glGetUniformLocation(m_program, "model_xform");
				glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(model_xform));
			}
			else if (model.modelName == "Jeep")
			{
				glDepthMask(GL_TRUE);
				glEnable(GL_DEPTH_TEST);

				glm::mat4 combined_xform = projection_xform * view_xform;

				glUseProgram(m_program);

				GLuint combined_xform_id = glGetUniformLocation(m_program, "combined_xform");
				glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(combined_xform));

				//Changes the scale, position, and angle of the model
				model_xform = glm::scale(model_xform, glm::vec3{ 0.4, 0.4, 0.4 });
				model_xform = glm::translate(model_xform, glm::vec3{ 2000, 60, 2600 });
				model_xform = glm::rotate(model_xform, 0.5f , glm::vec3{ 0, 1, 0 });

				glActiveTexture(GL_TEXTURE0);
				glUniform1i(glGetUniformLocation(m_program, "sampler_tex"), 0);
				glBindTexture(GL_TEXTURE_2D, mesh.txtr);

				// Send the model matrix to the shader in a uniform
				GLuint model_xform_id = glGetUniformLocation(m_program, "model_xform");
				glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(model_xform));
			}
			else if (model.modelName == "Cube")
			{
				glDepthMask(GL_TRUE);
				glEnable(GL_DEPTH_TEST);

				glm::mat4 combined_xform = projection_xform * view_xform;

				glUseProgram(m_cubeProgram);

				GLuint combined_xform_id = glGetUniformLocation(m_cubeProgram, "combined_xform");
				glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(combined_xform));

				model_xform = glm::scale(model_xform, glm::vec3{ 2.5, 2.5, 2.5 });
				model_xform = glm::translate(model_xform, glm::vec3{ 200, 40, 200 });

				//Constantly rotates the cube
				static float angle = 0;
				static bool rotateY = true;

				if (rotateY) // Rotate around y axis		
					model_xform = glm::rotate(model_xform, angle, glm::vec3{ 0, 1, 0 });
				else // Rotate around x axis		
					model_xform = glm::rotate(model_xform, angle, glm::vec3{ 1, 0, 0 });

				angle += 0.001f;
				if (angle > glm::two_pi<float>())
				{
					angle = 0;
					rotateY = !rotateY;
				}

				// Send the model matrix to the shader in a uniform
				GLuint model_xform_id = glGetUniformLocation(m_cubeProgram, "model_xform");
				glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(model_xform));
			}

			// Bind our VAO and render
			glBindVertexArray(mesh.vao);
			glDrawElements(GL_TRIANGLES, mesh.numElements, GL_UNSIGNED_INT, (void*)0);
		}
	}
}