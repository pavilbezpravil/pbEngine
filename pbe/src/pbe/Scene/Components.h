#pragma once

#include "pbe/Core/UUID.h"

namespace pbe {

#define COMPONENT_CLASS_TYPE(ComponentType) \
	ComponentType() = default;\
	ComponentType(const ComponentType& other) = default; \
	static const char* GetName() { return STRINGIFY(ComponentType); }

	struct IDComponent
	{
		UUID ID;

		COMPONENT_CLASS_TYPE(IDComponent)
	};

}
