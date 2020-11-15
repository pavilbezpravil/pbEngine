#include "pch.h"
#include "ScriptWrappers.h"

#include "pbe/Core/Math/Noise.h"

#include "pbe/Scene/Scene.h"
#include "pbe/Scene/Entity.h"
#include "pbe/Scene/Components.h"

#include <glm/gtc/type_ptr.hpp>

#include "pbe/Core/Input.h"

#define SOL_ALL_SAFETIES_ON 1
#include <glm/gtx/quaternion.hpp>
#include <sol/sol.hpp>

#include "pbe/Core/Application.h"

namespace pbe {

	extern sol::state g_luaState;

	namespace Script {

		void RegisterGameFunction()
		{
			HZ_CORE_INFO("    RegisterGameFunction");

			auto& game = g_luaState.create_table("game");
			game["GetTotalTime"] = [&]() { return Application::Get().GetTime(); };
		}

		void RegisterMathFunction()
		{
			HZ_CORE_INFO("    RegisterMathFunction");

			g_luaState.new_usertype<Vec3>("Vec3",
				sol::constructors<Vec3(), Vec3(float, float, float)>(),
				"x", &Vec3::x,
				"y", &Vec3::y,
				"z", &Vec3::z,
				"cross", [] (const Vec3& l, const Vec3& r) { return glm::cross(l, r); },
				"dot", [] (const Vec3& l, const Vec3& r) { return glm::dot(l, r); },
				"normalize", [] (const Vec3& l) { return glm::normalize(l); },
				"length", [] (const Vec3& l) { return glm::length(l); },
				"distance", [](const Vec3& l, const Vec3& r) { return glm::distance(l, r); },
				sol::meta_function::addition, [](const Vec3& l, const Vec3& r) { return l + r; },
				sol::meta_function::subtraction, [](const Vec3& l, const Vec3& r) { return l - r; },
				sol::meta_function::multiplication, [](const Vec3& l, const Vec3& r) { return l * r; },
				sol::meta_function::multiplication, [](const Vec3& l, float r) { return l * r; },
				sol::meta_function::to_string, [](const Vec3& l) { return std::string("{") + std::to_string(l.x) + ", " + std::to_string(l.y) + ", " + std::to_string(l.z) + "}"; }
				);

			// g_luaState.new_usertype<Quat>("Quat",
			// 	sol::constructors<Quat(), Quat(float, float, float, float)>(),
			// 	"fromMatrix", [](const Vec3& from, const Vec3& to) { return glm::quat(from, to); },
			// 	"angleAxis", [](float angle, const Vec3& axis) { return glm::angleAxis(angle, axis); },
			// 	"w", &Quat::w,
			// 	"x", &Quat::x,
			// 	"y", &Quat::y,
			// 	"z", &Quat::z,
			// 	"eulerAngles", [](const Quat& self) { return eulerAngles(self); },
			// 	"angle", [](const Quat& self) { return glm::angle(self); },
			// 	"toMat4", [](const Quat& self) { return glm::toMat4(self); },
			// 	"dot", [](const Quat& self, const Quat& r) { return glm::dot(self, r); },
			// 	"normalize", [](const Quat& self) { return glm::normalize(self); },
			// 	"inverse", [](const Quat& self) { return glm::inverse(self); },
			// 	"lerp", [](const Quat& self, const Quat& r, float t) { return glm::mix(self, r, t); },
			// 	"slerp", [](const Quat& self, const Quat& r, float t) { return glm::slerp(self, r, t); },
			// 	sol::meta_function::addition, [](const Vec3& l, const Vec3& r) { return l + r; },
			// 	sol::meta_function::subtraction, [](const Vec3& l, const Vec3& r) { return l - r; },
			// 	sol::meta_function::multiplication, [](const Quat& self, const Vec3& v) { return self * v; },
			// 	sol::meta_function::multiplication, [](const Quat& self, const Quat& r) { return self * r; },
			// 	sol::meta_function::to_string, [](const Quat& l) { return std::string("{") + std::to_string(l.w) + ", " + std::to_string(l.x) + ", " + std::to_string(l.y) + ", " + std::to_string(l.z) + "}"; }
			// );
		}

		void RegisterComponent()
		{
			HZ_CORE_INFO("    RegisterComponent");

			g_luaState.new_usertype<LightComponentBase>("LightComponentBaseComponent",
				"Enable", &LightComponentBase::Enable,
				"CastShadow", &LightComponentBase::CastShadow,
				"Color", &LightComponentBase::Color,
				"Multiplier", &LightComponentBase::Multiplier
				);

			g_luaState.new_usertype<DirectionLightComponent>("DirectionLightComponent",
				sol::base_classes, sol::bases<LightComponentBase>()
				);

			g_luaState.new_usertype<PointLightComponent>("PointLightComponent",
				sol::base_classes, sol::bases<LightComponentBase>(),
				"Radius", &PointLightComponent::Radius
				);

			g_luaState.new_usertype<SpotLightComponent>("SpotLightComponent",
				sol::base_classes, sol::bases<LightComponentBase>(),
				"Radius", &SpotLightComponent::Radius,
				"CutOff", &SpotLightComponent::CutOff
				);

			g_luaState.new_usertype<TransformComponent>("TransformComponent",
				"Translation", &TransformComponent::Translation,
				"Rotation", &TransformComponent::Rotation,
				"Scale", &TransformComponent::Scale,
				"Move", [](TransformComponent& self, const Vec3& v) { self.Translation += v; }
				);
		}

		template <typename T>
		sol::object GetComponentSafe(Entity& e, const char* name, sol::state_view& lua)
		{
			if (e.HasComponent<T>()) {
				return sol::make_object(lua, &e.GetComponent<T>());
			}
			return sol::make_object(lua, sol::lua_nil);
		};

		void RegisterEntity()
		{
			HZ_CORE_INFO("    RegisterEntity");

			g_luaState.new_usertype<Entity>("Entity",
				"getComponent", [](Entity& e, const char* name, sol::this_state s)
				{
					sol::state_view lua(s);

					if (!strcmp(name, TransformComponent::GetName())) {
						return GetComponentSafe<TransformComponent>(e, name , lua);
					} else if (!strcmp(name, DirectionLightComponent::GetName())) {
						return GetComponentSafe<DirectionLightComponent>(e, name, lua);
					} else if (!strcmp(name, PointLightComponent::GetName())) {
						return GetComponentSafe<PointLightComponent>(e, name, lua);
					} else if (!strcmp(name, SpotLightComponent::GetName())) {
						return GetComponentSafe<SpotLightComponent>(e, name, lua);
					} else if (!strcmp(name, MeshComponent::GetName())) {
						return GetComponentSafe<MeshComponent>(e, name, lua);
					} else if (!strcmp(name, TagComponent::GetName())) {
						return GetComponentSafe<TagComponent>(e, name, lua);
					} else if (!strcmp(name, CameraComponent::GetName())) {
						return GetComponentSafe<CameraComponent>(e, name, lua);
					}
					return sol::make_object(lua, sol::lua_nil);
				},
				"getUUID", [](const Entity& e) { return e.GetUUID(); },
				sol::meta_function::to_string, [](const Entity& e) { return std::string("Entity {") + std::to_string(e.GetSceneUUID()) + "}"; }
			);
		}
	}
}
