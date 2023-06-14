#include "meshComponent.h"

#include "entity.h"
#include "transformComponent.h"

MeshComponent::MeshComponent()
{}

MeshComponent::MeshComponent(Entity* owner, Mesh& mesh)
	: Component(owner)
{
	SetMesh(mesh);
}

void MeshComponent::SetMesh(Mesh& mesh)
{
	mesh_ = mesh;
}

void MeshComponent::Start()
{
}

void MeshComponent::Draw()
{
	mesh_.Draw();
}

void MeshComponent::SetModel(glm::mat4 const& model)
{
	mesh_.SetModel(model);
}

void MeshComponent::SetView(glm::mat4 const& view)
{
	mesh_.SetView(view);
}
void MeshComponent::SetProjection(glm::mat4 const& projection)
{
	mesh_.SetProjection(projection);
}