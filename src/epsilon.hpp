#pragma once

// Small offset to ray, to avoid self intersection of geometry
#define EPSILON_RAY 0.00001

// Smallest occlusion distance to evaluate, due to numeric precision
// Should be at least twice as big as EPSILON_RAY
#define EPSILON_DISTANCE 0.00005

// Smallest "valid" cos theta, due to numeric precision
#define EPSILON_COS_THETA 0.00001

// When all values are near zero, they are considered zero, and colour is black
#define EPSILON_BLACK 1.e-8f
