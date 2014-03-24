all: fib
	
fib: main.o FastFibonacci.o NaiveFibonacci.o
	g++ -o fib main.o FastFibonacci.o NaiveFibonacci.o

main.o: main.cpp  FastFibonacci.h NaiveFibonacci.h
	g++ -c main.cpp

FastFibonacci.o: FastFibonacci.cpp FastFibonacci.h 
	g++ -c FastFibonacci.cpp

NaiveFibonacci.o: NaiveFibonacci.cpp NaiveFibonacci.h 
	g++ -c NaiveFibonacci.cpp

run: fib
	echo 'slow 20' | ./fib