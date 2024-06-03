#pragma once
#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
	//the program ID
	unsigned int ID;

	//constructor to read and build shader
	Shader(const char* vertex_path, const char* fragement_path);
	//use/activate the shader
	void Use();
	//utility uniform functions
	void SetBool(const std::string& name, bool value) const;
	void SetInt(const std::string& name, int value) const;
	void SetFloat(const std::string& name, float value) const;
};

#endif