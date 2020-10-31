#pragma once


#include "Mesh.h"
#include "../../../Types.h"
#include "pbe/Core/Singleton.h"

class ColorBuffer;

namespace pbe {

	class Renderer : public Singleton<Renderer>
	{
	public:

		void Init();
		void Shutdown();

		void Resize(uint width, uint height);

		Ref<ColorBuffer>& GetFinalRT();
	};

}
