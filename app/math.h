#ifndef _MATH_H
#define _MATH_H

#define M_E  2.7182818284590452354 
#define M_PI 3.14159265358979323846

void srand(uint32_t seed);
uint32_t random();

double sin(double x); 
double cos(double x);
double sqrt(double x);
double log2(double x, double y);
double atan2(double y,double x);
double tan(double x);
double cot(double x);

double pow(double a, double b);
double exp(double x);
double log(double x);

#endif /*_MATH_H*/
