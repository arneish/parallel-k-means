#include <stdio.h>
#include "omp.h"
static long num_steps = 100000;
double step;
int main()
{
	int i; double x, pi, sum = 0.0;
	step = 1.0/(double) num_steps;
	for (i=0; i<num_steps; i++)
	{
		x = (i+0.5)*step;
		sum+=4.0/(1.0+x*x);
	}
	pi = step*sum;
}
	omp_set_num_threads(4);
#pragma omp parallel
	{
		int ID = omp_get_thread_num();
		pooh(ID, A);
		printf("Hello OpenMP! %d", ID);
	}
	printf("all done!\n");
	return 0;
}
