// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include <thread>
#include <atomic>
#include <random>
#include <iostream>
#include <glm/gtc/type_ptr.hpp> // Include this for value_ptr
// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

std::atomic<bool> animationRunning(false);

class item {
public:
	float l;
	float ang;
	GLuint vertexbuffer = 0, uvbuffer = 0, normalbuffer = 0, elementbuffer = 0;
	GLuint Texture = loadDDS("uvmap.DDS");
	item(float length,float angle):  l(length), ang(angle) {}
protected:
	float velocity = 0.015f;
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
	GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
	GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
	GLuint vertexUVID = glGetAttribLocation(programID, "vertexUV");
	GLuint vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");
	std::vector<unsigned short> indices;
public:
	void initialize() {
		MatrixID = glGetUniformLocation(programID, "MVP");
		ViewMatrixID = glGetUniformLocation(programID, "V");
		ModelMatrixID = glGetUniformLocation(programID, "M");
		//programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
		Texture = loadDDS("uvmap.DDS");
		TextureID = glGetUniformLocation(programID, "myTextureSampler");
		vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
		vertexUVID = glGetAttribLocation(programID, "vertexUV");
		vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals;
		bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);
		std::vector<glm::vec3> indexed_vertices;
		std::vector<glm::vec2> indexed_uvs;
		std::vector<glm::vec3> indexed_normals;
		indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
		// Load it into a VBO
		glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);
		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);
		glGenBuffers(1, &normalbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);
		// Generate a buffer for the indices as well
		glGenBuffers(1, &elementbuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	}

	void draw(float theta, glm::mat4& ProjectionMatrix, glm::mat4& ViewMatrix) {

		
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		//facing direction fixed
		glm::mat4 rotationMatrix1 = glm::rotate(glm::mat4(1.0f), 1.57f, glm::vec3(1, 0, 0));
		//translate at 
		glm::mat4 TranslationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 0.0f, 0.0f));
		//variable rotation
		glm::mat4 rotationMatrix2 = glm::rotate(glm::mat4(1.0f), theta, glm::vec3(0, 0, 1));
		//std::cout << "theta" << theta << std::endl;
		ModelMatrix = rotationMatrix2 * TranslationMatrix * rotationMatrix1 * ModelMatrix;

		//glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(programID, "useTexture"), 1);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(vertexPosition_modelspaceID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			vertexPosition_modelspaceID, // The attribute we want to configure
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(vertexUVID);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			vertexUVID,                       // The attribute we want to configure
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);
		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(vertexNormal_modelspaceID);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			vertexNormal_modelspaceID,        // The attribute we want to configure
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);
		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,      // mode
			indices.size(),    // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0           // element array buffer offset
		);

		
		glDisableVertexAttribArray(vertexPosition_modelspaceID);
		glDisableVertexAttribArray(vertexUVID);
		glDisableVertexAttribArray(vertexNormal_modelspaceID);
	}

	void update(std::vector<item>& items) {
		while(true){
			if (animationRunning)
			{
				
				std::random_device rd;
				std::mt19937 gen(rd()); // Mersenne Twister 32-bit PRNG
				std::uniform_real_distribution<float> distribution(0.01f, 0.02f); // 
				float randomValue1 = distribution(gen);
				float randomValue2 = distribution(gen);
				float randomValue3 = distribution(gen);
				float randomValue4 = distribution(gen);

				if (items[0].ang < -0.75) 
				{					
					items[0].velocity = randomValue1;
					if ((items[0].velocity + items[0].ang) < 0.75) { items[0].velocity = 0.03f; }
				}
				if ((items[1].ang - items[0].ang)<0.1) {
						items[0].velocity = -randomValue1;
						items[1].velocity = randomValue1;
					if ((items[1].ang - items[0].ang + items[1].velocity - items[0].velocity)<0.1)
					{
						items[0].velocity = -0.03f;
						items[1].velocity = 0.03f;
					}
					}
				if ((items[2].ang - items[1].ang) < 0.1) {
					items[0].velocity = -randomValue1;
					items[1].velocity = randomValue1;
					if ((items[2].ang - items[1].ang + items[2].velocity - items[1].velocity) < 0.1)
					{
						items[1].velocity = -0.03f;
						items[2].velocity = 0.03f;
					}
				}
				if ((items[3].ang - items[2].ang) < 0.1) {
					items[2].velocity = -randomValue1;
					items[3].velocity = randomValue1;
					if ((items[3].ang - items[2].ang + items[3].velocity - items[2].velocity) < 0.1)
					{
						items[2].velocity = -0.03f;
						items[3].velocity = 0.03f;
					}
				}
				if (items[3].ang > 4)
				{
					items[3].velocity = -randomValue4;
					if ((items[3].velocity + items[3].ang) > 4) { items[0].velocity = -0.03f; }
				}
				items[0].ang += items[0].velocity;
				items[1].ang += items[1].velocity;
				items[2].ang += items[2].velocity;
				items[3].ang += items[3].velocity;
				//std::cout << items[0].ang << std::endl;
				//std::cout << items[1].ang << std::endl;
				//std::cout << items[2].ang << std::endl;
				//std::cout << items[3].ang << std::endl;
				//printf("-----------------------------------------------------------");
			std::this_thread::sleep_for(std::chrono::milliseconds(16)); // Adjust frame rate
		}
		}
	}



};


