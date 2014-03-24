#include <iostream>
#include <string>

#include "Fibonacci.h"
#include "FastFibonacci.h"
#include "NaiveFibonacci.h"

int main() {
	std::string computationType;
	int n = 0;
	std::cin >> computationType >> n;
	IFibonacci* fib;
	if (computationType == "fast") {
		std::cerr << "Using fast version" << std::endl;
		fib = new FastFibonacci();
	} else {
		std::cerr << "Using slow version" << std::endl;
		fib = new NaiveFibonacci();
	}
	std::cout << fib->Calc(n) << std::endl;
}