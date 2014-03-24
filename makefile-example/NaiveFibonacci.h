#pragma once

#include "Fibonacci.h"

class NaiveFibonacci : public IFibonacci {
public:
	virtual int Calc(int n);
};