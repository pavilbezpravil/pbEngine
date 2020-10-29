#pragma once

#ifdef HZ_PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <memory>
#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include <functional>
#include <algorithm>

#include <fstream>

#include <pbe/Core/Base.h>
#include <pbe/Core/Log.h>
#include <pbe/Core/Events/Event.h>

// Math
#include <pbe/Core/Math/Mat4.h>

// dx12
#include "pbe/Renderer/pbe_dx12.h"