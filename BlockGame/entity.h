#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "component.h"

class Entity
{
	uint32_t id_;
	std::unordered_map<std::string, Component*> components_;
protected:
public:
	Entity();

	/*
	 * Called once, on the first frame the entity is added.
	 * Can be overriden, however make sure you call the base method at the start.
	 */
	virtual void Start();

	/*
	 * Called every frame.
	 * Can be overriden, however make sure you call the base method at the start.
	 */
	virtual void Update();

	void AddComponent(std::string name, Component* component);
	Component* GetComponentByName(std::string name);

	uint32_t const& GetId();

	static uint32_t NewId();
};