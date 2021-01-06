#include "pch.h"
#include "Blackboard.h"

namespace pbe
{
	namespace AI
	{

		template <class... Fs>
		struct overload;

		template <class F0, class... Frest>
		struct overload<F0, Frest...> : F0, overload<Frest...>
		{
			overload(F0 f0, Frest... rest) : F0(f0), overload<Frest...>(rest...) {}

			using F0::operator();
			using overload<Frest...>::operator();
		};

		template <class F0>
		struct overload<F0> : F0
		{
			overload(F0 f0) : F0(f0) {}

			using F0::operator();
		};

		template <class... Fs>
		auto make_visitor(Fs... fs)
		{
			return overload<Fs...>(fs...);
		}


		std::string BlackboardValue::TypeToString(Type type)
		{
			switch (type)
			{
			case Type::Int: return "Int";
			case Type::Float: return "Float";
			case Type::Vec2: return "Vec2";
			case Type::Vec3: return "Vec3";
			case Type::Vec4: return "Vec4";
			case Type::String: return "String";
			default: *((int*)0);
			}
		}

		BlackboardValue::Type BlackboardValue::GetType() const
		{
			auto visitor = make_visitor(
				[&](const int)
				{
					return Type::Int;
				},
				[&](const float)
				{
					return Type::Float;
				},
				[&](const Vec2&)
				{
					return Type::Vec2;
				},
				[&](const Vec3&)
				{
					return Type::Vec3;
				},
				[&](const Vec4&)
				{
					return Type::Vec4;
				},
				[&](const std::string&)
				{
					return Type::String;
				}
				);
			return std::visit(visitor, *this);
		}

		BlackboardValue& Blackboard::GetValue(const std::string& key)
		{
			auto it = map.find(key);
			if (it == map.end()) {
				map[key] = {};
				it = map.find(key);
			}
			return it->second;
		}

		const BlackboardValue& Blackboard::GetValue(const std::string& key) const
		{
			auto it = map.find(key);
			HZ_CORE_ASSERT(it != map.end());
			return it->second;
		}

		void Blackboard::SetValue(const std::string& key, const BlackboardValue& value)
		{
			map[key] = value;
		}

		void Blackboard::SetValue(const std::string& key, BlackboardValue&& value)
		{
			map[key] = std::move(value);
		}
	}
}
