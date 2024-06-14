#pragma once
#include "component.h"
#include "collisionDetection.h"

class World;

class CollisionBoxComponent : public Component
{
public:
	CollisionBoxComponent();
	CollisionBoxComponent(Entity* owner, const CollisionDetection::CollisionBox& collisionBox);

	void SetCollisionBox(const CollisionDetection::CollisionBox& collisionBox);
	const CollisionDetection::CollisionBox& GetCollisionBox();

	bool CanMoveTo(World* world, glm::vec3 newTranslation);
	void MoveTo(glm::vec3 newTranslation);
protected:
	CollisionDetection::CollisionBox collisionBox_;
};
