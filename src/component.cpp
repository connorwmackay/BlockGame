#include "component.h"

#include <cassert>

#include "entity.h"
#include "logging.h"

Component::Component()
{}

Component::Component(Entity* owner)
{
	owner_ = owner;

	if (!owner)
	{
		LOG("The pointer to the owner of this component was a nullptr\n");
	}
}

void Component::Start()
{}

void Component::Update()
{}

Entity* Component::GetOwner()
{
#ifdef _DEBUG
	/*
	 * Cause an exception if the owner was nullptr
	 */
	assert(owner_ != nullptr);
#endif
	return owner_;
}

void Component::SetOwner(Entity* owner)
{
	owner_ = owner;
}