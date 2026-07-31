#ifndef MATH_H
#define MATH_H

class CalculatorMath {
public:
    static double Add(double a, double b);
    static double Subtract(double a, double b);
    static double Multiply(double a, double b);
    static double Divide(double a, double b, bool& error);
};

#endif