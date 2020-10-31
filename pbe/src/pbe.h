//
// Note:	this file is to be included in client applications ONLY
//			NEVER include this file anywhere in the engine codebase
//
#pragma once

#include "pbe/Core/Application.h"
#include "pbe/Core/Log.h"
#include "pbe/Core/Input.h"
#include "pbe/Core/Timestep.h"
#include "pbe/Core/Timer.h"

#include "pbe/Core/Events/Event.h"
#include "pbe/Core/Events/ApplicationEvent.h"
#include "pbe/Core/Events/KeyEvent.h"
#include "pbe/Core/Events/MouseEvent.h"

#include "pbe/Core/Math/AABB.h"
#include "pbe/Core/Math/Ray.h"

#include "imgui/imgui.h"

// --- pbe Render API ------------------------------
#include "pbe/Renderer/Renderer.h"
#include "pbe/Renderer/SceneRenderer.h"
#include "pbe/Renderer/VertexBuffer.h"
#include "pbe/Renderer/IndexBuffer.h"
#include "pbe/Renderer/Texture.h"
#include "pbe/Renderer/Shader.h"
#include "pbe/Renderer/Mesh.h"
#include "pbe/Renderer/Camera.h"
// ---------------------------------------------------

// Scenes
#include "pbe/Scene/Entity.h"
#include "pbe/Scene/Scene.h"
#include "pbe/Scene/SceneCamera.h"
#include "pbe/Scene/SceneSerializer.h"
#include "pbe/Scene/Components.h"
