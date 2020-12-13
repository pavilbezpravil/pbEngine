#include "pch.h"
#include "UUID.h"

#include <random>
#include <unordered_set>

namespace pbe {

	static std::random_device s_RandomDevice;
	static std::mt19937_64 eng(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

	static std::unordered_set<uint64_t> s_GeneretedUUID;

	UUID UUIDGet()
	{
		// uint64 unique = s_UniformDistribution(eng);
		// while (s_GeneretedUUID.find(unique) != s_GeneretedUUID.end()) {
		// 	unique = s_UniformDistribution(eng);
		// }
		// UUIDAdd(unique);
		// return unique;
		return s_UniformDistribution(eng);
	}

	void UUIDAdd(uint64 uuid)
	{
		// HZ_CORE_ASSERT(s_GeneretedUUID.find(uuid) == s_GeneretedUUID.end());
		s_GeneretedUUID.insert(uuid);
	}

	void UUIDFree(UUID uuid)
	{
		// HZ_CORE_ASSERT(s_GeneretedUUID.find(uuid) != s_GeneretedUUID.end());
		s_GeneretedUUID.erase(uuid);
	}

	UUID::UUID() : m_UUID(UUID_INVALID)
	{
	}

	UUID::UUID(uint64_t uuid)
		: m_UUID(uuid)
	{
		// HZ_CORE_ASSERT(uuid == std::numeric_limits<uint64_t>::max() || s_GeneretedUUID.find(uuid) != s_GeneretedUUID.end());
	}

	bool UUID::Valid() const
	{
		return m_UUID != UUID_INVALID;
	}

}
