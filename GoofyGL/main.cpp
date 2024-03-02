#include "imgui.h"
#include "imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>

void FramebufferSizeCallback(GLFWwindow* _window, int _width, int _height);
void ProcessInput(GLFWwindow* _window);
static unsigned int CompileShader(const std::string& source, unsigned int type);
static unsigned int CreateShader(const std::string& vertex_shader, const std::string& fragment_shader);
struct ShaderProgramSource
{
	std::string vertex_source;
	std::string fragment_source;
};
static ShaderProgramSource ParseShader(const std::string& filepath);

//const unsigned int screen_width = 800;
//const unsigned int screen_height = 600;

int main()
{
	//glfw initialisation and configuration
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//glfw window creation
	GLFWwindow* window = glfwCreateWindow(800, 600, "GoofyGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

	//glad, loading all opengl function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialise GLAD" << std::endl;
		return -1;
	}

	//build and compile shaders
	//------------------------
	ShaderProgramSource source = ParseShader("Basic.shader");
	std::cout << "VERTEX" << std::endl;
	std::cout << source.vertex_source << std::endl;
	std::cout << "FRAGMENT" << std::endl;
	std::cout << source.fragment_source << std::endl;
	unsigned int shader_program = CreateShader(source.vertex_source, source.fragment_source);
	glUseProgram(shader_program);

	//variables to be changed in the imgui window
	bool draw_shape = true;
	//to change size using imgui slider
	float size = 1.0f;
	//value to change colour with imgui
	float color[4] = { 0.9f,0.2f, 0.4f, 1.0f };

	//exporting variables to shaders (values can be changed in imgui window so have in update instead)
	//glUniform1f(glGetUniformLocation(shader_program, "size"), size);
	//glUniform4f(glGetUniformLocation(shader_program, "color"), color[0], color[1], color[2], color[3]);


	//set up vertex data and buffers and configure vertex attributes
	//glViewport(0, 0, 800, 600);
	GLfloat vertices[] = {
		//first triangle
		-0.9f, -0.5f, 0.0f,  //left 
		-0.0f, -0.5f, 0.0f,  //right
		-0.45f, 0.5f, 0.0f,  //top 
		//second triangle
		0.0f, -0.5f, 0.0f,  //left
		0.9f, -0.5f, 0.0f,  //right
		0.45f, 0.5f, 0.0f   //top 
	};
	
	/*unsigned int indices[] =
	{
		0, 1, 3, //first triangle
		1, 2, 3 //second triangle
	};*/

	//vertex buffer object (VBO) is where we hold vertex data that is sent to the gpu. VBOs can a 
	//large number of vertices in GPU memory
	//vertex array object (VAO) holds pointers to the VBO(s) and tells opengl how to interpret them
	//they allow us to quickly switch between different VBOs
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	//bind the vertex array object first and then bind and set vertex buffers and configure vertex attributes
	//binding an object makes it the "current" object basically, so any function that modifies a type
	//of object would modify the currently bound object
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//this store vertices in the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//comment out ebo as we dont need them to draw 2 triangles, used them before to draw rectangle as we didn't want overhead
	//of specifying the same vertices. ebo only stores the vertices needed and let us dictate the order of drawing, so there was no overhead
	/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);*/

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(0);

	//this is allowed, the call to glVertexAttribPoiner registered VBO as the vertex attribute's
	//bound vertex buffer object so after we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	 // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	//glBindVertexArray(0);

	//imgui test
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	//uncomment to draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while (!glfwWindowShouldClose(window))
	{
		//inpur
		ProcessInput(window);

		//rendering
		glClearColor(0.2f, 0.6f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//tell opengl a new frame is about to begin
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//glUseProgram(shader_program);
		glBindVertexArray(VAO); //as theres a single vao theres no need to bind it everytime, but doing so anyway

		//draw triangles if imgui checkbox is ticked
		if (draw_shape)
		{
			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		//glBindVertexArray(0); //no need to unbind everytime
		//check and poll IO events and swap front and back buffer

		//imgui window creation, name and features (text, checkbox etc.)
		ImGui::Begin("Test Window");
		ImGui::Text("Hello GoofyGL");
		ImGui::Checkbox("Draw triangle(s)", &draw_shape);
		ImGui::SliderFloat("Size", &size, 0.5f, 2.0f);
		ImGui::ColorEdit4("Color", color);
		ImGui::End();

		//export variables to shader
		glUseProgram(shader_program);
		glUniform1f(glGetUniformLocation(shader_program, "size"), size);
		glUniform4f(glGetUniformLocation(shader_program, "color"), color[0], color[1], color[2], color[3]);

		//render imgui elements
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//end imgui processes
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	//delete all created objects
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shader_program);

	//glfwterminate clearing all previously allocated glfw resources
	glfwTerminate();
	return 0;
}

static ShaderProgramSource ParseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);
	//std::filesystem::exists(filepath);

	enum class ShaderType
	{
		NONE = -1,
		VERTEX = 0,
		FRAGMENT = 1
	};
	
	std::string line;
	std::stringstream string_stream[2]; //stack allocated array, 1 for vertex 1 for fragment
	ShaderType type = ShaderType::NONE;

	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
			{
				//set mode to vertex
				type = ShaderType::VERTEX;
			}
			else if (line.find("fragment") != std::string::npos)
			{
				//set mode to fragment
				type = ShaderType::FRAGMENT;
			}
		}
		else
		{
			string_stream[(int)type] << line << '\n';
		}
	}

	//std::cout << string_stream[0].str() << std::endl;
	//std::cout << string_stream[1].str() << std::endl;
	return { string_stream[0].str(), string_stream[1].str() };
}

static unsigned int CompileShader(const std::string& source, unsigned int type)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int success;
	char info_log[512];
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if (!success) //if success == GL_FALSE
	{
		glGetShaderInfoLog(id, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::" << 
			(type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << 
			"::COMPILATION_FAILED\n" << info_log << std::endl;

		glDeleteShader(id);
		return 0;
	}

	return id;
}

static unsigned int CreateShader(const std::string& vertex_shader, const std::string& fragment_shader)
{
	//build shaders
	//------------------------
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(vertex_shader, GL_VERTEX_SHADER);
	unsigned int fs = CompileShader(fragment_shader, GL_FRAGMENT_SHADER);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);
	//glDeleteProgram(program)

	return program;
}

//glfw, whenever the window size changed this callback function executes
void FramebufferSizeCallback(GLFWwindow* _window, int _width, int _height)
{
	glViewport(0, 0, _width, _height);
}

//process input, ask glfw whether relevant keys are pressed/released this frame and react accordingly
void ProcessInput(GLFWwindow* _window)
{
	if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(_window, true);
	}
}