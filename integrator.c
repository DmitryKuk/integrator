#include "integrator.h"

int cmp(const long double *a, const long double *b)
{
	return (a - b);
}


long double simple_integrator(int status_fd, unsigned char child_id, long double left, long double right, long double delta, long double (*func)(long double))
{
	long double res = 0, l = left, r = l + delta;
	
	if (status_fd < 0) {	// Работа без сообщений о готовности
		for (; r <= right; l = r, r += delta)
			res += (func(l) + func(r)) * (r - l) / 2;
		if (l < right)
			res += (func(l) + func(right)) * (right- l) / 2;
	} else {	// Работа с сообщениями о готовности
		unsigned char status_data[1 + sizeof(float)];
		status_data[0] = child_id;
		
		*(float *)(status_data + 1) = (float)0;
		write(status_fd, status_data, sizeof(status_data));
		
		for (; r <= right; l = r, r += delta) {
			res += (func(l) + func(r)) * (r - l) / 2;
			
			status_data[0] = child_id;
			*(float *)(status_data + 1) = (float)(100 * r / right);
			write(status_fd, status_data, sizeof(status_data));
		}
		
		if (l < right) {
			res += (func(l) + func(right)) * (right - l) / 2;
			
			*(float *)(status_data + 1) = (float)100;
			write(status_fd, status_data, sizeof(status_data));
		}
	}
	
	return res;
}


long double integrator(unsigned char processes, long double left, long double right, long double delta, long double (*func)(long double))
{
	if (processes == 0) processes = 1;
	
	int fd[2];
	if (pipe(fd)) {
		perror("Pipe creation error");
		return 0;
	}
	
	int status_fd[2];
	if (pipe(status_fd)) {
		perror("Pipe (status) creation error");
		status_fd[0] = status_fd[1] = -1;
	}
	
	unsigned char i;
	pid_t pid;
	
	
	// Создание дочерних процессов
	long double l = left, dl = (right - left) / processes, r = l + dl;
	for (i = 0; i < processes; ++i, l = r, r += dl) {
		pid = fork();
		if (pid == 0) {	// Дочерний процесс
			close(fd[0]);
			close(status_fd[0]);
			if (r > right) r = right;
			
			//fprintf(stderr, "%Lf..%Lf\n", x, r);
			
			// Вычисление интеграла на отрезке [l, l + dl]
			long double res = simple_integrator(status_fd[1], i, l, r, delta, func);
			write(fd[1], &res, sizeof(res));
			
			close(fd[1]);
			close(status_fd[1]);
			exit(0);
		} else if (pid < 0)	// Родительский процесс: проверка на ошибку fork
			perror("Fork error");
	}
	close(fd[1]);
	close(status_fd[1]);
	
	
	if (status_fd[0] >= 0) {
		pid = fork();
		if (pid == 0) {	// Потомок, выводящий статус
			float statuses[processes];
			unsigned char status_data[1 + sizeof(float)];
			unsigned int updates = 0, max_updates_print = 1200, max_updates_update = max_updates_print - 3 * processes;
			unsigned int j;
			
			while (read(status_fd[0], status_data, sizeof(status_data)) == sizeof(status_data)) {
				if (status_data[0] < processes) {
					if (updates > max_updates_update)
						statuses[status_data[0]] = *(float *)(status_data + 1);
					++updates;
				}
				
				if (updates > max_updates_print) {
					updates = 0;
					
					for (j = 0; j < processes; ++j)	// Печатаем результаты
						printf("%6.2f%%  ", statuses[j]);
					for (j = 0; j < processes * 10; ++j)	// ... и стираем их
						putchar('\b');
				}
			}
			
			for (j = 0; j < processes; ++j)	// Печатаем результаты
				printf("100.00%%  ");
			putchar('\n');
			
			close(status_fd[0]);
			exit(0);
		} else if (pid < 0)
			perror("Fork (status) creation error");
		close(status_fd[0]);
	}
	
	
	// Получаем результаты, вычисленные потомками
	long double res_array[processes];
	for (i = 0; i < processes && read(fd[0], res_array + i, sizeof(long double)) == sizeof(long double); ++i)
		;
	close(fd[0]);
	
	// Сортируем их для меньшей погрешности
	qsort(res_array, i, sizeof(long double), (int (*)(const void *, const void *))cmp);
	double res = 0;
	unsigned char j = 0;
	for (; j < i; ++j)
		res += res_array[j];
	
	
	if (status_fd[0] >= 0) {	// Ждём конца печати статусов
		int status;
		waitpid(pid, &status, 0);
	}
	
	return res;
}