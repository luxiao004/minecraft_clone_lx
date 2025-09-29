#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

using namespace std;
using namespace glm;

class Camera {
private:
	vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	float Yaw= -90.0f;
	float Pitch= 0.0f;
	float lastX = 400, lastY = 300;
	bool firstMouse = true;
public:
	float fov;
	float speed;
	float sensitivity;
	vec3 cameraPos = vec3(0.0f, 0.0f, 3.0f);
	vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
	Camera(float fov = 45.0f,float speed=2.5f,float sensitivity=0.05f,vec3 cameraPos = vec3(0.0f, 0.0f, 3.0f)) {
		this->fov = fov;
		this->speed = speed;
		this->sensitivity = sensitivity;
		this->cameraPos = cameraPos;
	}
	//¼üÅÌ
	void processInput(GLFWwindow* window) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		float cameraSpeed = speed*deltaTime;
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			cameraPos += cameraSpeed * cameraFront;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cameraPos -= cameraSpeed * cameraFront;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cameraPos += cameraSpeed * normalize(cross(cameraUp, cameraFront));
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cameraPos -= cameraSpeed * normalize(cross(cameraUp, cameraFront));
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			cameraPos.y += cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			cameraPos.y -= cameraSpeed;

	}

	//Êó±ê
	void mousePos(double xpos, double ypos) {
		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}
		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		xoffset *= sensitivity;
		yoffset *= sensitivity;
		
		Yaw += xoffset;
		Pitch += yoffset;
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;
		vec3 front;
		front.x = cos(radians(Yaw)) * cos(radians(Pitch));
		front.y = sin(radians(Pitch));
		front.z = sin(radians(Yaw)) * cos(radians(Pitch));
		cameraFront =normalize(front);
	}
	//»¬ÂÖ
	void mouseScroll(double yoffset) {
		if (fov >= 1.0f && fov <= 85.0f)
			fov = fov - yoffset * 5;
		if (fov <= 15.0f)
			fov = 15.0f;
		if (fov >= 85.0f)
			fov = 85.0f;
	}

	mat4 GetV() {
		return lookAt(cameraPos,cameraPos+cameraFront, cameraUp);
	}
};