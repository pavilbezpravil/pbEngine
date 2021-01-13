#include "pch.h"
#include "Entity.h"

namespace pbe {
	bool Entity::IsValid() const
	{
		return this->operator bool() && !m_Scene->IsPendingForDestroy(*this);
	}
}
