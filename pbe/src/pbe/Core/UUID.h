#pragma once

#include "Base.h"
#include <xhash>

namespace pbe {

	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID& other) = default;

		bool operator==(const UUID& rhs) const { return m_UUID == rhs.m_UUID; }
		bool operator!=(const UUID& rhs) const { return !(*this == rhs); }

		operator const uint64_t () const { return m_UUID; }

		bool Valid() const;
	private:
		uint64_t m_UUID;
	};

	const UUID UUID_INVALID = std::numeric_limits<uint64_t>::max();

	UUID UUIDGet();
	void UUIDAdd(uint64 uuid);
	void UUIDFree(UUID uuid);

}

namespace std {

	template <>
	struct hash<pbe::UUID>
	{
		std::size_t operator()(const pbe::UUID& uuid) const
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};
}
