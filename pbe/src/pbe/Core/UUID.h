#pragma once

#include "Base.h"
#include <xhash>

namespace pbe {

	// "UUID" (universally unique identifier) or GUID is (usually) a 128-bit integer
	// used to "uniquely" identify information. In pbe, even though we use the term
	// GUID and UUID, at the moment we're simply using a randomly generated 64-bit
	// integer, as the possibility of a clash is low enough for now.
	// This may change in the future.
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID& other);

		bool operator==(const UUID& rhs) const { return m_UUID == rhs.m_UUID; }
		bool operator!=(const UUID& rhs) const { return !(*this == rhs); }

		operator uint64_t () { return m_UUID; }
		operator const uint64_t () const { return m_UUID; }

		bool Valid() const;
	private:
		uint64_t m_UUID;
	};

	const UUID UUID_INVALID = std::numeric_limits<uint64_t>::max();

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
