#ifndef PARTICLE_SYSTEM_HPP
#define PARTICLE_SYSTEM_HPP

#include "glm/glm.hpp"
#include "common/array.hpp"

struct particle_system
{
    array<float> e_emission_rates;
    array<glm::vec2> e_positions;

    array<int> p_from;
    array<glm::vec2> p_positions;
    array<glm::vec2> p_velocities;
    array<double> p_lifetimes;

    void create_emitter(const glm::vec2 & position, float emission_rate);

    size_t size();
    int count();

    void tick(double dt);
    void emit(int from, const glm::vec2 & p, const glm::vec2 & v, double l, double dt);
};

#endif // PARTICLE_SYSTEM_HPP
