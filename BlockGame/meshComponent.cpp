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

void MeshComponent::Draw(glm::mat4 model, glm::mat4 view, glm::mat4 projection)
{
	mesh_.Draw(model, view, projection);
}