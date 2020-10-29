#include "pch.h"
#include "Base.h"

#include "Log.h"

#define PBE_BUILD_ID "v0.1a"

namespace pbe {

	void InitializeCore()
	{
		pbe::Log::Init();

		HZ_CORE_TRACE("pbe {}", PBE_BUILD_ID);
		HZ_CORE_TRACE("Initializing...");
	}

	void ShutdownCore()
	{
		HZ_CORE_TRACE("Shutting down...");
	}

}