#pragma once
#include <random>

#include "Common.h"

namespace pbe {

	class Random
	{
	public:

		int Int(int min, int max) {
			std::uniform_int_distribution<int> distribution(min, max);
			return distribution(generator);
		}

		float Float(float min = 0.f, float max = 1.f) {
			std::uniform_real_distribution<float> distribution(min, max);
			return distribution(generator);
		}

		Vec2 InSquare(Vec2 min = Vec2_Zero, Vec2 max = Vec2_One) {
			return {Float(min.x, max.x), Float(min.y, max.y) };
		}
		
	private:
		std::default_random_engine generator;
		
	};
	inline static Random s_Random;

}
