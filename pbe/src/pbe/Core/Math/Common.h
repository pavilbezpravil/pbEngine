#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;

using Mat2 = glm::mat2;
using Mat3 = glm::mat3;
using Mat4 = glm::mat4;

using Quat = glm::quat;

std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition(const glm::mat4& transform);

namespace Math
{
	template <typename T> __forceinline T AlignUpWithMask(T value, size_t mask) {
		return (T)(((size_t)value + mask) & ~mask);
	}

	template <typename T> __forceinline T AlignDownWithMask(T value, size_t mask) {
		return (T)((size_t)value & ~mask);
	}

	template <typename T> __forceinline T AlignUp(T value, size_t alignment) {
		return AlignUpWithMask(value, alignment - 1);
	}

	template <typename T> __forceinline T AlignDown(T value, size_t alignment) {
		return AlignDownWithMask(value, alignment - 1);
	}

	template <typename T> __forceinline bool IsAligned(T value, size_t alignment) {
		return 0 == ((size_t)value & (alignment - 1));
	}

	template <typename T> __forceinline T DivideByMultiple(T value, size_t alignment) {
		return (T)((value + alignment - 1) / alignment);
	}

	template <typename T> __forceinline bool IsPowerOfTwo(T value) {
		return 0 == (value & (value - 1));
	}

	template <typename T> __forceinline bool IsDivisible(T value, T divisor) {
		return (value / divisor) * divisor == value;
	}

	__forceinline uint8_t Log2(uint64_t value) {
		unsigned long mssb; // most significant set bit
		unsigned long lssb; // least significant set bit

		// If perfect power of two (only one set bit), return index of bit.  Otherwise round up
		// fractional log by adding 1 to most signicant set bit's index.
		if (_BitScanReverse64(&mssb, value) > 0 && _BitScanForward64(&lssb, value) > 0)
			return uint8_t(mssb + (mssb == lssb ? 0 : 1));
		else
			return 0;
	}

	template <typename T> __forceinline T AlignPowerOfTwo(T value) {
		return value == 0 ? 0 : 1 << Log2(value);
	}

}