int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Tutorial 09 - Rendering several models", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

	// Get a handle for our "MVP" uniform
	//GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	//GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	//GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Get a handle for our buffers
	//GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
	//GLuint vertexUVID = glGetAttribLocation(programID, "vertexUV");
	//GLuint vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");
	//GLuint vertexColorID = glGetAttribLocation(programID, "vertexColor");
	// Load the texture
	GLuint Texture2 = loadBMP_custom("WoodPlanksOld.bmp");
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");



	item it1(4.0f, 0.0f);
	it1.initialize();
	item it2(4.0f, 0.75f);
	it2.initialize();
	item it3(4.0f, 1.57f);
	it3.initialize();
	item it4(4.0f, 3.14f);
	it4.initialize();
	std::vector <item> items;
	items.push_back(it1);
	items.push_back(it2);
	items.push_back(it3);
	items.push_back(it4);

	std::vector<std::thread> threads;
	for (item &it : items) {
		threads.push_back(std::thread(&item::update, &it, std::ref(items)));
	}
	



	// Initialize lights with positions, colors, and powers

	// Get a handle for our "LightPosition" uniform
	//glUseProgram(programID);
	//GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace[0]");
	//glm::vec3 lightPos = glm::vec3(4, 4, 4);
	//glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
	//GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace[1]");
	//glm::vec3 lightPos = glm::vec3(4, 4, 4);
	//glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Get a handle for our buffers
	GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
	GLuint vertexUVID = glGetAttribLocation(programID, "vertexUV");
	GLuint vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");
	GLuint vertexColorID = glGetAttribLocation(programID, "vertexColor");
	// Load the texture



	GLfloat rectangleVertices[] = {
		-8.0f,  -4.0f,-8.0f,
		8.0f,  -4.0f,-8.0f,
		8.0f,  -4.0f,8.0f,
		-8.0f,  -4.0f,8.0f,
	};
	GLfloat rectangleNormals[] = {
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f
	};

	GLfloat rectangleUVs[] = {
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 1.0f
	};
	GLfloat g_color_buffer_data[] = {
	0.0f, 1.0f,0.0f,
	0.0f, 1.0f,0.0f,
	0.0f, 1.0f,0.0f,
	0.0f, 1.0f,0.0f
	};

	GLuint rectangleVBO, rectangleNormalVBO, rectangleUVVBO;
	glGenBuffers(1, &rectangleVBO);
	glGenBuffers(1, &rectangleNormalVBO);
	glGenBuffers(1, &rectangleUVVBO);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, rectangleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), rectangleVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, rectangleNormalVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleNormals), rectangleNormals, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, rectangleUVVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleUVs), rectangleUVs, GL_STATIC_DRAW);

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;



	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//it1.update(std::ref(items));
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix0 = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix0;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(programID, "useTexture"), 1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		// Render the rectangle vertices
		/*glUniform1i(glGetUniformLocation(programID, "useTexture"), 0);*/
		glBindBuffer(GL_ARRAY_BUFFER, rectangleVBO);
		glEnableVertexAttribArray(vertexPosition_modelspaceID);
		glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(vertexColorID);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(
			vertexColorID,               // The attribute we want to configure
			3,                           // size
			GL_FLOAT,                    // type
			GL_FALSE,                    // normalized?
			0,                           // stride
			(void*)0                     // array buffer offset
		);
		// Render the rectangle normals
		glBindBuffer(GL_ARRAY_BUFFER, rectangleNormalVBO);
		glEnableVertexAttribArray(vertexNormal_modelspaceID); // Assuming 1 is the location for normals in your shader
		glVertexAttribPointer(vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//// Render the rectangle UVs
		glBindBuffer(GL_ARRAY_BUFFER, rectangleUVVBO);
		glEnableVertexAttribArray(vertexUVID); // Assuming 2 is the location for UVs in your shader
		glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);



		glDrawArrays(GL_QUADS, 0, 4); // Drawing as a quad
		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}
		if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		{
			animationRunning = true;
		}
		
		std::random_device rd;
		std::mt19937 gen(rd()); // Mersenne Twister 32-bit PRNG
		std::uniform_int_distribution<int> distribution(3, 7);
		glm::vec3 lightPositions[4];
		float lightPowers[4];

		for (int i = 0; i < 4; i++) {
			lightPositions[i] = glm::vec3(4 * cos(items[i].ang), 4 * sin(items[i].ang), 0);
			lightPowers[i] = float(distribution(gen));
		}

		glUseProgram(programID);
		glUniform3fv(glGetUniformLocation(programID, "LightPositions_worldspace"), 4, glm::value_ptr(lightPositions[0]));
		glUniform1fv(glGetUniformLocation(programID, "LightPower"), 4, lightPowers);
		glUniform1i(glGetUniformLocation(programID, "NumLights"), 4); // Number of active lights


		it1.draw(items[0].ang, ProjectionMatrix, ViewMatrix);
		it2.draw(items[1].ang, ProjectionMatrix, ViewMatrix);
		it3.draw(items[2].ang, ProjectionMatrix, ViewMatrix);
		it4.draw(items[3].ang, ProjectionMatrix, ViewMatrix);

		glfwSwapBuffers(window);
		glfwPollEvents();


		// Swap buffers


	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);
	glfwTerminate();
	// Cleanup VBO and shader
	for(item &it: items){
	glDeleteBuffers(1, &it.vertexbuffer);
	glDeleteBuffers(1, &it.uvbuffer);
	glDeleteBuffers(1, &it.normalbuffer);
	glDeleteBuffers(1, &it.elementbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &it.Texture);
	}
	
	for (std::thread& thread : threads) {
		thread.join();
	}
	// Close OpenGL window and terminate GLFW
	

	return 0;
}

