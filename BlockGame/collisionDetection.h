#pragma once

#include <vector>
#include <glm/glm.hpp>

class World;

namespace CollisionDetection {

	struct CollisionBox {
		glm::vec3 origin;
		glm::vec3 size;
	};

	struct CollisionBoxBounds {
		glm::vec3 min;
		glm::vec3 max;
	};

	struct RaycastHit
	{
		CollisionBox hit;
	};

	CollisionBoxBounds getCollisionBoxBounds(CollisionBox box);

	bool isOverlapping(CollisionBox box1, CollisionBox box2);
	bool isPointInsideBox(CollisionBox box, glm::vec3 point);
}
