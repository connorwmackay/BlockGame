#include "meshComponent.h"

MeshComponent::MeshComponent()
{}

MeshComponent::MeshComponent(Mesh& mesh)
{
	SetMesh(mesh);
}

void MeshComponent::SetMesh(Mesh& mesh)
{
	mesh_ = mesh;
}

void MeshComponent::Draw(glm::mat4 const& model, glm::mat4 const& view, glm::mat4 const& projection)
{
	mesh_.Draw(model, view, projection);
}