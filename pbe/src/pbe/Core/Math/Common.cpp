#include "pch.h"

#include "Common.h"

#include <glm/gtx/matrix_decompose.hpp>

std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition(const glm::mat4& transform)
{
	glm::vec3 scale, translation, skew;
	glm::vec4 perspective;
	glm::quat orientation;
	glm::decompose(transform, scale, orientation, translation, skew, perspective);

	return { translation, orientation, scale };
}
