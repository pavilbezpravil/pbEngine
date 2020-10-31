#pragma once

namespace pbe
{
	template<typename T>
	class Singleton
	{
	private:
		inline static T* _inst = nullptr;
	public:
		static void Instantiate() {
			_inst = new T();
		}

		static void Deinstantiate() {
			delete _inst;
		}
		
		static T& Get() {
			return *_inst;
		}
	};
}
