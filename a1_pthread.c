#include "lab1_io.h"
#include "lab1_pthread.h"
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <assert.h>

#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

int num_points_global;
int num_threads_global;
int num_iterations_global;
int K_global;
int *data_points_global;
float *centroids_global;
int *data_point_cluster_global;
int **cluster_count_global;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t centroid_update_barrier;

void *kmeans_assignment_thread(void *tid)
{
    int *id = (int *)tid;
    int length_per_thread = num_points_global / num_threads_global;
    int start = (*id) * length_per_thread;
    int range = min(start + length_per_thread, num_points_global);
    printf("Thread ID:%d, start:%d, range:%d\n", *id, start, range);
    int i = 0, j = 0;
    double min_dist, current_dist;
    int *point_to_cluster = (int *)malloc(length_per_thread * sizeof(int));
    float *cluster_location = (float *)malloc(K_global * 3 * sizeof(float));
    int *cluster_count = (int *)malloc(K_global * sizeof(int));
    int iter_counter = 0;
    while (iter_counter < num_iterations_global + 1) //+1 is for the last assignment to cluster centroids (from previous iter)
    {
        for (i = 0; i < K_global * 3; i++)
            cluster_location[i] = 0.0;
        for (i = 0; i < K_global; i++)
            cluster_count[i] = 0;
        for (i = start; i < range; i++)
        {
            //assign these points to their nearest cluster
            min_dist = DBL_MAX;
            for (j = 0; j < K_global; j++)
            {
                current_dist = pow((double)(centroids_global[(iter_counter * K_global + j) * 3] - (float)data_points_global[i * 3]), 2.0) +
                               pow((double)(centroids_global[(iter_counter * K_global + j) * 3 + 1] - (float)data_points_global[i * 3 + 1]), 2.0) +
                               pow((double)(centroids_global[(iter_counter * K_global + j) * 3 + 2] - (float)data_points_global[i * 3 + 2]), 2.0);
                if (current_dist < min_dist)
                {
                    min_dist = current_dist;
                    point_to_cluster[i - start] = j;
                }
            }
            if (iter_counter == num_iterations_global)
            {
                //assign points to clusters
                data_point_cluster_global[i * 4] = data_points_global[i * 3];
                data_point_cluster_global[i * 4 + 1] = data_points_global[i * 3 + 1];
                data_point_cluster_global[i * 4 + 2] = data_points_global[i * 3 + 2];
                data_point_cluster_global[i * 4 + 3] = point_to_cluster[i - start];
                assert(point_to_cluster[i - start] >= 0 && point_to_cluster[i - start] < K_global);
                continue;
            }
            //add to local cluster_loc coordinates
            cluster_count[point_to_cluster[i - start]] += 1;
            cluster_location[point_to_cluster[i - start] * 3] += (float)data_points_global[i * 3];
            cluster_location[point_to_cluster[i - start] * 3 + 1] += (float)data_points_global[i * 3 + 1];
            cluster_location[point_to_cluster[i - start] * 3 + 2] += (float)data_points_global[i * 3 + 2];
        }
        if (iter_counter == num_iterations_global)
        {
            printf("Thread %d returning\n", *id);
            return NULL;
        }
        //write cluster_location to centroids_global and cluster_count_global
        pthread_mutex_lock(&mutex1);
        for (i = 0; i < K_global; i++)
        {
            if (cluster_count_global[iter_counter][i] + cluster_count[i] == 0)
            {
                printf("Unlikely situation!\n");
                centroids_global[((iter_counter + 1) * K_global + i) * 3] = centroids_global[(iter_counter * K_global + i) * 3];
                centroids_global[((iter_counter + 1) * K_global + i) * 3 + 1] = centroids_global[(iter_counter * K_global + i) * 3 + 1];
                centroids_global[((iter_counter + 1) * K_global + i) * 3 + 2] = centroids_global[(iter_counter * K_global + i) * 3 + 2];
                continue;
            }
            centroids_global[((iter_counter + 1) * K_global + i) * 3] =
                (centroids_global[((iter_counter + 1) * K_global + i) * 3] * cluster_count_global[iter_counter][i] + cluster_location[i * 3]) / (float)(cluster_count_global[iter_counter][i] + cluster_count[i]);
            // printf("nr: %f\n", centroids_global[iter_counter + 1][i * 3] * cluster_count_global[iter_counter][i] + cluster_location[i * 3]);
            // printf("here cluster_count[i]:%f, %d\n", centroids_global[iter_counter+1][i*3], cluster_count[i]);
            centroids_global[((iter_counter + 1) * K_global + i) * 3 + 1] =
                (centroids_global[((iter_counter + 1) * K_global + i) * 3 + 1] * cluster_count_global[iter_counter][i] + cluster_location[i * 3 + 1]) / (float)(cluster_count_global[iter_counter][i] + cluster_count[i]);
            centroids_global[((iter_counter + 1) * K_global + i) * 3 + 2] =
                (centroids_global[((iter_counter + 1) * K_global + i) * 3 + 2] * cluster_count_global[iter_counter][i] + cluster_location[i * 3 + 2]) / (float)(cluster_count_global[iter_counter][i] + cluster_count[i]);
            cluster_count_global[iter_counter][i] += cluster_count[i];
        }
        pthread_mutex_unlock(&mutex1);
        pthread_barrier_wait(&centroid_update_barrier);
        // if (*id == 0)
        // {
        //     printf("\n");
        //     for (i = 0; i < K_global; i++)
        //     {
        //         printf("thread 0's print of centroid #%d: %f,%f,%f\n", i + 1, centroids_global[((iter_counter + 1) * K_global + i) * 3], centroids_global[((iter_counter + 1) * K_global + i) * 3 + 1], centroids_global[((iter_counter + 1) * K_global + i) * 3 + 2]);
        //     }
        // }
        //printf("centroid-update barrier crossed for thread:%d\n", *id);
        iter_counter++;
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
    printf("in kmeans_pthread function number of iters:%d\n", *num_iterations);
    int i = 0;
    num_points_global = N;
    num_threads_global = num_threads;
    num_iterations_global = *num_iterations;
    K_global = K;
    data_points_global = data_points;
    /*Allocating space for data_points_cluster:*/
    *data_point_cluster = (int *)malloc(N * 4 * sizeof(int));
    data_point_cluster_global = *data_point_cluster;
    /*Allocating space for centroids:*/
    *centroids = (float *)calloc((*num_iterations + 1) * K * 3, sizeof(float));
    centroids_global = *centroids;
    /*Assigning first K points to be initial centroids:*/
    for (i = 0; i < K; i++)
    {
        (*centroids)[i * 3] = data_points[i * 3];
        (*centroids)[i * 3 + 1] = data_points[i * 3 + 1];
        (*centroids)[i * 3 + 2] = data_points[i * 3 + 2];
    }
    /*Printing initial centroids:*/
    for (i = 0; i < K; i++)
    {
        printf("initial centroid #%d: %f,%f,%f\n", i + 1, (*centroids)[i * 3], (*centroids)[i * 3 + 1], (*centroids)[i * 3 + 2]);
    }
    /*Allocating space for cluster_count_global:*/
    cluster_count_global = (int **)malloc(*num_iterations * sizeof(int *));
    for (i = 0; i < *num_iterations; i++)
    {
        cluster_count_global[i] = (int *)calloc(K, sizeof(int));
    }
    /*Creating threads:*/
    pthread_t kmeans_thread[num_threads];
    pthread_barrier_init(&centroid_update_barrier, NULL, num_threads);
    int *tid = (int *)malloc(sizeof(int) * num_threads);
    int *iret = (int *)malloc(sizeof(int) * num_threads);
    for (i = 0; i < num_threads; i++)
    {
        tid[i] = i;
        printf("Creating thread:%d\n", i);
        iret[i] = pthread_create(&kmeans_thread[i], NULL, kmeans_assignment_thread, &tid[i]);
        if (!iret[i])
            printf("Thread %d successfully created!\n", i);
        else
            printf("Thread %d creation FAILED \n", i);
    }
    for (i = 0; i < num_threads; i++)
    {
        pthread_join(kmeans_thread[i], NULL);
    }
    pthread_barrier_destroy(&centroid_update_barrier);
    /*Printing final centroids:*/
    for (i = 0; i < K; i++)
    {
        printf("centroid #%d: %f,%f,%f\n", i + 1, (*centroids)[((*num_iterations) * K + i) * 3], (*centroids)[((*num_iterations) * K + i) * 3 + 1], (*centroids)[((*num_iterations) * K + i) * 3 + 2]);
    }
};