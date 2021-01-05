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
			case Int: return "Int";
			case Float: return "Float";
			case String: return "String";
			default: *((int*)0);
			}
		}

		BlackboardValue::Type BlackboardValue::GetType() const
		{
			auto visitor = make_visitor(
				[&](const int i)
				{
					return Type::Int;
				},
				[&](const float f)
				{
					return Type::Float;
				},
					[&](const std::string& s)
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
