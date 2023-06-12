#include "entity.h"

#include <memory>

Entity::Entity()
{
	id_ = Entity::NewId();
	components_ = std::vector<std::unique_ptr<Component>>();
}

void Entity::Start()
{
	
}

void Entity::Update()
{
	for (int i=0; i < components_.size(); i++)
	{
		std::unique_ptr<Component> component = std::move(components_[i]);

		if (component) {
			component->Update();
		}

		components_[i] = std::move(component);
	}
}

uint32_t Entity::NewId()
{
	static int currentId = 0;
	return currentId++;
}

void Entity::AddComponent(std::unique_ptr<Component> component)
{
	if (component) {
		component->Start();
		components_.push_back(std::move(component));
	}
}