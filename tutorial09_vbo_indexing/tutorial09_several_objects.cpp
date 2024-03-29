// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

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

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Tutorial 09 - Rendering several models", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
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
    glfwSetCursorPos(window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );

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
	GLuint Texture = loadDDS("uvmap.DDS");
	GLuint Texture2 = loadBMP_custom("uvtemplate.bmp");
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	GLfloat rectangleVertices[] = {
		-4.0f, -4.0f, 0.0f,
		4.0f, -4.0f, 0.0f,
		4.0f, 4.0f, 0.0f,
		-4.0f, 4.0f, 0.0f
	};
	GLfloat rectangleNormals[] = {
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f
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

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	// Load it into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// For speed computation
	double lastTime = glfwGetTime();
	double lastTime2 = glfwGetTime();
	int nbFrames = 0;
	int lightbutton = 0;
	do{

		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0/double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		
		double currentTime2 = glfwGetTime();
		if (currentTime2 - lastTime2 >= 3){
		if (glfwGetKey( window, GLFW_KEY_L ) == GLFW_PRESS) 
		{
			lightbutton = 1 - lightbutton;
			lastTime2= currentTime2;
			glUniform1i(glGetUniformLocation(programID, "ToggleLight"), lightbutton);
			printf("%d\n",lightbutton);
		}
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		
		

		
		// Use our shader
		glUseProgram(programID);
	
		glm::vec3 lightPos = glm::vec3(4,4,4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"
		
		//draw rectangle
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



		////// Start of the rendering of the first object //////
		glm::mat4 ModelMatrix1 = glm::mat4(1.0);

		glm::mat4 rotationMatrix1 = glm::rotate(glm::mat4(1.0f), 1.57f, glm::vec3(1,0,0));
		glm::mat4 rotationMatrix2 = glm::rotate(glm::mat4(1.0f), 1.57f, glm::vec3(0, 0, 1));
		ModelMatrix1 = glm::translate(ModelMatrix1, glm::vec3(1.75f, 0.0f, 0.75f));
		ModelMatrix1 = ModelMatrix1*rotationMatrix2 * rotationMatrix1  ;
		glm::mat4 MVP1 = ProjectionMatrix * ViewMatrix * ModelMatrix1;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP1[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix1[0][0]);


		// Bind our texture in Texture Unit 0
		//glActiveTexture(GL_TEXTURE0);
		//glUniform1i(glGetUniformLocation(programID, "useTexture"), 1);
		//glBindTexture(GL_TEXTURE_2D, Texture);
		//// Set our "myTextureSampler" sampler to user Texture Unit 0
		//glUniform1i(TextureID, 0);

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

		//the left one------------------------------------------------------------------------------------//
		glm::mat4 ModelMatrix3 = glm::mat4(1.0);

		rotationMatrix1 = glm::rotate(glm::mat4(1.0f), 1.57f, glm::vec3(1, 0, 0));
		rotationMatrix2 = glm::rotate(glm::mat4(1.0f), -1.57f, glm::vec3(0, 0, 1));
		ModelMatrix3 = glm::translate(ModelMatrix3, glm::vec3(-1.75f, 0.0f, 0.75f));
		ModelMatrix3 = ModelMatrix3 * rotationMatrix2 * rotationMatrix1;
		glm::mat4 MVP3 = ProjectionMatrix * ViewMatrix * ModelMatrix3;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP3[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix3[0][0]);


		// Bind our texture in Texture Unit 0
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, Texture);
		//// Set our "myTextureSampler" sampler to user Texture Unit 0
		//glUniform1i(TextureID, 0);

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

		//*******************the upper*************************//
		////// End of rendering of the first object //////
		////// Start of the rendering of the second object //////

		// In our very specific case, the 2 objects use the same shader.
		// So it's useless to re-bind the "programID" shader, since it's already the current one.
		//glUseProgram(programID);
		
		// Similarly : don't re-set the light position and camera matrix in programID,
		// it's still valid !
		// *** You would have to do it if you used another shader ! ***
		//glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		//glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

		
		// Again : this is already done, but this only works because we use the same shader.
		//// Bind our texture in Texture Unit 0
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, Texture);
		//// Set our "myTextureSampler" sampler to user Texture Unit 0
		//glUniform1i(TextureID, 0);
		
		
		// BUT the Model matrix is different (and the MVP too)
		glm::mat4 ModelMatrix2 = glm::mat4(1.0);
		rotationMatrix1 = glm::rotate(glm::mat4(1.0f), 1.57f, glm::vec3(1, 0, 0));
		rotationMatrix2 = glm::rotate(glm::mat4(1.0f), 3.14f, glm::vec3(0, 0, 1));
		ModelMatrix2 = glm::translate(ModelMatrix2, glm::vec3(0.0f, 1.57f, 0.75f));
		ModelMatrix2 = ModelMatrix2*rotationMatrix2*rotationMatrix1;

		glm::mat4 MVP2 = ProjectionMatrix * ViewMatrix * ModelMatrix2;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix2[0][0]);


		// The rest is exactly the same as the first object
		
		// 1rst attribute buffer : vertices
		//glEnableVertexAttribArray(vertexPosition_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		//glEnableVertexAttribArray(vertexUVID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		//glEnableVertexAttribArray(vertexNormal_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);


		////// End of rendering of the second object //////
		glm::mat4 ModelMatrix4 = glm::mat4(1.0);
		rotationMatrix1 = glm::rotate(glm::mat4(1.0f), 1.57f, glm::vec3(1, 0, 0));
		//glm::mat4 rotationMatrix2 = glm::rotate(glm::mat4(1.0f), 1.57f, glm::vec3(0, 0, 1));
		
		ModelMatrix4 = glm::translate(ModelMatrix4, glm::vec3(0.0f, -1.57f, 0.75f));
		
		ModelMatrix4 = ModelMatrix4 * rotationMatrix1;

		glm::mat4 MVP4 = ProjectionMatrix * ViewMatrix * ModelMatrix4;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP4[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix4[0][0]);


		// The rest is exactly the same as the first object

		// 1rst attribute buffer : vertices
		//glEnableVertexAttribArray(vertexPosition_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		//glEnableVertexAttribArray(vertexUVID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		//glEnableVertexAttribArray(vertexNormal_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);



		glDisableVertexAttribArray(vertexPosition_modelspaceID);
		glDisableVertexAttribArray(vertexUVID);
		glDisableVertexAttribArray(vertexNormal_modelspaceID);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

