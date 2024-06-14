#include "freeFormController.h"

#include "cameraComponent.h"
#include "transformComponent.h"

FreeFormController::FreeFormController(GLFWwindow* window, glm::vec3 position, glm::vec3 rotation)
{
	AddComponent("transform", new TransformComponent(this, position, rotation, glm::vec3(1.0f, 1.0f, 1.0f)));
	AddComponent("camera", new CameraComponent(this));

	cameraComponent_ = static_cast<CameraComponent*>(GetComponentByName("camera"));
	transformComponent_ = static_cast<TransformComponent*>(GetComponentByName("transform"));
	window_ = window;

	moveSpeed_ = 0.05f;
	sensitivity_ = 0.05f;
}

void FreeFormController::Start()
{
}

void FreeFormController::Update(World* world)
{
	// Check for Forward/Backward, Left/Right Movement
	glm::vec3 currentTranslation = transformComponent_->GetTranslation();

	if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS)
	{
		currentTranslation += transformComponent_->GetForwardVector() * moveSpeed_;
	}
	else if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
	{
		currentTranslation += transformComponent_->GetForwardVector() * moveSpeed_ * -1.0f;
	}

	if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS)
	{
		currentTranslation += transformComponent_->GetRightVector() * moveSpeed_ * -1.0f;
	}
	else if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS)
	{
		currentTranslation += transformComponent_->GetRightVector() * moveSpeed_;
	}

	// Check for Going Up/Down
	if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS && glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
	{
		currentTranslation += transformComponent_->GetUpVector() * moveSpeed_;
	}
	else if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS && glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		currentTranslation += transformComponent_->GetUpVector() * moveSpeed_ * -1.0f;
	}

	CollisionDetection::CollisionBox box = getCollisionBox();
	box.origin = { currentTranslation.x, currentTranslation.y - 1.4f, currentTranslation.z };

	CollisionDetection::CollisionBox hitBox{};
	bool willCollide = world->IsCollidingWithWorld(box, hitBox);

	if (!willCollide) {
		transformComponent_->SetTranslation(currentTranslation);
	}

	// Check for Mouse changes
	double mouseXPos, mouseYPos;
	glfwGetCursorPos(window_, &mouseXPos, &mouseYPos);

	double mouseXOffset = mouseXPos - prevMouseXPos_;
	double mouseYOffset = prevMouseYPos_ - mouseYPos;
	mouseXOffset *= sensitivity_;
	mouseYOffset *= sensitivity_;

	transformComponent_->RotateX(mouseXOffset * sensitivity_ * -1.0f);
	transformComponent_->RotateY(mouseYOffset * sensitivity_ * -1.0f);

	prevMouseXPos_ = mouseXPos;
	prevMouseYPos_ = mouseYPos;
}

CollisionDetection::CollisionBox FreeFormController::getCollisionBox() {
	CollisionDetection::CollisionBox box{};
	glm::vec3 pos = transformComponent_->GetTranslation();
	pos.y -= 1.4f;

	box.origin = pos;
	box.size = glm::vec3(0.8f, 1.8f, 0.8f);

	return box;
}