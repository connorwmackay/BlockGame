#include "collisionDetection.h"

namespace CollisionDetection {
	CollisionBoxBounds getCollisionBoxBounds(CollisionBox box) {
		CollisionBoxBounds bounds{};

		bounds.min.x = box.origin.x - (box.size.x / 2);
		bounds.max.x = box.origin.x + (box.size.x / 2);

		bounds.min.y = box.origin.y - (box.size.y / 2);
		bounds.max.y = box.origin.y + (box.size.y / 2);

		bounds.min.z = box.origin.z - (box.size.z / 2);
		bounds.max.z = box.origin.z + (box.size.z / 2);

		return bounds;
	}

	bool isOverlapping(CollisionBox box1, CollisionBox box2) {
		CollisionBoxBounds box1Bounds = getCollisionBoxBounds(box1);
		CollisionBoxBounds box2Bounds = getCollisionBoxBounds(box2);

		bool isOverlapping = box1Bounds.min.x <= box2Bounds.max.x &&
			box1Bounds.max.x >= box2Bounds.min.x &&
			box1Bounds.min.y <= box2Bounds.max.y &&
			box1Bounds.max.y >= box2Bounds.min.y &&
			box1Bounds.min.z <= box2Bounds.max.z &&
			box1Bounds.max.z >= box2Bounds.min.z;

		return isOverlapping;
	}

	bool isPointInsideBox(CollisionBox box, glm::vec3 point) {
		CollisionBoxBounds boxBounds = getCollisionBoxBounds(box);

		bool isInBox = point.x >= boxBounds.min.x &&
			point.x <= boxBounds.max.x &&
			point.y >= boxBounds.min.y &&
			point.y <= boxBounds.max.y &&
			point.z >= boxBounds.min.z &&
			point.z <= boxBounds.max.z;

		return isInBox;
	}
}