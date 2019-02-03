
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

float* cluster_count;
void *functionC();
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
int counter = 0;

int* array_global;
void fun(int *a){
    printf("a:%d, ", *a);
    int b = *a+1;
    printf("a':%d", b);
}
void fun2(int * num)
{
    printf("fun2:%d\n", num[0]);
}



main()
{
    int a = 10;  
    fun(&a);
    
    // int i = 0;
    // int *array = (int *)malloc(sizeof(int) * 10);
    // for (i = 0; i < 10; i++)
    // {
    //     array[i] = rand() % 5;
    //     printf("%d, ", array[i]);
    // }
    // printf("\n");
    // fun(array);
    // int rc1, rc2;
    // pthread_t thread1, thread2;

    // if ((rc1=pthread_create( &thread1, NULL, &functionC, NULL)))
    // {
    //     printf("Thread creation failed: %d\n", rc1);
    // }
    // if ((rc2=pthread_create( &thread2, NULL, &functionC, NULL)))
    // {
    //     printf("Thread creation failed: %d\n", rc2);
    // }
    // pthread_join( thread1, NULL);
    // pthread_join( thread2, NULL);
    exit(0);
}

void *functionC()
{
    //pthread_mutex_lock( &mutex1 );
    counter++;
    printf("Counter value: %d\n", counter);
    //pthread_mutex_unlock( &mutex1 );
}