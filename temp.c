#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *functionC();
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
int counter = 0;

int* array_global;
void fun(int *a){
    array_global = a+1;
    printf("function print\n");
    for (int i=0;i<10; i++)
    {
        printf("%d, ", array_global[i]);
    } 
    printf("\n");
}

void fun2(int * num)
{
    printf("fun2:%d\n", num[0]);
}

main()
{
    int num = 5;
    fun2(&num);
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