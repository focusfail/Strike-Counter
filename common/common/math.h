#ifndef COMMON_MATH_H
#define COMMON_MATH_H
#include <math.h>

#define COMMON_MATH_EPSILON (0.0000005f)

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define clamp(a, min_, max_) (min(max(a, min_), max_)) // Fixed!
#define epsilon_eq(a, b) (fabs((a) - (b)) <= COMMON_MATH_EPSILON)

#endif // COMMON_MATH_H
