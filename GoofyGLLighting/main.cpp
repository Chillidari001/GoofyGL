#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "../GoofyGL/shader_manager.h"
//#include "camera.h"
#include "../GoofyGL/stb_image.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "../GoofyGL/Camera.h"

void FramebufferSizeCallback(GLFWwindow* _window, int _width, int _height);
void ProcessInput(GLFWwindow* _window);
void MouseCallback(GLFWwindow* _window, double _xpos, double _ypos);
void ScrollCallback(GLFWwindow* _window, double _xoffset, double _yoffset);

const unsigned int screen_width = 1280;
const unsigned int screen_height = 720;
//------------------------------------
//camera
Camera main_camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), base_yaw, base_pitch);
//ExampleCamera main_camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool first_mouse = true;
float last_x = screen_width / 2;
float last_y = screen_height / 2;

//timing
float delta_time = 0.0f; //time between current and last frame
float last_frame = 0.0f;

bool cursor_enabled = false;

int main(void)
{
	//glfw initialisation and configuration
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//glfw window creation
	GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "GoofyGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetScrollCallback(window, ScrollCallback);

	//glad, loading all opengl function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialise GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	//build and compile shaders
	//------------------------
	Shader first_shader("shader.vs", "shader.fs");

	//set up vertex data and buffers and configure vertex attributes
	//glViewport(0, 0, 800, 600);
	GLfloat vertices[] = {
		/*
			// positions          // colors           // texture coords
		0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
		0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
		//second triangle
		/*0.0f, -0.5f, 0.0f,  //left
		0.9f, -0.5f, 0.0f,  //right
		0.45f, 0.5f, 0.0f*/   //top 

		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	glm::vec3 cube_positions[] =
	{
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	GLuint indices[] =
	{
		0, 1, 3, //first triangle
		1, 2, 3 //second triangle
	};

	//vertex buffer object (VBO) is where we hold vertex data that is sent to the gpu. VBOs can hold a 
	//large number of vertices in GPU memory
	//vertex array object (VAO) holds pointers to the VBO(s) and tells opengl how to interpret them
	//they allow us to quickly switch between different VBOs
	GLuint VBO, VAO, EBO; //cube vao, vbo
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);
	//bind the vertex array object first and then bind and set vertex buffers and configure vertex attributes
	//binding an object makes it the "current" object basically, so any function that modifies a type
	//of object would modify the currently bound object
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//this store vertices in the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//comment out ebo as we dont need them to draw 2 triangles, used them before to draw rectangle as we didn't want overhead
	//of specifying the same vertices. ebo only stores the vertices needed and let us dictate the order of drawing, so there was no overhead
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//position attribs
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//colour attribs
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//texture coord attribs
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
	unsigned int light_cubeVAO;
	glGenVertexArrays(1, &light_cubeVAO);
	glBindVertexArray(light_cubeVAO);

	//position attribs again for light cube (i think)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//this is allowed, the call to glVertexAttribPoiner registered VBO as the vertex attribute's
	//bound vertex buffer object so after we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	 // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	//glBindVertexArray(0);

	//texture handling
	unsigned int texture1, texture2;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	//set texture wrapping/filtering options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//load and generate the texture
	int width, height, nr_channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("container.jpg", &width, &height, &nr_channels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture1" << std::endl;
	}
	stbi_image_free(data);

	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	//set texture wrapping/filtering options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//load and generate the texture
	//int width, height, nr_channels;
	data = stbi_load("awesomeface.png", &width, &height, &nr_channels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture2" << std::endl;
	}
	stbi_image_free(data);

	//variables to be changed in the imgui window
	bool draw_shape = true;
	//to change size using imgui slider
	float size = 1.0f;
	//value to change colour with imgui
	float color[4] = { 0.8f,0.3f, 0.02f, 1.0f };

	//exporting variables to shaders
	//glUseProgram(shader_program);
	//glUniform1f(glGetUniformLocation(shader_program, "size"), size);
	//glUniform4f(glGetUniformLocation(shader_program, "color"), color[0], color[1], color[2], color[3]);

	//uncomment to draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//we can tell opengl which texture unit each shader sampler belongs to before the render loop, only need to do it once
	first_shader.Use();

	//glUniform1i(glGetUniformLocation(first_shader.ID, "texture1"), 0); // set it manually
	//glUniform1i(glGetUniformLocation(first_shader.ID, "texture2"), 1); // set it manually
	first_shader.SetInt("container_texture", 0);
	first_shader.SetInt("face_texture", 1);


	while (!glfwWindowShouldClose(window))
	{
		float current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		//input
		ProcessInput(window);

		if (cursor_enabled)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		if (!cursor_enabled)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}

		//rendering
		glClearColor(0.2f, 0.6f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//bind texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		//glBindVertexArray(VAO);

		//latest camera stuff

		//pass projection matrix to shader, can change every frame
		glm::mat4 projection = glm::perspective(glm::radians(main_camera.zoom), (float)screen_width / (float)screen_height, 0.1f, 100.0f); //fov, aspect ratio, near, far
		first_shader.SetMat4("projection", projection); //currently projection matrix is set each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.

		//camera/view transformation
		glm::mat4 view = main_camera.GetViewMatrix();
		first_shader.SetMat4("view", view);

		glBindVertexArray(VAO); //as theres a single vao theres no need to bind it everytime, but doing so anyway

		// update the uniform color
		//float time_value = glfwGetTime();
		//float green_value = sin(time_value) / 2.0f + 0.5f;
		//int vertex_color_location = glGetUniformLocation(shader_program, "time_color");
		//glUniform4f(vertex_color_location, 0.0f, green_value, 0.0f, 1.0f);

		//draw triangles if opengl checkbox is ticked
		if (draw_shape)
		{
			for (unsigned int i = 0; i < 10; i++)
			{
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, cube_positions[i]);
				float angle = 20.0f * i;
				model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
				first_shader.SetMat4("model", model);

				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}


		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//delete all created objects
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	//glDeleteProgram(shader_program);

	//glfwterminate clearing all previously allocated glfw resources
	glfwTerminate();
	return 0;
}

//glfw, whenever the window size changed this callback function executes
void FramebufferSizeCallback(GLFWwindow* _window, int _width, int _height)
{
	glViewport(0, 0, _width, _height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void ProcessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS)
		cursor_enabled = !cursor_enabled;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		main_camera.ProcessKeyboard(FORWARD, delta_time);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		main_camera.ProcessKeyboard(BACKWARD, delta_time);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		main_camera.ProcessKeyboard(LEFT, delta_time);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		main_camera.ProcessKeyboard(RIGHT, delta_time);
}


void MouseCallback(GLFWwindow* _window, double _xpos, double _ypos)
{
	if (!cursor_enabled)
	{
		float x_pos = static_cast<float>(_xpos);
		float y_pos = static_cast<float>(_ypos);

		if (first_mouse)
		{
			last_x = x_pos;
			last_y = y_pos;
			first_mouse = false;
		}

		float x_offset = x_pos - last_x;
		float y_offset = last_y - y_pos; //reversed since y-coordinates range from bottom to top

		last_x = x_pos;
		last_y = y_pos;

		main_camera.ProcessMouseMovement(x_offset, y_offset);
	}
}

void ScrollCallback(GLFWwindow* _window, double _xoffset, double _yoffset)
{
	if (!cursor_enabled)
	{
		main_camera.ProcessMouseScroll(static_cast<float>(_yoffset));
	}
}
