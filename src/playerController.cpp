#include "playerController.h"

#include "cameraComponent.h"
#include "transformComponent.h"

#include "world.h"
#include "logging.h"

PlayerController::PlayerController(GLFWwindow* window, glm::vec3 position, glm::vec3 rotation)
{
	AddComponent("transform", new TransformComponent(this, position, rotation, glm::vec3(1.0f, 1.0f, 1.0f)));
	AddComponent("cameraTransform", new TransformComponent(this, position, rotation, glm::vec3(1.0f, 1.0f, 1.0f)));
	AddComponent("camera", new CameraComponent(this));
	AddComponent("collision", new CollisionBoxComponent(this, {
		position,
		glm::vec3(0.5f, 1.9f, 0.5f)
	}));

	cameraComponent_ = static_cast<CameraComponent*>(GetComponentByName("camera"));
	transformComponent_ = static_cast<TransformComponent*>(GetComponentByName("transform"));
	cameraTransformComponent_ = static_cast<TransformComponent*>(GetComponentByName("cameraTransform"));
	collisionBoxComponent_ = static_cast<CollisionBoxComponent*>(GetComponentByName("collision"));

	window_ = window;

	moveSpeed_ = 0.2f;
	sensitivity_ = 0.05f;
	gravity_ = -0.981f;
	jumpForce_ = 1.25f;
	jumpHeight_ = 1.5f;
	hasJustPressedJump = false;
	isJumping = false;
	hasJustPressBreakBlock = false;
	hasJustPressedPlaceBlock = false;

	friction_ = 0.95f;
	velocity_ = glm::vec3(0.0f);

    glfwGetCursorPos(window_, &prevMouseXPos_, &prevMouseYPos_);
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
	if (collisionBoxComponent_->CanMoveTo(world, { currentTranslation.x, translation.y, currentTranslation.z})) {
		transformComponent_->SetTranslation({ currentTranslation.x, translation.y, currentTranslation.z});
		cameraTransformComponent_->SetTranslation({ currentTranslation.x, translation.y, currentTranslation.z});
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
	if (collisionBoxComponent_->CanMoveTo(world, { translation.x, currentTranslation.y, translation.z})) {
		transformComponent_->SetTranslation({ translation.x, currentTranslation.y, translation.z});
		cameraTransformComponent_->SetTranslation({ translation.x, currentTranslation.y, translation.z});
	}
	else if (velocity_.y < 0.0f) {
		velocity_.y = 0.0f;
	}

	// Offset the Camera
	cameraTransformComponent_->SetTranslation({ translation.x, currentTranslation.y + 1.8f, translation.z});

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

	if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		hasJustPressBreakBlock = false;
	}

	if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
	{
		hasJustPressedPlaceBlock = false;
	}

	if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !hasJustPressBreakBlock) {
		hasJustPressBreakBlock = true;

		CollisionDetection::RaycastHit hit{};
		bool wasHitFound = world->PerformRaycast(hit, cameraTransformComponent_->GetTranslation(), cameraTransformComponent_->GetForwardVector(), 4.0f, 0.5f, 8.0f);

		if (wasHitFound)
		{
            LOG("Hit\n");

            // FIXME: Blocks aren't removed at the correct place. Bresenham's line algorithm could be the solution.
            //world->BreakBlock(hit.hit.origin);
		}
		else
		{
			printf("No Hit\n");
		}
	}

	if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !hasJustPressedPlaceBlock)
	{
		hasJustPressedPlaceBlock = true;

		CollisionDetection::RaycastHit hit{};
		bool wasHitFound = world->PerformRaycast(hit, cameraTransformComponent_->GetTranslation(), cameraTransformComponent_->GetForwardVector(), 4.0f, 0.5f, 8.0f);

		if (wasHitFound)
		{
            LOG("Hit\n");

            // FIXME: Blocks aren't placed in the correct place. Bresenham's line algorithm could be the solution.
			//world->PlaceBlock(hit.hit.origin, BLOCK_TYPE_STONE);
		}
		else
		{
            LOG("No Hit\n");
		}
	}
}