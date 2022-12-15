#pragma once

#include "FastMath.h"
#include "Camera.h"
#include "World.h"

#define RENDER_DISTANCE 10
#define MAX_CHUNK_DISTANCE RENDER_DISTANCE * CHUNK_SIZE

static vec3 skybox(vec3& direction) { // TODO
    return vec3(std::abs(direction[0]), std::abs(direction[1]), std::abs(direction[2]));
}

static vec3 trace(const vec3& source, const Ray& ray, World& world, int bounces, int maxBounces, float& depth);

static void calculateTMaxForNextVoxel(vec3& rayLoc, vec3& rayDir, short* locDif, vec3& tMax) {
    for (char i = 0; i < 3; i++) {
        tMax[i] = rayDir[i] == 0.000f ? std::numeric_limits<float>::max() : (((rayLoc[i] == fastfloor(rayLoc[i]) ? rayLoc[i] + locDif[i] : (locDif[i] == 1 ? fastceil(rayLoc[i]) : fastfloor(rayLoc[i]))) - rayLoc[i]) / rayDir[i]);
    }
}

static vec3 reflect(const vec3& source, const Material& mat, vec3& location, vec3& direction, vec3& normal, World& world, int bounces, int maxBounces) { // returns a color from reflected
    vec3 refDir = direction;
    for (char i = 0; i < 3; i++) {
        if (normal[i] != 0) {
            refDir[i] = -refDir[i];
            break;
        }
    }
    Ray reflected = Ray(location, refDir);
    float depth;
    return trace(source, reflected, world, bounces, maxBounces, depth);
}

