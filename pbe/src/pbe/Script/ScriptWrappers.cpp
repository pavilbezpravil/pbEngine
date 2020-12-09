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
#include "pbe/Renderer/RendPrim.h"

namespace pbe {

	namespace Script {

		sol::protected_function_result pbe_script_default_on_error(lua_State* L, sol::protected_function_result pfr) {
			sol::error err = pfr;
			HZ_CORE_WARN("Error while load  script {}", err.what());
			return pfr;
		}

		void LoadSystemScriptInternal(sol::state& g_luaState, const char* path)
		{
			auto res = g_luaState.safe_script_file(path, pbe_script_default_on_error);

			if (!res.valid()) {
				sol::error err = res;
				HZ_CORE_WARN("Error while load system internal script '{}'. /n{}", path, err.what());
			}
		}

		void LoadSystemScripts(sol::state& g_luaState)
		{
			HZ_CORE_INFO("    LoadSystemScripts");

			LoadSystemScriptInternal(g_luaState, "assets/scripts/pbe_sys.lua");
		}

		void RegisterGameFunction(sol::state& g_luaState)
		{
			HZ_CORE_INFO("    RegisterGameFunction");

			auto& game = g_luaState.create_table("game");
			game["GetTotalTime"] = [&]() { return Application::Get().GetTime(); };
		}

		void RegisterMathFunction(sol::state& g_luaState)
		{
			HZ_CORE_INFO("    RegisterMathFunction");

			{
				using VecT = Vec4;
				auto vec3 = g_luaState.new_usertype<VecT>("Vec4",
					sol::constructors<VecT(), VecT(float, float, float, float)>());

				vec3.set("x", &VecT::x);
				vec3.set("y", &VecT::y);
				vec3.set("z", &VecT::z);
				vec3.set("w", &VecT::w);

				vec3.set("X", sol::property([]() { return Vec4_X; }));
				vec3.set("XNeg", sol::property([]() { return Vec4_XNeg; }));
				vec3.set("Y", sol::property([]() { return Vec4_Y; }));
				vec3.set("YNeg", sol::property([]() { return Vec4_YNeg; }));
				vec3.set("Z", sol::property([]() { return Vec4_Z; }));
				vec3.set("ZNeg", sol::property([]() { return Vec4_ZNeg; }));
				vec3.set("W", sol::property([]() { return Vec4_W; }));
				vec3.set("WNeg", sol::property([]() { return Vec4_WNeg; }));

				vec3.set("dot", [](const VecT& l, const VecT& r) { return glm::dot(l, r); });
				vec3.set("normalize", [](const VecT& l) { return glm::normalize(l); });
				vec3.set("length", [](const VecT& l) { return glm::length(l); });
				vec3.set("distance", [](const VecT& l, const VecT& r) { return glm::distance(l, r); });

				vec3.set(sol::meta_function::unary_minus, [](const VecT& self) { return -self; });

				vec3.set(sol::meta_function::addition, [](const VecT& l, const VecT& r) { return l + r; });
				vec3.set(sol::meta_function::subtraction, [](const VecT& l, const VecT& r) { return l - r; });
				vec3.set(sol::meta_function::multiplication,
					sol::overload(
						[](const VecT& l, const VecT& r) { return l * r; },
						[](const VecT& l, float r) { return l * r; })
				);

				vec3.set(sol::meta_function::to_string, [](const VecT& l) { return std::string("{") + std::to_string(l.x) + ", " + std::to_string(l.y) + ", " + std::to_string(l.z) + ", " + std::to_string(l.z) + "}"; });
			}

			{
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
			}

			{
				auto vec2 = g_luaState.new_usertype<Vec2>("Vec2",
					sol::constructors<Vec2(), Vec2(float, float)>());

				vec2.set("x", &Vec2::x);
				vec2.set("y", &Vec2::y);

				vec2.set("X", sol::property([]() { return Vec2_X; }));
				vec2.set("XNeg", sol::property([]() { return Vec2_XNeg; }));
				vec2.set("Y", sol::property([]() { return Vec2_Y; }));
				vec2.set("YNeg", sol::property([]() { return Vec2_YNeg; }));

				vec2.set("dot", [](const Vec2& l, const Vec2& r) { return glm::dot(l, r); });
				vec2.set("normalize", [](const Vec2& l) { return glm::normalize(l); });
				vec2.set("length", [](const Vec2& l) { return glm::length(l); });
				vec2.set("distance", [](const Vec2& l, const Vec2& r) { return glm::distance(l, r); });

				vec2.set(sol::meta_function::unary_minus, [](const Vec2& self) { return -self; });

				vec2.set(sol::meta_function::addition, [](const Vec2& l, const Vec2& r) { return l + r; });
				vec2.set(sol::meta_function::subtraction, [](const Vec2& l, const Vec2& r) { return l - r; });
				vec2.set(sol::meta_function::multiplication,
					sol::overload(
						[](const Vec2& l, const Vec2& r) { return l * r; },
						[](const Vec2& l, float r) { return l * r; })
				);

				vec2.set(sol::meta_function::to_string, [](const Vec2& l) { return std::string("{") + std::to_string(l.x) + ", " + std::to_string(l.y) + "}"; });
			}

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

		void RegisterColor(sol::state& luaState)
		{
			{
				auto color = luaState.new_usertype<Color>("Color",
					sol::constructors<
						  Color(float, float, float)
						, Color(float, float, float, float)
						, Color(const Vec3&)
						, Color(const Vec4&)
					>()
					);

				color.set(sol::base_classes, sol::bases<Vec4>());

				color.set("r", &Color::r);
				color.set("g", &Color::g);
				color.set("b", &Color::b);
				color.set("a", &Color::a);

				color.set("Red", sol::property([]() { return Color_Red; }));
				color.set("Green", sol::property([]() { return Color_Green; }));
				color.set("Blue", sol::property([]() { return Color_Blue; }));

				color.set(sol::meta_function::to_string, [](const Color& l) { return std::string("Color {") + std::to_string(l.r) + ", " + std::to_string(l.g) + ", " + std::to_string(l.b) + ", " + std::to_string(l.w) + "}"; });
			}
		}

		void RegisterComponent(sol::state& g_luaState)
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
				"position", sol::property(&TransformComponent::WorldPosition, [](TransformComponent& tc, const Vec3& position) { tc.UpdatePosition(position, Space::World); }),
				"rotation", sol::property(&TransformComponent::WorldRotation, [](TransformComponent& tc, const Vec3& rotation) { tc.UpdateRotation(rotation, Space::World); }),
				"scale", sol::property(&TransformComponent::WorldScale, [](TransformComponent& tc, const Vec3& scale) { tc.UpdateScale(scale, Space::World); }),
				"localPosition", &TransformComponent::LocalPosition,
				"localRotation", &TransformComponent::LocalRotation,
				"localScale", &TransformComponent::LocalScale
				);

