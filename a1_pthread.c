#include "lab1_io.h"
#include "lab1_pthread.h"
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <limits.h>
#include <float.h>

int num_points_global;
int num_threads_global;
int K_global;
int *data_points_global;
float **centroids_global;
int iter_counter=0;
int *cluster_count_global;

void* kmeans_assignment_thread(void *tid)
{
    int *id = (int*) tid;
    int length_per_thread = num_points_global/num_threads_global;
    int start = (*id)*length_per_thread;
    int i=0, j=0;
    float min_dist, current_dist;
    int *point_to_cluster = (int*) malloc(length_per_thread*sizeof(int));
    float *cluster_location =  (float *) malloc(K_global*3*sizeof(float));
    int *cluster_count = (int*) malloc(K_global*sizeof(int));
    /*for (i=0; i<K_global*3; i++)
    {
        cluster_location[i]=0.0;
    }
    for (i=0; i<K_global; i++)
    {
        cluster_count[i]=0;
    }*/
    for(i=start; i<start+length_per_thread; i++)
    {
        //assign these points to their nearest cluster
        min_dist = FLT_MAX;
        for (j = 0; j<K_global; j++)
        {
            current_dist = pow((double)(centroids_global[iter_counter][j*3]-data_points_global[i*3]), 2.0) +
            pow((double)(centroids_global[iter_counter][j*3+1]-(float)data_points_global[i*3+1]), 2.0) +
            pow((double)(centroids_global[iter_counter][j*3+2]-(float)data_points_global[i*3+2]), 2.0);
            if (current_dist<min_dist)
            {
                min_dist = current_dist;
                point_to_cluster[i]=j;
            }
        }
        //add to local cluster_loc coordinates
        cluster_count[point_to_cluster[i]]+=1;
        cluster_location[point_to_cluster[i]*3]+=data_points_global[i*3];
        cluster_location[point_to_cluster[i]*3+1]+=data_points_global[i*3+1];
        cluster_location[point_to_cluster[i]*3+2]+=data_points_global[i*3+2];
    }
    //write cluster_location to centroids_global & cluster_count_global too
    for (i=0; i<K_global; i++)
    {
        centroids_global[iter_counter+1][i*3]+=cluster_location[i*3];
        centroids_global[iter_counter+1][i*3+1]+=cluster_location[i*3+1];
        centroids_global[iter_counter+1][i*3+2]+=cluster_location[i*3+2];
    }

}

void kmeans_pthread(int num_threads,
                    int N,
                    int K,
                    int *data_points,
                    int **data_point_cluster,
                    float **centroids,
                    int *num_iterations)
{
    int i=0;
    num_points_global = N;
    num_threads_global = num_threads;
    K_global = K;
    data_points_global = data_points;
    /*Allocating space for centroids*/
    centroids = (float**)malloc((*num_iterations)*sizeof(float*));
    for (i=0; i<*num_iterations; i++)
    {
        centroids[i] = (float*)malloc(K*3*sizeof(float));
    }
    centroids_global = centroids;
    cluster_count_global = (int*)malloc(K*sizeof(int));
    
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