static vec3 refract(const vec3& source, const Material& mat, vec3& location, vec3& direction, vec3& normal, World& world, int bounces, int maxBounces) {
    float fresnel;
    float cosi = clamp(-1, 1, dot(direction, normal));
    float etai = 1, etat = mat.effectValue; // TODO: etai has to be the same as the effectvalue of the voxel before this intersection
    if (cosi > 0) { std::swap(etai, etat); }
    // Compute sini using Snell's law
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
    // Total internal reflection
    if (sint >= 1) {
        fresnel = 1;
    } else {
        float cost = sqrtf(std::max(0.f, 1 - sint * sint));
        cosi = fabsf(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        fresnel = (Rs * Rs + Rp * Rp) / 2;
    }

    // reflect
    if (fresnel == 1.0f) { return reflect(source, mat, location, direction, normal, world, bounces, maxBounces); }

    // refract
    vec3 n = normal;
    if (cosi < 0) { cosi = -cosi; }
    else { std::swap(etai, etat); n = -normal; }
    float eta = etai / etat;
    float k = 1 - eta * eta * (1 - cosi * cosi);
    vec3 refractDir = k < 0 ? vec3(0, 0, 0) : (direction * eta + n * (eta * cosi - sqrtf(k)));
    Ray refracted = Ray(location, refractDir);

    // output
    float depth = 0;
    if (fresnel == 0.0f) { return trace(source, refracted, world, bounces, maxBounces, depth); }
    return reflect(source, mat, location, direction, normal, world, bounces, maxBounces) * fresnel + trace(source, refracted, world, bounces, maxBounces, depth) * (1.0f - fresnel);
}

static bool shadow(const vec3& source, const vec3& start, World& world) {
    vec3 location = start;
    vec3 direction = world.sunDirection;
    direction = direction.normalize();
    vec3 tMax(0, 0, 0);
    vec3 normal;
    short locDif[3]{ direction.x() > 0.0f ? 1 : -1, direction.y() > 0.0f ? 1 : -1, direction.z() > 0.0f ? 1 : -1 };
    int x = fastfloor(location.x()), y = fastfloor(location.y()), z = fastfloor(location.z());

    calculateTMaxForNextVoxel(location, direction, locDif, tMax);

    //printf_s("Shadow debug:\n");
    //direction.print();
    //tMax.print();

    // Iterate ray
    while ((location - source).length() < MAX_CHUNK_DISTANCE) {
        // traverse the ray to next voxel
        calculateTMaxForNextVoxel(location, direction, locDif, tMax);
        if (tMax.x() < tMax.y() && tMax.x() < tMax.z()) { // tMaxX
            location += direction * tMax.x();
            x += locDif[0];
            normal = vec3((locDif[0] == 1 ? -1.0f : 1.0f), 0.0f, 0.0f);
        } else if (tMax.y() < tMax.z()) { //tMaxY
            location += direction * tMax.y();
            y += locDif[1];
            normal = vec3(0.0f, (locDif[1] == 1 ? -1.0f : 1.0f), 0.0f);
        } else if (tMax.z() < 1000.0f) { // tMaxZ, check included to make sure tMaxZ is not near infinite
            location += direction * tMax.z();
            z += locDif[2];
            normal = vec3(0.0f, 0.0f, (locDif[2] == 1 ? -1.0f : 1.0f));
        } else {
            printf_s("tMax values are too big..\n");
            exit(-200);
        }

        // Check if voxel is solid
        if (world.get_voxel(x, y, z)) { // TODO: fix glass scattering twice if the same material repeats immediatly in the next voxel
            return true;
        }
    }
    return false;
}

vec3 trace(const vec3& source, const Ray& ray, World& world, int bounces, int maxBounces, float& depth) {
    vec3 location = ray.position;
    vec3 direction = unit_vector(ray.direction);
    if (bounces == 0) { return skybox(direction); }
    vec3 tMax(0, 0, 0);
    vec3 normal;
    short locDif[3]{ direction.x() > 0.0f ? 1 : -1, direction.y() > 0.0f ? 1 : -1, direction.z() > 0.0f ? 1 : -1 };
    int32_t next[3]{ direction.x() > 0.0f ? fastceil(location.x()) : fastfloor(location.x()),
                     direction.y() > 0.0f ? fastceil(location.y()) : fastfloor(location.y()),
                     direction.z() > 0.0f ? fastceil(location.z()) : fastfloor(location.z()) };
    int32_t current[3]{ direction.x() < 0.0f ? fastceil(location.x()) : fastfloor(location.x()),
                        direction.y() < 0.0f ? fastceil(location.y()) : fastfloor(location.y()),
                        direction.z() < 0.0f ? fastceil(location.z()) : fastfloor(location.z()) };
    uint32_t voxelMaterialId;

    calculateTMaxForNextVoxel(location, direction, locDif, tMax);

    // Iterate ray
    while ((location - source).length() < MAX_CHUNK_DISTANCE) {
        // traverse the ray to next voxel
        calculateTMaxForNextVoxel(location, direction, locDif, tMax);
        if (tMax.x() < tMax.y() && tMax.x() < tMax.z()) { // tMaxX
            location += direction * tMax.x();
            current[0] += locDif[0];
            normal = vec3((locDif[0] == 1 ? -1.0f : 1.0f), 0.0f, 0.0f);
        } else if (tMax.y() < tMax.z()) { //tMaxY
            location += direction * tMax.y();
            current[1] += locDif[1];
            normal = vec3(0.0f, (locDif[1] == 1 ? -1.0f : 1.0f), 0.0f);
        } else if (tMax.z() < 1000.0f) { // tMaxZ, check included to make sure tMaxZ is not near infinite
            location += direction * tMax.z();
            current[2] += locDif[2];
            normal = vec3(0.0f, 0.0f, (locDif[2] == 1 ? -1.0f : 1.0f));
        } else {
            printf_s("tMax values are too big..\n");
            exit(-200);
        }

        // Check if voxel is solid
        if (voxelMaterialId = world.get_voxel(current[0], current[1], current[2])) { // TODO: fix glass scattering twice if the same material repeats immediatly in the next voxel
            if (voxelMaterialId > world.materials.size()-1) {
                printf_s("VoxelMaterialId %i was over %i, reset to 0", voxelMaterialId, world.materials.size() - 1);
                voxelMaterialId = 0;
            }
            const Material& mat = world.materials[voxelMaterialId];
            Ray shadowRay = Ray();
            vec3 rayLoc = location;
            rayLoc -= direction * 0.0001f;
            bool shad = shadow(source, location - direction * 0.0001f, world);
            //printf_s("%s\n", shad ? "true" : "false");
            float light = dot(world.sunDirection, normal) * (shad ? 1.0f : 1.0f);
            switch (mat.type) {
            case REFLECTIVE: {
                if (mat.effectValue <= 0.0f) { return mat.albedo * light; }
                if (mat.effectValue >= 1.0f) { return reflect(source, mat, rayLoc, direction, normal, world, bounces - 1, maxBounces) * light; }
                return reflect(source, mat, rayLoc, direction, normal, world, bounces - 1, maxBounces) * mat.effectValue * light + mat.albedo * (1 - mat.effectValue) * light;
                break;
            }
            case REFRACTIVE: { // TODO: FIX: half of the surface is darker and the other half is lighter, is this because of reflection?
                return mat.albedo * light; // TMP
                return refract(source, mat, rayLoc, direction, normal, world, bounces - 1, maxBounces) * light;
                break;
            }
            default: {
                return mat.albedo * light;
                break;
            }
            }
        }
    }
    return skybox(direction);
}