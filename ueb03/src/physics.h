/**
 * @file physics.h
 * @brief TODO
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef PHYSICS_H
#define PHYSICS_H

#include <fhwcg/fhwcg.h>

typedef struct {
    vec3 point;
    vec3 normal;

    float s, t;
    bool valid;
} ContactInfo;

typedef struct {
    vec3 center;
    vec3 velocity;
    vec3 acceleration; 
    vec3 rollDir;
    
    ContactInfo contact;
    float mass;
    float radius;
} Ball;


#endif // PHYSICS_H