			trans.set("forward", sol::property(&TransformComponent::WorldForward));
			trans.set("up", sol::property(&TransformComponent::WorldUp));
			trans.set("right", sol::property(&TransformComponent::WorldRight));
			trans.set("localForward", sol::property(&TransformComponent::WorldForward));
			trans.set("localUp", sol::property(&TransformComponent::WorldUp));
			trans.set("localRight", sol::property(&TransformComponent::WorldRight));
		}

		template <typename T>
		sol::object GetComponentSafe(Entity& e, const char* name, sol::state_view& lua)
		{
			if (e.HasComponent<T>()) {
				return sol::make_object(lua, &e.GetComponent<T>());
			}
			return sol::make_object(lua, sol::lua_nil);
		};

		void RegisterEntity(sol::state& g_luaState)
		{
			HZ_CORE_INFO("    RegisterEntity");

			using EntityT = LuaEntity;
			
			auto& entity = g_luaState.new_usertype<EntityT>("Entity");

			entity.set("getComponent", [](EntityT& e, const char* name, sol::this_state s)
				{
					sol::state_view lua(s);

					if (!strcmp(name, TransformComponent::GetName())) {
						return GetComponentSafe<TransformComponent>(e, name, lua);
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
				});
			entity.set("getUUID", [](const EntityT& e) { return e.GetUUID(); });
			entity.set(sol::meta_function::to_string, [](const EntityT& e)
			{
				return std::string("Entity {") + std::to_string(e.GetUUID()) + "}";
			});
			entity.set(sol::meta_function::index, &dynamic_object::dynamic_get);
			entity.set(sol::meta_function::new_index, &dynamic_object::dynamic_set);
		}

		void RegisterInput(sol::state& g_luaState)
		{
			HZ_CORE_INFO("    RegisterInput");

			auto keyCodes = g_luaState.create_table("KeyCode");

			// #define REGISTER_KEY_CODE(keyCode) keyCodes.set(STRINGIFY(keyCode), sol::property([]() { return KeyCode::keyCode; }))
			#define REGISTER_KEY_CODE(keyCode) keyCodes[STRINGIFY(keyCode)] =  KeyCode::keyCode;

			REGISTER_KEY_CODE(A);
			REGISTER_KEY_CODE(B);
			REGISTER_KEY_CODE(C);
			REGISTER_KEY_CODE(D);
			REGISTER_KEY_CODE(E);
			REGISTER_KEY_CODE(F);
			REGISTER_KEY_CODE(G);
			REGISTER_KEY_CODE(H);
			REGISTER_KEY_CODE(I);

			REGISTER_KEY_CODE(J);
			REGISTER_KEY_CODE(K);
			REGISTER_KEY_CODE(L);
			REGISTER_KEY_CODE(M);
			REGISTER_KEY_CODE(N);
			REGISTER_KEY_CODE(O);
			REGISTER_KEY_CODE(P);
			REGISTER_KEY_CODE(Q);
			REGISTER_KEY_CODE(R);

			REGISTER_KEY_CODE(S);
			REGISTER_KEY_CODE(T);
			REGISTER_KEY_CODE(U);
			REGISTER_KEY_CODE(V);
			REGISTER_KEY_CODE(W);
			REGISTER_KEY_CODE(X);
			REGISTER_KEY_CODE(Y);
			REGISTER_KEY_CODE(Z);

			REGISTER_KEY_CODE(D1);
			REGISTER_KEY_CODE(D2);
			REGISTER_KEY_CODE(D3);
			REGISTER_KEY_CODE(D4);
			REGISTER_KEY_CODE(D5);
			REGISTER_KEY_CODE(D6);
			REGISTER_KEY_CODE(D7);
			REGISTER_KEY_CODE(D8);
			REGISTER_KEY_CODE(D9);
			REGISTER_KEY_CODE(D0);

			REGISTER_KEY_CODE(Space);
			REGISTER_KEY_CODE(Enter);
			REGISTER_KEY_CODE(LeftShift);
			REGISTER_KEY_CODE(RightShift);
			REGISTER_KEY_CODE(LeftControl);
			REGISTER_KEY_CODE(RightControl);

			auto input = g_luaState.create_table("Input");

			input.set("isKeyPressed", [](const KeyCode& keyCode) { return Input::IsKeyPressed(keyCode); });
			input.set("isMouseButtonPressed", [](int button) { return Input::IsMouseButtonPressed(button); });
			input.set("getMousePosition", []() { auto [x, y] = Input::GetMousePosition(); return Vec2(x, y); });
			input.set("getMouseDelta", []() { auto [x, y] = Input::GetMouseDelta(); return Vec2(x, y); });
		}

		void RegisterRendPrim(sol::state& luaState)
		{
			auto input = luaState.create_table("RendPrim");

			input.set("drawLine", [](const Vec3& from, const Vec3& to, const Vec4& color) { RendPrim::DrawLine(from, to, Color::FromVec4(color)); });
			input.set("drawAABB", [](const Vec3& min, const Vec3& max, const Vec4& color) { RendPrim::DrawAABB(AABB{min, max}, Color::FromVec4(color)); });
			input.set("drawCircle", [](const Vec3& center, const Vec3& normal, float radius, uint nSegments, const Color& color) { RendPrim::DrawCircle(center, normal, radius, nSegments, color); });
			input.set("drawSphere", [](const Vec3& center, float radius, uint nSegments, const Color& color) { RendPrim::DrawSphere(center, radius, nSegments, Color::FromVec4(color)); });
			input.set("drawCone", [](const Vec3& center, const Vec3& forward, float angle, float radius, uint nSegments, const Color& color) { RendPrim::DrawCone(center, forward, angle, radius, nSegments, color); });
		}
	}
}
