#pragma once

#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Color : glm::vec4 {
	Color(float r, float g, float b) : glm::vec4(r, g, b, 1.f) {}
	Color(float r, float g, float b, float a) : glm::vec4(r, g, b, a) {}
	Color(const glm::vec3& v) : glm::vec4(v.x, v.y, v.z, 1.f) {}
	Color(const glm::vec4& v) : glm::vec4(v.x, v.y, v.z, v.w) {}

	// const float* GetPtr() const { return glm::value_ptr(*this); }
	const float* GetPtr() const { return (const float*)this; }
	float* GetPtr() { return (float*)this; }

	static Color FromVec4(const glm::vec4& vec4) { return Color(vec4); }
};

const Color Color_Red = Color(1, 0, 0);
const Color Color_Green = Color(0, 1, 0);
const Color Color_Blue = Color(0, 0, 1);
