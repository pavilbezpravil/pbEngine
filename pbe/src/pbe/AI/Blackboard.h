#pragma once
#include <variant>
#include <string>
#include <unordered_map>

#include "pbe/Core/Math/Common.h"

namespace pbe
{
	namespace AI
	{
		class Controller;

		struct BlackboardValue : std::variant<int, float, Vec2, Vec3, Vec4,std::string>
		{
			typedef std::variant<int, float, Vec2, Vec3, Vec4, std::string> Base;

			enum class Type
			{
				Int,
				Float,
				Vec2,
				Vec3,
				Vec4,
				String,
			};

			static std::string TypeToString(Type type);

			Type GetType() const;

			template<typename T>
			T& As()
			{
				return std::get<T>(*this);
			}

			template<typename T>
			const T& As() const
			{
				return std::get<T>(*this);
			}

			template<typename T>
			BlackboardValue& operator=(const T& o) {
				Base::operator=(o);
				return *this;
			}

			template<typename T>
			BlackboardValue& operator=(T&& o) {
				Base::operator=(std::move(o));
				return *this;
			}
		};

		
		class Blackboard
		{
		public:
			BlackboardValue& GetValue(const std::string& key);
			const BlackboardValue& GetValue(const std::string& key) const;

			void SetValue(const std::string& key, const BlackboardValue& value);
			void SetValue(const std::string& key, BlackboardValue&& value);

			// todo: tmp
			Controller* aiController = NULL;
			
			using Ptr = std::shared_ptr<Blackboard>;
			
		private:
			std::unordered_map<std::string, BlackboardValue> map;
		};
	}
}