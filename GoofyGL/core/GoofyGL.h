#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "shader_manager.h"
#include "../Cameras/Header/Camera.h"
#include "../external/stb_image.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "../Cameras/Header/ExampleCamera.h"

//glfw shit being weird


class GoofyGL
{
public:
	GoofyGL();
	~GoofyGL();
	//int GoofyGLInit();
	void GoofyGLRun();
	
	//timing
	float delta_time = 0.0f; //time between current and last frame
	float last_frame = 0.0f;
	glm::vec3 light_pos;
	const unsigned int screen_width = 1280;
	const unsigned int screen_height = 720;
	Camera main_camera;
	bool cursor_enabled = false;
	bool first_mouse = true;
	float last_x = screen_width / 2;
	float last_y = screen_height / 2;
private:
	GLFWwindow* window;
	static void FramebufferSizeCallback(GLFWwindow* _window, int _width, int _height);
	static void MouseCallback(GLFWwindow* _window, double _xpos, double _ypos);
	static void ScrollCallback(GLFWwindow* _window, double _xoffset, double _yoffset);
	void ProcessInput(GLFWwindow* _window);
	unsigned int LoadTexture(char const* path);
};

