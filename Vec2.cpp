#include "Vec2.h"
#include <cmath>
#include <iostream>
#include <SFML/Graphics.hpp>

using namespace sf;

Vec2::Vec2() : x(0.0f), y(0.0f) {}

Vec2::Vec2(float x, float y) : x(x), y(y) {}

Vec2 Vec2::normalize() const {
	float m = sqrt(x*x + y*y);

	return Vec2(x/m, y/m);
}

Vec2 Vec2::rotate(float ang) const {
	return Vec2(x * cos(ang) - y * sin(ang), x * sin(ang) + y * cos(ang));
}

Vector2f Vec2::toVector2f() const {
	return Vector2f(x, y);
}

float Vec2::dot(Vec2& v) const {
	return x*v.x + y*v.y;
}

float Vec2::cross(Vec2& v) const {
	return (x*v.y) - (y*v.x);
}

float Vec2::distance(Vec2& v) const {
	float lX = x - v.x;
	float lY = y - v.y;

	return sqrt(lX * lX + lY * lY);
}

float Vec2::distanceSquared(Vec2& v) const {
	float lX = x - v.x;
	float lY = y - v.y;

	/*return pow(lX, 2) + pow(lY, 2);*/
	return lX * lX + lY * lY;
}

float Vec2::mag() const {
	return sqrt(x*x+y*y);
}

float Vec2::angleBetween(Vec2& v) const {
	return atan2(v.y * x - v.x * y, v.x * x + v.y * y);
}

Vec2 Vec2::fromAngle(float rads) {
	return Vec2(cos(rads), sin(rads));
}

Vec2 Vec2::operator+(const Vec2& v) const {
	return Vec2(x+v.x, y+v.y);
}

Vec2 Vec2::operator-(Vec2& v) const {
	return Vec2(x - v.x, y - v.y);
}

Vec2 Vec2::operator*(float scalar) const {
	return Vec2(x*scalar, y*scalar);
}

Vec2 Vec2::operator*(Vec2& v) const {
	return Vec2(x * v.x, y * v.y);
}

Vec2 Vec2::operator/(float scalar) const {
	return Vec2(x / scalar, y / scalar);
}

Vec2 Vec2::operator/(Vec2& v) const {
	return Vec2(x / v.x, y / v.y);
}

std::ostream& operator<<(std::ostream& stream, const Vec2& v) {
	return stream << v.x << ", " << v.y;
}