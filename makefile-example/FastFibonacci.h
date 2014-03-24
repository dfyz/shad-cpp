#pragma once

#include "Fibonacci.h"

class FastFibonacci : public IFibonacci {
public:
	virtual int Calc(int n);
};