#include "ForceBit.h"
#include "MyColors.h"
#include <SFML/Graphics.hpp>
#include <cmath>

ForceBit::ForceBit() {};

ForceBit::ForceBit(float x, float y, Vec2& vel, uint8_t type) : pos(x, y), vel(vel), type(type) {
	vel = vel;
	type ? color = MyColors::positive : color = MyColors::negative;
	//vertex.color = color;
}

void ForceBit::switchPolarity() {
	type = !type;
	type ? color = MyColors::positive : color = MyColors::negative;
	//vertex.color = color;
}

void ForceBit::setPolarity(uint8_t& newType) {
	type = newType;
	type ? color = MyColors::positive : color = MyColors::negative;
	//vertex.color = color;
}

void ForceBit::calcExitSteps(float& universeRadius, Vec2& center) {
    // Translate forcebit's position to be relative to circle's center
    Vec2 relPos = pos - center;

    // Calculate the coefficients of the quadratic equation (a is always 1 since vel is normalized)
    float a = 1; // vel.dot(vel) is 1 for normalized vectors
    float b = 2 * relPos.dot(vel);
    float c = relPos.dot(relPos) - universeRadius * universeRadius;

    // Calculate the discriminant
    float discriminant = b * b - 4 * a * c;

    // Check if there is an intersection
    if (discriminant < 0) {
        // No intersection, should not happen if forcebit is inside the circle
        std::cout << "ERROR: No intersection, this should not happen if forcebit is inside the circle\n";
        return;
    }

    // Calculate both solutions (possible exit times)
    float sqrtDiscriminant = sqrt(discriminant);
    float t1 = (-b + sqrtDiscriminant) / (2 * a);
    float t2 = (-b - sqrtDiscriminant) / (2 * a);

    // Select the smallest positive solution for the exit time
    if (t1 > 0 && t2 > 0) {
        stepsToExit = std::min(t1, t2);
    }
    else {
        stepsToExit = std::max(t1, t2); // One of them might be negative
    }

    steps = 0;

    // std::cout << "Steps to exit: " << stepsToExit << std::endl;
}