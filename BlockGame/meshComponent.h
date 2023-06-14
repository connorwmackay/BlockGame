#pragma once
#include "component.h"
#include "mesh.h"
#include <glm/glm.hpp>

class MeshComponent : public Component 
{
protected:
	Mesh mesh_;
public:
	MeshComponent();
	MeshComponent(Mesh& mesh);

	void SetMesh(Mesh& mesh);

	void Draw(glm::mat4 const& model, glm::mat4 const& view, glm::mat4 const& projection);
};
