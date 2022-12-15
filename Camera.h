#pragma once

#include "Vec3.h"

struct Ray {
public:
	vec3 position, direction;

	Ray() { position = { 0,0,0 }; direction = { 0,1,0 }; }
	Ray(vec3 p, vec3 d) : position(p), direction(d){}
};

struct Camera {
public:
	float focusDistance;
	vec3 position{ 0,0,0 };

private:
	float fov;
	float lensRadius;
	float aspect;
	float aperture;
	vec3 up;
	vec3 lowerLeftCorner;
	vec3 horizontal;
	vec3 vertical;
	vec3 u, v, w; // ???

public:
	Camera() {}
	Camera(vec3 vup, float vfov, float aspect, float aperture, float focusDist) : up(vup), fov(vfov), aspect(aspect), aperture(aperture), focusDistance(focusDist) {}

	void prepare(const vec3& dir) {
		lensRadius = aperture / 2;
		float theta = fov * M_PI / 180;
		float halfHeight = tan(theta / 2);
		float halfWidth = aspect * halfHeight;
		w = unit_vector(-dir);
		u = unit_vector(cross(up, w));
		v = cross(w, u);
		lowerLeftCorner = position - halfWidth * focusDistance * u - halfHeight * focusDistance * v - focusDistance * w;
		horizontal = 2 * halfWidth * focusDistance * u;
		vertical = 2 * halfHeight * focusDistance * v;
	}

	Ray get_ray(float s, float t) const { return Ray(position, lowerLeftCorner + s * horizontal + t * vertical - position); }
};