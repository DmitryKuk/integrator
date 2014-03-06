#ifndef INTEGRATOR_H
#define INTEGRATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

long double integrator(
		unsigned char processes,			// Количество потоков
		long double left,					// Левая граница
		long double right,					// Правая граница
		long double delta,					// Приращение аргуметна
		long double (*func)(long double)	// Функция
	);

#endif	// INTEGRATOR_H