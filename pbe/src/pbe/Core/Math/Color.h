#pragma once

#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Color : glm::vec4 {
	Color(float x, float y, float z, float w) : vec<4, float, glm::defaultp>(x, y, z, w) {}

	// const float* GetPtr() const { return glm::value_ptr(*this); }
	const float* GetPtr() const { return (const float*)this; }
	float* GetPtr() { return (float*)this; }
};
