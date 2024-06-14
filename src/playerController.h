#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

#include "cameraComponent.h"
#include "collisionBoxComponent.h"
#include "entity.h"
#include "transformComponent.h"
#include "collisionDetection.h"

class World;

class PlayerController : public Entity
{
protected:
	TransformComponent* transformComponent_;
	CollisionBoxComponent* collisionBoxComponent_;
	TransformComponent* cameraTransformComponent_;
	CameraComponent* cameraComponent_;
	GLFWwindow* window_;

	float moveSpeed_;
	float friction_;
	float jumpForce_;
	float jumpHeight_;
	glm::vec3 jumpStartLocation_;
	float sensitivity_;
	float gravity_;
	double prevMouseXPos_, prevMouseYPos_;

	glm::vec3 velocity_;

	bool hasJustPressedJump;
	bool isJumping;

	bool hasJustPressBreakBlock;
	bool hasJustPressedPlaceBlock;
public:
	PlayerController(GLFWwindow* window, glm::vec3 position, glm::vec3 rotation);

	void Start();
	void Update(World* world);

	CollisionDetection::CollisionBox getCollisionBox();
};
