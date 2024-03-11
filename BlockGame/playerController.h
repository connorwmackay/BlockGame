#pragma once
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

#include "cameraComponent.h"
#include "entity.h"
#include "transformComponent.h"
#include "collisionDetection.h"

#include "world.h"

class PlayerController : public Entity
{
protected:
	TransformComponent* transformComponent_;
	TransformComponent* cameraTransformComponent_;
	CameraComponent* cameraComponent_;
	GLFWwindow* window_;

	float moveSpeed_;
	float jumpForce_;
	float sensitivity_;
	float gravity_;
	double prevMouseXPos_, prevMouseYPos_;

	bool hasJustPressedJump;
public:
	PlayerController(GLFWwindow* window, glm::vec3 position, glm::vec3 rotation);

	void Start() override;
	void Update(World* world);

	CollisionDetection::CollisionBox getCollisionBox();
};
