#include "playerController.h"

#include "cameraComponent.h"
#include "transformComponent.h"

#include "world.h"

PlayerController::PlayerController(GLFWwindow* window, glm::vec3 position, glm::vec3 rotation)
{
	AddComponent("transform", new TransformComponent(this, position, rotation, glm::vec3(1.0f, 1.0f, 1.0f)));
	AddComponent("cameraTransform", new TransformComponent(this, position, rotation, glm::vec3(1.0f, 1.0f, 1.0f)));
	AddComponent("camera", new CameraComponent(this));

	cameraComponent_ = static_cast<CameraComponent*>(GetComponentByName("camera"));
	transformComponent_ = static_cast<TransformComponent*>(GetComponentByName("transform"));
	cameraTransformComponent_ = static_cast<TransformComponent*>(GetComponentByName("cameraTransform"));

	window_ = window;

	moveSpeed_ = 0.03f;
	sensitivity_ = 0.05f;
	gravity_ = -0.0981f;
	jumpForce_ = 0.14f;
	jumpHeight_ = 1.8f;
	hasJustPressedJump = false;
	isJumping = false;

	friction_ = 0.95f;
	velocity_ = glm::vec3(0.0f);
}

void PlayerController::Start()
{
	Entity::Start();
}

void PlayerController::Update(World* world)
{
	Entity::Update(world);

	velocity_ = glm::vec3(0.0f);

	// Check for Forward/Backward, Left/Right Movement
	glm::vec3 currentTranslation = transformComponent_->GetTranslation();

	bool hasPressedMoveKey = false;

	// Handle Input
	if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS)
	{
		velocity_ += transformComponent_->GetForwardVector() * moveSpeed_;
		hasPressedMoveKey = true;
	}
	else if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
	{
		velocity_ += transformComponent_->GetForwardVector() * moveSpeed_ * -1.0f;
		hasPressedMoveKey = true;
	}

	if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS)
	{
		velocity_ += transformComponent_->GetRightVector() * moveSpeed_ * -1.0f;
		hasPressedMoveKey = true;
	}
	else if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS)
	{
		velocity_ += transformComponent_->GetRightVector() * moveSpeed_;
		hasPressedMoveKey = true;
	}

	if (!hasPressedMoveKey) {
		if (velocity_.z <= 0.01f && velocity_.z >= -0.01f) {
			velocity_.z = 0.0f;
		}

		if (velocity_.x <= 0.01f && velocity_.x >= -0.01f) {
			velocity_.x = 0.0f;
		}

		if (velocity_.y <= 0.01f && velocity_.y >= -0.01f) {
			velocity_.y = 0.0f;
		}
	}

	// Add Friction For Smoother Stopping
	velocity_ *= friction_;
	currentTranslation += velocity_;

	glm::vec3 translation = transformComponent_->GetTranslation();
	// Check Forward / Back / Left / Right Collision
	if (CanMoveTo(world, { currentTranslation.x, translation.y, currentTranslation.z })) {
		transformComponent_->SetTranslation({ currentTranslation.x, translation.y, currentTranslation.z });
		cameraTransformComponent_->SetTranslation({ currentTranslation.x, translation.y, currentTranslation.z });
	}

	translation = transformComponent_->GetTranslation();

	if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_RELEASE && hasJustPressedJump)
	{
		hasJustPressedJump = false;
	}

	currentTranslation = transformComponent_->GetTranslation();
	// Handle Gravity
	velocity_.y += gravity_;

	// Jump
	if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS && !hasJustPressedJump)
	{
		hasJustPressedJump = true;
		isJumping = true;
		jumpStartLocation_ = transformComponent_->GetTranslation();
	}

	if (isJumping) {
		if (glm::distance(glm::vec3(0.0f, jumpStartLocation_.y, 0.0f), glm::vec3(0.0f, currentTranslation.y, 0.0f)) < jumpHeight_) {
			velocity_ += transformComponent_->GetUpVector() * jumpForce_;
		}
		else {
			isJumping = false;
		}
	}

	currentTranslation.y += velocity_.y;

	// Up / Down
	if (CanMoveTo(world, { translation.x, currentTranslation.y, translation.z })) {
		transformComponent_->SetTranslation({ translation.x, currentTranslation.y, translation.z });
		cameraTransformComponent_->SetTranslation({ translation.x, currentTranslation.y, translation.z });
	}
	else if (velocity_.y < 0.0f) {
		velocity_.y = 0.0f;
	}

	// Check for Mouse changes
	double mouseXPos, mouseYPos;
	glfwGetCursorPos(window_, &mouseXPos, &mouseYPos);

	double mouseXOffset = mouseXPos - prevMouseXPos_;
	double mouseYOffset = prevMouseYPos_ - mouseYPos;
	mouseXOffset *= sensitivity_;
	mouseYOffset *= sensitivity_;

	transformComponent_->RotateX(mouseXOffset * sensitivity_ * -1.0f);
	cameraTransformComponent_->RotateX(mouseXOffset * sensitivity_ * -1.0f);

	cameraTransformComponent_->RotateY(mouseYOffset * sensitivity_ * -1.0f);

	prevMouseXPos_ = mouseXPos;
	prevMouseYPos_ = mouseYPos;
}

CollisionDetection::CollisionBox PlayerController::getCollisionBox() {
	CollisionDetection::CollisionBox box{};
	glm::vec3 pos = transformComponent_->GetTranslation();
	pos.z += 0.4f;
	pos.y -= 1.4f;

	box.origin = pos;
	box.size = glm::vec3(0.8f, 1.8f, 0.8f);

	return box;
}

bool PlayerController::CanMoveTo(World* world, glm::vec3 newTranslation) {

	CollisionDetection::CollisionBox box = getCollisionBox();
	float boxOffset = 1.4f;
	box.origin = { newTranslation.x, newTranslation.y - boxOffset, newTranslation.z + 0.4f };

	bool willCollide = world->IsCollidingWithWorld(box);

	return !willCollide;
}