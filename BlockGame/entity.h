#pragma once
#include <cstdint>
#include <memory>
#include <vector>

#include "component.h"

class Entity
{
	uint32_t id_;
	std::vector<std::unique_ptr<Component>> components_;
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

	void AddComponent(std::unique_ptr<Component> component);

	static uint32_t NewId();
};