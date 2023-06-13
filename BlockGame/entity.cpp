#include "entity.h"

#include <memory>

#include "logging.h"

Entity::Entity()
{
	id_ = Entity::NewId();
	components_ = std::unordered_map<std::string, Component*>();
}

void Entity::Start()
{
	
}

void Entity::Update()
{
	for (auto componentPair : components_)
	{
		Component* component = componentPair.second;

		if (component) {
			component->Update();
		}
	}
}

uint32_t const& Entity::GetId()
{
	return id_;
}

uint32_t Entity::NewId()
{
	static int currentId = 0;
	return currentId++;
}

void Entity::AddComponent(std::string name, Component* component)
{
	if (component) {
		component->Start();
		components_.insert(std::make_pair(name, component));
	} else {
		LOG("Cannot add component to entity %d since it refers to a null pointer.\n", id_);
	}
}

Component* Entity::GetComponentByName(std::string name)
{
	Component* component = components_.at(name);

	if (component)
	{
		return component;
	}

	LOG("Component %s either doesn't exist or was a nullptr\n", name.c_str());
	return nullptr;
}