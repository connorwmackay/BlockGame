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

	void Draw(glm::mat4 model, glm::mat4 view, glm::mat4 projection);
};
