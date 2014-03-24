#include "NaiveFibonacci.h"

int NaiveFibonacci::Calc(int n) {
	if (n <= 1) {
		return 1;
	}
	return Calc(n - 1) + Calc(n - 2);
}