#include "lab1_io.h"
#include "lab1_pthread.h"
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <limits.h>

#ifndef CORES
#define CORES 4
#endif


int num_points_global;
int num_threads_global;
int K_global;
int* data_points_global;


void* kmeans_assignment_thread(void *tid)
{
    int *id = (int*) tid;
    int length_per_thread = num_points_global/num_threads_global;
    int start = (*id)*length_per_thread;
    int i=0;
    for(i=start; i<start+length_per_thread; i++)
    {
        //assign these points to their nearest cluster
        int min_distance = INT_MAX;
        
        
        //calculate local means 

    }


}

void kmeans_pthread(int num_threads,
                    int N,
                    int K,
                    int *data_points,
                    int **data_point_cluster,
                    int **centroids,
                    int *num_iterations)
{
    int i=0;
    num_points_global = N;
    num_threads_global = num_threads;
    K_global = K;
    data_points_global = data_points;
    centroids = (int**)malloc((*num_iterations)*sizeof(int*));
    for (i=0; i<*num_iterations; i++)
    
    pthread_t kmeans_thread[num_threads];
    int *tid = (int *)malloc(sizeof(int) * num_threads);
    int *iret = (int *)malloc(sizeof(int) * num_threads);
    for (i = 0; i < num_threads; i++)
    {
        tid[i]=i;
        iret[i]=pthread_create(&kmeans_thread[i], NULL, kmeans_assignment_thread, &tid[i]);
        if (!iret[i])
            printf("Thread %d successfully created!\n",i);
    }
    for (i=0; i<num_threads; i++)
    {
        pthread_join(kmeans_thread[i], NULL);
    }


};