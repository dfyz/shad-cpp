#include "FastFibonacci.h"

int FastFibonacci::Calc(int n) {
	int a = 1;
	int b = 1;
	for (int x = 2; x <= n; x++) {
		int c = a + b;
		a = b;
		b = c;
	}
	return b;
}