#include "GoofyGL.h"

GoofyGL::GoofyGL()
{
	main_camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), base_yaw, base_pitch);
	light_pos = glm::vec3(1.2f, 1.0f, 2.0f);


	//glfw initialisation and configuration
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//glfw window creation
	window = glfwCreateWindow(screen_width, screen_height, "GoofyGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		//return -1;
	}
	glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
	//GoofyGL* obj = reinterpret_cast<GoofyGL*>(glfwGetWindowUserPointer(window));
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetScrollCallback(window, ScrollCallback);

	//glad, loading all opengl function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialise GLAD" << std::endl;
		//return -1;
	}

	glEnable(GL_DEPTH_TEST);

	//return 0;
}

GoofyGL::~GoofyGL()
{
	
}

/*int GoofyGL::GoofyGLInit()
{
	return 0;
}*/

void GoofyGL::GoofyGLRun()
{
	//build and compile shaders
	//------------------------
	//Shader first_shader("shader.vs", "shader.fs");
	Shader lighting_shader("assets/shaders/light_shader.vs", "assets/shaders/light_shader.fs");
	Shader light_cube_shader("assets/shaders/light_cube.vs", "assets/shaders/light_cube.fs");

	//set up vertex data and buffers and configure vertex attributes
	//glViewport(0, 0, 800, 600);
	GLfloat vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	glm::vec3 cube_positions[] = {
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

	glm::vec3 point_lights_positions[] = {
	glm::vec3(0.7f,  0.2f,  2.0f),
	glm::vec3(2.3f, -3.3f, -4.0f),
	glm::vec3(-4.0f,  2.0f, -12.0f),
	glm::vec3(0.0f,  0.0f, -3.0f)
	};

	//vertex buffer object (VBO) is where we hold vertex data that is sent to the gpu. VBOs can hold a 
	//large number of vertices in GPU memory
	//vertex array object (VAO) holds pointers to the VBO(s) and tells opengl how to interpret them
	//they allow us to quickly switch between different VBOs
	GLuint VBO, VAO; //cube vao, vbo
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//this store vertices in the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


	glBindVertexArray(VAO);

	//comment out ebo as we dont need them to draw 2 triangles, used them before to draw rectangle as we didn't want overhead
	//of specifying the same vertices. ebo only stores the vertices needed and let us dictate the order of drawing, so there was no overhead
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//texture coord attribute
	// normal attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//colour attribs
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3* sizeof(float)));
	//glEnableVertexAttribArray(1);

	//texture coord attribs
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(2);

	//configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
	unsigned int light_cubeVAO;
	glGenVertexArrays(1, &light_cubeVAO);
	glBindVertexArray(light_cubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//position attribs again for light cube (i think)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int diffuse_map = LoadTexture("assets/textures/container2.png");
	unsigned int specular_map = LoadTexture("assets/textures/container2_specular.png");

	//imgui test
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	//variables to manipulate in imgui window
	float light_linear = 0.09f;
	float light_quadratic = 0.032f;
	glm::vec3 _light_pos = glm::vec3(1.2f, 1.0f, 2.0f);



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
		glClearColor(0.00f, 0.00f, 0.00f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//tell opengl a new frame is about to begin
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//glBindVertexArray(VAO);

		//activate shader when setting uniforms/drawing objects
		lighting_shader.Use();
		//glUniform1i(glGetUniformLocation(lighting_shader.ID, "object_color"), 1); // set it manually
		//lighting_shader.SetVec3("object_color", glm::vec3(1.0f, 0.5f, 0.31f));
		//lighting_shader.SetVec3("light_color", glm::vec3(1.0f, 1.0f, 1.0f));
		
		
		lighting_shader.SetVec3("view_pos", main_camera.position);

		glm::vec3 light_color;
		light_color.x = sin(glfwGetTime() * 2.0f);
		light_color.y = sin(glfwGetTime() * 0.7f);
		light_color.z = sin(glfwGetTime() * 1.3f);
		glm::vec3 diffuse_color = light_color * glm::vec3(0.5f);
		glm::vec3 ambient_color = diffuse_color * glm::vec3(0.2f);

		//set values of properties in the different light structs
		//directional light
		lighting_shader.SetVec3("directional_light.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
		lighting_shader.SetVec3("directional_light.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		lighting_shader.SetVec3("directional_light.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
		lighting_shader.SetVec3("directional_light.specular", glm::vec3(0.5f, 0.5f, 0.5f));

		//looping through the four point lights and setting their values
		for (int i = 0; i < 4; i++)
		{
			std::string number = std::to_string(i);
			lighting_shader.SetVec3("point_lights[" + number + "].position", point_lights_positions[i]);
			lighting_shader.SetVec3("point_lights[" + number + "].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
			lighting_shader.SetVec3("point_lights[" + number + "].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
			lighting_shader.SetVec3("point_lights[" + number + "].specular", glm::vec3(1.0f, 1.0f, 1.0f));
			lighting_shader.SetFloat("point_lights[" + number + "].constant", 1.0f);
			lighting_shader.SetFloat("point_lights[" + number + "].linear", 0.09f);
			lighting_shader.SetFloat("point_lights[" + number + "].quadratic", 0.032f);
		}

		//spotlight
		lighting_shader.SetVec3("spot_light.position", main_camera.position);
		lighting_shader.SetVec3("spot_light.direction", main_camera.front);
		lighting_shader.SetVec3("spot_light.ambient", glm::vec3(0.00f, 0.00f, 0.00f));
		lighting_shader.SetVec3("spot_light.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		lighting_shader.SetVec3("spot_light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		lighting_shader.SetFloat("spot_light.constant", 1.0f);
		lighting_shader.SetFloat("spot_light.linear", 0.09f);
		lighting_shader.SetFloat("spot_light.quadratic", 0.032f);
		lighting_shader.SetFloat("spot_light.cutoff", glm::cos(glm::radians(12.5f)));
		lighting_shader.SetFloat("spot_light.outer_cutoff", glm::cos(glm::radians(15.0f)));

		//set values of material vectors in material struct
		//lighting_shader.SetVec3("material.ambient", glm::vec3(1.0f, 0.5f, 0.31f));
		//lighting_shader.SetVec3("material.diffuse", glm::vec3(1.0f, 0.5f, 0.31f));
		lighting_shader.SetInt("material.diffuse", 0);
		lighting_shader.SetInt("material.specular", 1);
		lighting_shader.SetFloat("material.shininess", 32.0f);

		//latest camera stuff
		//pass projection matrix to shader, can change every frame
		glm::mat4 projection = glm::perspective(glm::radians(main_camera.zoom), (float)screen_width / (float)screen_height, 0.1f, 100.0f); //fov, aspect ratio, near, far
		lighting_shader.SetMat4("projection", projection); //currently projection matrix is set each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
		//camera/view transformation
		glm::mat4 view = main_camera.GetViewMatrix();
		lighting_shader.SetMat4("view", view);

		//world transformation
		glm::mat4 model = glm::mat4(1.0f);
		lighting_shader.SetMat4("model", model);

		//bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuse_map);

		//bind diffuse map
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specular_map);

		//render multiple cubes
		glBindVertexArray(VAO);
		for (unsigned int i = 0; i < 10; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cube_positions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			lighting_shader.SetMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}


		//render cube
		//glBindVertexArray(VAO);
		//glDrawArrays(GL_TRIANGLES, 0, 36);

		//then draw "lamp" object
		light_cube_shader.Use();
		light_cube_shader.SetMat4("projection", projection);
		light_cube_shader.SetMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, light_pos);
		model = glm::scale(model, glm::vec3(0.2f)); //a smaller cube
		light_cube_shader.SetMat4("model", model);

		glBindVertexArray(light_cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//glBindVertexArray(0); //no need to unbind everytime
		//check and poll IO events and swap front and back buffer

		//imgui window creation, name and features (text, checkbox etc.)
		ImGui::Begin("Test Window");
		ImGui::Text("Hello GoofyGL");
		ImGui::SliderFloat("Light Linear", &light_linear, 0.0f, 2.0f);
		ImGui::SliderFloat("Light Quadratic", &light_quadratic, 0.0f, 2.0f);
		ImGui::Text("Linear: %.3f", light_linear);
		ImGui::Text("Quadratic: %.3f", light_quadratic);
		ImGui::SliderFloat3("Light Position", &light_pos[0], -10.0f, 100.0f);
		ImGui::End();

		//export variables to shader
		//glUseProgram(shader_program);
		//glUniform1f(glGetUniformLocation(shader_program, "size"), size);
		//glUniform4f(glGetUniformLocation(shader_program, "color"), color[0], color[1], color[2], color[3]);

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
	glDeleteVertexArrays(1, &light_cubeVAO);
	glDeleteBuffers(1, &VBO);
	//glDeleteProgram(shader_program);

	//glfwterminate clearing all previously allocated glfw resources
	glfwTerminate();
}

//glfw, whenever the window size changed this callback function executes
void GoofyGL::FramebufferSizeCallback(GLFWwindow* _window, int _width, int _height)
{
	glViewport(0, 0, _width, _height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void GoofyGL::ProcessInput(GLFWwindow* window)
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


void GoofyGL::MouseCallback(GLFWwindow* _window, double _xpos, double _ypos)
{
	//GoofyGL* GoofyGL = reinterpret_cast<GoofyGL*>(glfwGetWindowUserPointer(_window));
	//GoofyGL goofy;

	GoofyGL* GoofyGLInstance = static_cast<GoofyGL*>(glfwGetWindowUserPointer(_window));
	if (!GoofyGLInstance->cursor_enabled)
	{
		float x_pos = static_cast<float>(_xpos);
		float y_pos = static_cast<float>(_ypos);

		if (GoofyGLInstance->first_mouse)
		{
			GoofyGLInstance->last_x = x_pos;
			GoofyGLInstance->last_y = y_pos;
			GoofyGLInstance->first_mouse = false;
		}

		float x_offset = x_pos - GoofyGLInstance->last_x;
		float y_offset = GoofyGLInstance->last_y - y_pos; //reversed since y-coordinates range from bottom to top

		GoofyGLInstance->last_x = x_pos;
		GoofyGLInstance->last_y = y_pos;

		GoofyGLInstance->main_camera.ProcessMouseMovement(x_offset, y_offset);
	}
}

void GoofyGL::ScrollCallback(GLFWwindow* _window, double _xoffset, double _yoffset)
{
	GoofyGL* GoofyGLInstance = static_cast<GoofyGL*>(glfwGetWindowUserPointer(_window));
	if (!GoofyGLInstance->cursor_enabled)
	{
		GoofyGLInstance->main_camera.ProcessMouseScroll(static_cast<float>(_yoffset));
	}
}

unsigned int GoofyGL::LoadTexture(char const * path)
{
	unsigned int texture_id;
	glGenTextures(1, &texture_id);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return texture_id;
}
