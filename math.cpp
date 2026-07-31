#include "math.h"

double CalculatorMath::Add(double a, double b) {
    return a + b;
}

double CalculatorMath::Subtract(double a, double b) {
    return a - b;
}

double CalculatorMath::Multiply(double a, double b) {
    return a * b;
}

double CalculatorMath::Divide(double a, double b, bool& error) {
    if (b == 0.0) {
        error = true;
        return 0.0;
    }
    error = false;
    return a / b;
}