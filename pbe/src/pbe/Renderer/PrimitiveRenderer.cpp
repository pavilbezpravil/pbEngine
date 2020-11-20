#include "pch.h"
#include "PrimitiveRenderer.h"


namespace pbe {

	struct PrimitiveRenderer
	{
		void Init();

		void Shutdown();

		// std::vector<P>
	};

	static PrimitiveRenderer* primRnd = NULL;

	namespace Debug
	{

		void Init() {
			primRnd = new PrimitiveRenderer();
		}

		void Term() {
			delete primRnd;
		}

		void DrawLine(const Vec3& from, const Vec3& to, const Color& color)
		{
		}

	}

}
