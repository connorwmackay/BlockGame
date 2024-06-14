#include "collisionBoxComponent.h"
#include "world.h"

CollisionBoxComponent::CollisionBoxComponent()
{}

CollisionBoxComponent::CollisionBoxComponent(Entity* owner, const CollisionDetection::CollisionBox& collisionBox)
	: Component(owner)
{
	owner_ = owner;
	collisionBox_ = collisionBox;
}

void CollisionBoxComponent::SetCollisionBox(const CollisionDetection::CollisionBox& collisionBox)
{
	collisionBox_ = collisionBox;
}

const CollisionDetection::CollisionBox& CollisionBoxComponent::GetCollisionBox()
{
	return collisionBox_;
}

bool CollisionBoxComponent::CanMoveTo(World* world, glm::vec3 newTranslation)
{
	CollisionDetection::CollisionBox box{};
	box.size = collisionBox_.size;
	box.origin = { newTranslation.x, newTranslation.y, newTranslation.z};

	CollisionDetection::CollisionBox hitBox{};
	bool willCollide = world->IsCollidingWithWorld(box, hitBox);

	return !willCollide;
}

void CollisionBoxComponent::MoveTo(glm::vec3 newTranslation)
{
	collisionBox_.origin = newTranslation;
}