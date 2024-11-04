#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

//fucked but cba changing rn
//options for camera movement
enum Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

//base values
static constexpr float base_yaw = -90.0f;
static constexpr float base_pitch = 0.0f;
static constexpr float base_speed = 2.5f;
static constexpr float base_sensitivity = 0.1f;
static constexpr float base_zoom = 90.0f;

class Camera
{
public:
	//camera attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up;
	//euler angles
	float yaw = 0.0f;
	float pitch = 0.0f;
	//camera options
	float movement_speed = 0.0f;
	float mouse_sensitivity = 0.0f;
	float zoom = 0.0f;
	
	//base constructor
	Camera();
	
	//constructor with vectors
	Camera(glm::vec3 _position, glm::vec3 _up, float _yaw, float _pitch);

	//constructor with scalar values
	Camera(float pos_x, float pos_y, float pos_z, float up_x, float up_y, float up_z, float _yaw, float _pitch);

	~Camera();
	
	//returns view matrix calculated with euler angles and LookAt matrix
	glm::mat4 GetViewMatrix();

	//process input recieved from any keyboard-like input system, accepts input param in the form of camera defined enum
	void ProcessKeyboard(Camera_Movement direction, float delta_time);

	//process input recieved from a mouse input system, expects the offset value in both the x and y direction
	void ProcessMouseMovement(float x_offset, float y_offset, GLboolean constrain_pitch = true);

	//process input recieved from a mouse scroll-wheel event, only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float y_offset);
	
	//constexpr float GetBaseYaw();
	//constexpr float GetBasePitch();
	//constexpr float GetBaseSpeed();
	//constexpr float GetBaseSensitivity();
	//constexpr float GetBaseZoom();
	
private:
	
	//calculates the front vector from the camera's updated euler angles
	void UpdateCameraVectors();

};
#endif
