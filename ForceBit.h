#pragma once

#include <SFML/Graphics.hpp>
#include "Matter.h"
#include "Vec2.h"

class ForceBit {
public:
	Vec2 pos;
	Vec2 vel;
	float stepsToExit;
	float steps;
	uint8_t type;
	sf::Color color;
	Matter* passing = nullptr;
	bool used = true;
	static const uint8_t s_radius = 2;
public:
	ForceBit();

	ForceBit(float x, float y, Vec2& vel, uint8_t type);

	void switchPolarity();

	void setPolarity(uint8_t& type);

	void calcExitSteps(float& universeRadius, Vec2& center);
};