#pragma once

#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Color : glm::vec4 {
	Color(float r, float g, float b) : glm::vec4(r, g, b, 1.f) {}
	Color(float r, float g, float b, float a) : glm::vec4(r, g, b, a) {}
	Color(const glm::vec3& rgb) : glm::vec4(rgb.x, rgb.y, rgb.z, 1.f) {}
	Color(const glm::vec3& rgb, float a) : glm::vec4(rgb.x, rgb.y, rgb.z, a) {}
	Color(const glm::vec4& rgba) : glm::vec4(rgba.x, rgba.y, rgba.z, rgba.w) {}

	// const float* GetPtr() const { return glm::value_ptr(*this); }
	const float* GetPtr() const { return (const float*)this; }
	float* GetPtr() { return (float*)this; }

	static Color FromVec4(const glm::vec4& vec4) { return Color(vec4); }
};

const Color Color_Red = Color(1, 0, 0);
const Color Color_Green = Color(0, 1, 0);
const Color Color_Blue = Color(0, 0, 1);
