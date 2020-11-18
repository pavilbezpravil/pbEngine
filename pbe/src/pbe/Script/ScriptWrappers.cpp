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

			auto vec3 = g_luaState.new_usertype<Vec3>("Vec3",
				sol::constructors<Vec3(), Vec3(float, float, float)>());

			vec3.set("x", &Vec3::x);
			vec3.set("y", &Vec3::y);
			vec3.set("z", &Vec3::z);

			vec3.set("X", sol::property([]() { return Vec3_X; }));
			vec3.set("XNeg", sol::property([]() { return Vec3_XNeg; }));
			vec3.set("Y", sol::property([]() { return Vec3_Y; }));
			vec3.set("YNeg", sol::property([]() { return Vec3_YNeg; }));
			vec3.set("Z", sol::property([]() { return Vec3_Z; }));
			vec3.set("ZNeg", sol::property([]() { return Vec3_ZNeg; }));

			vec3.set("cross", [](const Vec3& l, const Vec3& r) { return glm::cross(l, r); });
			vec3.set("dot", [](const Vec3& l, const Vec3& r) { return glm::dot(l, r); });
			vec3.set("normalize", [](const Vec3& l) { return glm::normalize(l); });
			vec3.set("length", [](const Vec3& l) { return glm::length(l); });
			vec3.set("distance", [](const Vec3& l, const Vec3& r) { return glm::distance(l, r); });

			vec3.set(sol::meta_function::unary_minus, [](const Vec3& self) { return -self; });

			vec3.set(sol::meta_function::addition, [](const Vec3& l, const Vec3& r) { return l + r; });
			vec3.set(sol::meta_function::subtraction, [](const Vec3& l, const Vec3& r) { return l - r; });
			vec3.set(sol::meta_function::multiplication,
				sol::overload(
					[](const Vec3& l, const Vec3& r) { return l * r; },
					[](const Vec3& l, float r) { return l * r; })
				);

			vec3.set(sol::meta_function::to_string, [](const Vec3& l) { return std::string("{") + std::to_string(l.x) + ", " + std::to_string(l.y) + ", " + std::to_string(l.z) + "}"; });

			auto quat = g_luaState.new_usertype<Quat>("Quat",
				sol::constructors<Quat(), Quat(float, float, float, float)>()
			);

			quat.set("w", &Quat::w);
			quat.set("x", &Quat::x);
			quat.set("y", &Quat::y);
			quat.set("z", &Quat::z);

			quat.set("euler", [](const Vec3& euler) { return glm::quat(euler); });
			quat.set("angleAxis", [](float angle, const Vec3& axis) { return glm::angleAxis(angle, axis); });
			quat.set("rotation", [](const Vec3& from, const Vec3& to) { return glm::rotation(from, to); });

			quat.set("angle", [](const Quat& self) { return glm::angle(self); });
			quat.set("axis", [](const Quat& self) { return glm::axis(self); });
			quat.set("eulerAngles", [](const Quat& self) { return glm::eulerAngles(self); });
			quat.set("toMat4", [](const Quat& self) { return glm::toMat4(self); });
			quat.set("dot", [](const Quat& self, const Quat& r) { return glm::dot(self, r); });
			quat.set("normalize", [](const Quat& self) { return glm::normalize(self); });
			quat.set("inverse", [](const Quat& self) { return glm::inverse(self); });
			quat.set("lerp", [](const Quat& self, const Quat& r, float t) { return glm::mix(self, r, t); });
			quat.set("slerp", [](const Quat& self, const Quat& r, float t) { return glm::slerp(self, r, t); });
			
			quat.set(sol::meta_function::addition, [](const Quat& self, const Quat& r) { return self + r; });
			quat.set(sol::meta_function::subtraction, [](const Quat& self, const Quat& r) { return self - r; });
			quat.set(sol::meta_function::multiplication,
				sol::overload(
					[](const Quat& self, const Quat& r) { return self * r; },
					[](const Quat& self, const Vec3& v) { return self * v; },
					[](const Quat& self, const Vec4& v) { return self * v; })
			);
			quat.set(sol::meta_function::to_string, [](const Quat& l) { return std::string("{") + std::to_string(l.w) + ", " + std::to_string(l.x) + ", " + std::to_string(l.y) + ", " + std::to_string(l.z) + "}"; });
		}

		void RegisterComponent()
		{
			HZ_CORE_INFO("    RegisterComponent");

			g_luaState.new_usertype<LightComponentBase>("LightComponentBaseComponent",
				"enable", &LightComponentBase::Enable,
				"castShadow", &LightComponentBase::CastShadow,
				"color", &LightComponentBase::Color,
				"multiplier", &LightComponentBase::Multiplier
				);

			g_luaState.new_usertype<DirectionLightComponent>("DirectionLightComponent",
				sol::base_classes, sol::bases<LightComponentBase>()
				);

			g_luaState.new_usertype<PointLightComponent>("PointLightComponent",
				sol::base_classes, sol::bases<LightComponentBase>(),
				"radius", &PointLightComponent::Radius
				);

			g_luaState.new_usertype<SpotLightComponent>("SpotLightComponent",
				sol::base_classes, sol::bases<LightComponentBase>(),
				"radius", &SpotLightComponent::Radius,
				"cutOff", &SpotLightComponent::CutOff
				);

			auto trans = g_luaState.new_usertype<TransformComponent>("TransformComponent",
				"translation", &TransformComponent::Translation,
				"rotation", &TransformComponent::Rotation,
				"scale", &TransformComponent::Scale,
				"move", [](TransformComponent& self, const Vec3& v) { self.Translation += v; }
				);

			trans.set("forward", sol::property(&TransformComponent::Forward));
			trans.set("up", sol::property(&TransformComponent::Up));
			trans.set("right", sol::property(&TransformComponent::Right));
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

			auto& entity = g_luaState.new_usertype<Entity>("Entity");

			// entity.set();
			entity.set("getComponent", [](Entity& e, const char* name, sol::this_state s)
				{
					sol::state_view lua(s);

					if (!strcmp(name, TransformComponent::GetName())) {
						return GetComponentSafe<TransformComponent>(e, name, lua);
					}
					else if (!strcmp(name, DirectionLightComponent::GetName())) {
						return GetComponentSafe<DirectionLightComponent>(e, name, lua);
					}
					else if (!strcmp(name, PointLightComponent::GetName())) {
						return GetComponentSafe<PointLightComponent>(e, name, lua);
					}
					else if (!strcmp(name, SpotLightComponent::GetName())) {
						return GetComponentSafe<SpotLightComponent>(e, name, lua);
					}
					else if (!strcmp(name, MeshComponent::GetName())) {
						return GetComponentSafe<MeshComponent>(e, name, lua);
					}
					else if (!strcmp(name, TagComponent::GetName())) {
						return GetComponentSafe<TagComponent>(e, name, lua);
					}
					else if (!strcmp(name, CameraComponent::GetName())) {
						return GetComponentSafe<CameraComponent>(e, name, lua);
					}
					return sol::make_object(lua, sol::lua_nil);
				});
			entity.set("getUUID", [](const Entity& e) { return e.GetUUID(); });
			entity.set(sol::meta_function::to_string, [](const Entity& e) { return std::string("Entity {") + std::to_string(e.GetSceneUUID()) + "}"; });
		}
	}
}
