#include "../Header/Camera.h"

Camera::Camera()
{
	
}

Camera::Camera(glm::vec3 _position, glm::vec3 _up, float _yaw, float _pitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movement_speed(base_speed), mouse_sensitivity(base_sensitivity), zoom(base_zoom)
{
	position = _position;
	world_up = _up;
	yaw = _yaw;
	pitch = _pitch;
	UpdateCameraVectors();
}

Camera::Camera(float pos_x, float pos_y, float pos_z, float up_x, float up_y, float up_z, float _yaw, float _pitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movement_speed(base_speed), mouse_sensitivity(base_sensitivity), zoom(base_zoom)
{
	position = glm::vec3(pos_x, pos_y, pos_z);
	world_up = glm::vec3(up_x, up_y, up_z);
	yaw = _yaw;
	pitch = _pitch;
	UpdateCameraVectors();
}

Camera::~Camera()
{
	
}


glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float delta_time)
{
	float velocity = movement_speed * delta_time;
	if (direction == FORWARD)
	{
		position += front * velocity;
	}
	if (direction == BACKWARD)
	{
		position -= front * velocity;
	}
	if (direction == LEFT)
	{
		position -= right * velocity;
	}
	if (direction == RIGHT)
	{
		position += right * velocity;
	}
}

void Camera::ProcessMouseMovement(float x_offset, float y_offset, GLboolean constrain_pitch)
{
	x_offset *= mouse_sensitivity;
	y_offset *= mouse_sensitivity;
	
	yaw += x_offset;
	pitch += y_offset;

	//std::cout << "x_offset: " << x_offset << ", y_offset: " << y_offset << std::endl;

	//if pitch is out of bounds, screen doesnt flip
	if (constrain_pitch)
	{
		if (pitch > 89.0f)
		{
			pitch = 89.0f;
		}
		if (pitch < -89.0f)
		{
			pitch = -89.0f;
		}
	}

	//update front right and up vectors using updated euler angles
	UpdateCameraVectors();

	//std::cout << "Yaw: " << yaw << " Pitch: " << pitch << std::endl;
	//std::cout << "Front: " << front.x << ", " << front.y << ", " << front.z << std::endl;
	//std::cout << "Right: " << right.x << ", " << right.y << ", " << right.z << std::endl;
	//std::cout << "Up: " << up.x << ", " << up.y << ", " << up.z << std::endl;
}

void Camera::ProcessMouseScroll(float y_offset)
{
	zoom -= (float)y_offset;
	if (zoom < 1.0f)
	{
		zoom = 1.0f;
	}
	if (zoom > 120.0f)
	{
		zoom = 120.0f;
	}
}

void Camera::UpdateCameraVectors()
{
	//calculate new front vector
	glm::vec3 Front;
	Front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	Front.y = sin(glm::radians(pitch));
	Front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(Front);
	//recalculate up and right vector
	right = glm::normalize(glm::cross(front, world_up)); //normalise vectors because their length gets closer to 0 the more camera looks up or down resulting in slower movement
	up = glm::normalize(glm::cross(right, front));
}

/*
constexpr float Camera::GetBaseYaw()
{
	return base_yaw;
}

constexpr float Camera::GetBasePitch()
{
	return base_pitch;
}

constexpr float Camera::GetBaseSpeed()
{
	return base_speed;
}

constexpr float Camera::GetBaseSensitivity()
{
	return base_sensitivity;
}

constexpr float Camera::GetBaseZoom()
{
	return base_zoom;
}
*/

