#pragma once

class Entity;

class Component
{
protected:
	Entity* owner_;
public:
	Component();
	Component(Entity* owner);
	virtual ~Component() = default;

	virtual void Start();
	virtual void Update();

	void SetOwner(Entity* owner);
	Entity* GetOwner();
};