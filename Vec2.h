#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>

using namespace sf;

struct Vec2 {
	float x, y;

	Vec2();

	Vec2(float x, float y);

	Vec2 normalize() const;

	Vec2 rotate(float rads) const;

	Vector2f toVector2f() const;

	float dot(Vec2& v) const;

	float cross(Vec2& v) const;

	float distance(Vec2& v) const;

	float distanceSquared(Vec2& v) const;

	float mag() const;

	float angleBetween(Vec2& v) const;

	static Vec2 fromAngle(float rads);

	Vec2 operator+(const Vec2& v) const;

	Vec2 operator-(Vec2& v) const;

	Vec2 operator*(float scalar) const;

	Vec2 operator*(Vec2& v) const;

	Vec2 operator/(float scalar) const;

	Vec2 operator/(Vec2& v) const;
};

std::ostream& operator<<(std::ostream& stream, const Vec2& v);