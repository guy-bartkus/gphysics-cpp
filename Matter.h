#pragma once
#include "Vec2.h"
#include <vector>
#include <SFML/Graphics.hpp>

struct Zone {
	Vec2 dir;
	uint16_t bits = 0;
};

class Matter : public CircleShape {
public:
	Vec2 pos;
	Vec2 vel;
	Vec2 velNorm;
	uint8_t type;
	uint16_t mass;
	uint8_t zoneCount;
	std::vector<Zone> zones;
	sf::Color color;

	static const uint8_t s_radius = 30;
	static const uint16_t s_mass = 10;
public:
	Matter();
	Matter(uint8_t type, float x, float y, uint8_t zoneCount);

	void calcZones(bool initial = false);

	void calcVel();

	void reset();

	float getRot();
	void addRot(float amount);
private:
	float _rot = 0;
};
