#include "entity.h"

#include <memory>

#include "logging.h"
#include "world.h"

Entity::Entity()
{
	id_ = Entity::NewId();
	components_ = std::unordered_map<std::string, Component*>();
	children_ = std::vector<Entity*>();
	parent_ = nullptr;
}

void Entity::Start()
{
	
}

void Entity::Update(World* world)
{
	Update();
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
	if (components_.find(name) != components_.end()) {
		Component* component = components_.at(name);

		if (component)
		{
			return component;
		}
	}

	LOG("Component %s either doesn't exist or was a nullptr\n", name.c_str());
	return nullptr;
}

void Entity::SetParent(Entity* parent) {
	parent_ = parent;
}

void Entity::AddChild(Entity* child) {
	children_.push_back(child);
}

Entity* Entity::GetParent() {
	return parent_;
}

std::vector<Entity*> Entity::GetChildren() {
	return children_;
}
