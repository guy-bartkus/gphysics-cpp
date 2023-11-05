#include "Matter.h"
#include "MyColors.h"
#include "Defines.h"
#include <cmath>

Matter::Matter() {};

Matter::Matter(uint8_t type, float x, float y, uint8_t zoneCount) : type(type), pos(x, y), zoneCount(zoneCount) {
	mass = Matter::s_mass;

    zones.resize(zoneCount);

	calcZones(true);

    if (type == 0) {
        color = MyColors::negative;
    }
    else {
        color = MyColors::positive;
    }

    setOrigin(sf::Vector2f(Matter::s_radius, Matter::s_radius));
    setRadius(Matter::s_radius);
    setOutlineThickness(2);
    setOutlineColor(color);
    setFillColor(sf::Color(0, 0, 0, 150));
    setPosition(x, y);
}

void Matter::calcZones(bool initial) {
    for (uint8_t i = 0; i < zoneCount; i++) {
        float curRot = fmod(_rot + (i * PI2) / (float)zoneCount, PI2);

        if (!initial) {
            zones[i].dir = Vec2::fromAngle(curRot);
        } else {
            zones[i] = {Vec2::fromAngle(curRot), 0};
        }
    }
}

void Matter::calcVel() {
    Vec2 newVel;

    for (uint8_t i = 0; i < zoneCount; i++) {
        Zone& zone = zones[i];

        if (zone.bits == 0) continue;

        Vec2 dirOpp = zone.dir * -1.0f;
        Vec2 dirOppScaled = dirOpp * (zone.bits / (float)mass);
        newVel = newVel + dirOppScaled;
    }

    velNorm = newVel.normalize();
    vel = velNorm;

    //std::cout << newVel << std::endl;

    if (newVel.mag() <= 1) vel = newVel;
}

void Matter::reset() {
    mass = Matter::s_mass;

    for (uint8_t i = 0; i < zoneCount; i++) {
        zones[i].bits = 0;
    }

    calcVel();
}

float Matter::getRot() {
    return _rot;
}

void Matter::addRot(float amount) {
    _rot = fmod(_rot+amount, PI2);
    calcZones();
}