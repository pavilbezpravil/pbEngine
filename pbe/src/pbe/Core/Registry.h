#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>

namespace pbe
{
	namespace core
	{

		////////////////////////////////////////////////////////////////////////////////////////////////
		// Portions of this code adapted from https://stackoverflow.com/a/42191109/201787
		////////////////////////////////////////////////////////////////////////////////////////////////
		template<class T, class F>
		struct InvokeApplyOnConstruction
		{
			InvokeApplyOnConstruction()
			{
				F::template Register<T>();
			}
		};

		////////////////////////////////////////////////////////////////////////////////////////////////
		template<class T, class F>
		struct RegistrationInvoker
		{
			static const InvokeApplyOnConstruction<T, F> _registrationInvoker;
		};

		template<class T, class F>
		const InvokeApplyOnConstruction<T, F> RegistrationInvoker<T, F>::_registrationInvoker
			= InvokeApplyOnConstruction<T, F>();


		////////////////////////////////////////////////////////////////////////////////////////////////
		// Classic Registry object allowing runtime registration
		////////////////////////////////////////////////////////////////////////////////////////////////
		template<class AbstractProduct>
		struct Registry
		{
			using Product = std::shared_ptr<AbstractProduct>;
			using Creator = std::function<Product()>;

			static Registry& Instance()
			{
				static Registry f;
				return f;
			}

			void Register(std::size_t id, Creator creator)
			{
				assert(_map.find(id) == std::end(_map));
				_map[id] = std::move(creator);
			}

			void RegisterWithName(std::size_t id, const char* name, Creator creator)
			{
				assert(_map.find(id) == std::end(_map));
				_map[id] = std::move(creator);

				assert(_nameToID.find(name) == std::end(_nameToID));
				_nameToID[name] = id;

				assert(_idToName.find(id) == std::end(_idToName));
				_idToName[id] = name;
			}

			Product CreateByID(const std::size_t id) const
			{
				const auto entry = _map.find(id);
				assert(entry != std::end(_map));
				return entry->second();
			}

			Product CreateByName(const char* name) const
			{
				return CreateByID(GetRegistryIDByName(name));
			}

			const std::unordered_map<std::size_t, Creator>& GetRegistryMap()
			{
				return _map;
			}

			std::size_t GetRegistryIDByName(const char* name) const
			{
				const auto entry = _nameToID.find(name);
				assert(entry != std::end(_nameToID));
				return entry->second;
			}

			const char* GetRegistryIDByName(std::size_t id) const
			{
				const auto entry = _idToName.find(id);
				assert(entry != std::end(_idToName));
				return entry->second.c_str();
			}

		private:
			Registry() = default;
			Registry(const Registry&) = delete;

			std::unordered_map<std::size_t, Creator> _map;
			std::unordered_map<std::string, size_t> _nameToID;
			std::unordered_map<size_t, std::string> _idToName;
		};


		////////////////////////////////////////////////////////////////////////////////////////////////
		// This functor gets invoked by the generic auto-registration classes.
		template<class Base>
		struct RegisterInRegistry
		{
			// template<class T>
			// static void Register()
			// {
			//     Registry<Base>::Instance().Register(T::registryID, [] { return std::make_unique<T>(); });
			// }

			template<class T>
			static void Register()
			{
				Registry<Base>::Instance().RegisterWithName(T::registryID, T::GetRegistryName(), [] { return std::make_shared<T>(); });
			}
		};

		////////////////////////////////////////////////////////////////////////////////////////////////
		template<class Base, class Derived>
		struct AutomaticRegistryRegistration : RegistrationInvoker<Derived, RegisterInRegistry<Base>>
		{
			static const std::size_t registryID;
		};

		// Create a unique ID by using the address of a static var. This also keeps the var and thus our invocation from being optimized away.
		template<class Base, class Derived>
		const std::size_t AutomaticRegistryRegistration<Base, Derived>::registryID
			= reinterpret_cast<std::size_t>(&RegistrationInvoker<Derived, RegisterInRegistry<Base>>::_registrationInvoker);


		#define REGISTRY_TYPE(Type) \
			static const char* GetRegistryName() { return STRINGIFY(Type); } \
			void __FuckCPP() { Type::registryID; }
	}
}
