#ifndef WORLD_HPP
#define WORLD_HPP

#include <cstdint>
#include <cstddef>
#include <list>
#include "components.hpp"

struct world
{
	world();
	uint64_t create();
	void kill(uint64_t eid);
	void update(double dt);

	inline size_t count() const
		{ return m_count; }

	position	pos;
	health		hea;
	attack		att;
	movement	mov;
	team		tea;

private:
	std::list<uint64_t> m_freelist;
	uint64_t m_next_index;
	size_t m_count;

	std::vector<uint64_t> m_dead;
};

#endif // WORLD_HPP