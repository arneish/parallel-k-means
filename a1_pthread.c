#include "lab1_io.h"
#include "lab1_pthread.h"
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <assert.h>

#define MAX_ITER 100
#define THRESHOLD 1e-6
#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

int num_points_global;
int num_threads_global;
int num_iterations_global;
double delta_global = THRESHOLD + 1;
int K_global;
int *data_points_global;
float *centroids_global;
int *data_point_cluster_global;
int **cluster_count_global;

#ifdef USE_SPINLOCK
pthread_spinlock_t spinlock;
#else
pthread_mutex_t mutex1;
#endif

pthread_barrier_t centroid_update_barrier;
pthread_barrier_t delta_check_barrier;

void *kmeans_assignment_thread(void *tid)
{
    int *id = (int *)tid;
    int length_per_thread = num_points_global / num_threads_global;
    int start = (*id) * length_per_thread;
    int range = start + length_per_thread;
    if (range + length_per_thread > num_points_global)
    {
        //assign last undistributed points to this thread for computation
        range = num_points_global;
        length_per_thread = num_points_global - start;
    }
    printf("Thread ID:%d, start:%d, range:%d\n", *id, start, range);
    int i = 0, j = 0;
    double min_dist, current_dist;
    int *point_to_cluster = (int *)malloc(length_per_thread * sizeof(int));
    float *cluster_location = (float *)malloc(K_global * 3 * sizeof(float));
    int *cluster_count = (int *)malloc(K_global * sizeof(int));
    int iter_counter = 0;
    while ((delta_global > THRESHOLD) && (iter_counter < MAX_ITER)) //+1 is for the last assignment to cluster centroids (from previous iter)
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

            //add to local cluster_loc coordinates
            cluster_count[point_to_cluster[i - start]] += 1;
            cluster_location[point_to_cluster[i - start] * 3] += (float)data_points_global[i * 3];
            cluster_location[point_to_cluster[i - start] * 3 + 1] += (float)data_points_global[i * 3 + 1];
            cluster_location[point_to_cluster[i - start] * 3 + 2] += (float)data_points_global[i * 3 + 2];
        }
//write cluster_location to centroids_global and cluster_count_global
#ifdef USE_SPINLOCK
        pthread_spin_lock(&spinlock);
#else
        pthread_mutex_lock(&mutex1);
#endif
        for (i = 0; i < K_global; i++)
        {
            if (cluster_count[i] == 0)
            {
                printf("Unlikely situation!\n");
                continue;
            }
            centroids_global[((iter_counter + 1) * K_global + i) * 3] =
                (centroids_global[((iter_counter + 1) * K_global + i) * 3] * cluster_count_global[iter_counter][i] + cluster_location[i * 3]) / (float)(cluster_count_global[iter_counter][i] + cluster_count[i]);
            centroids_global[((iter_counter + 1) * K_global + i) * 3 + 1] =
                (centroids_global[((iter_counter + 1) * K_global + i) * 3 + 1] * cluster_count_global[iter_counter][i] + cluster_location[i * 3 + 1]) / (float)(cluster_count_global[iter_counter][i] + cluster_count[i]);
            centroids_global[((iter_counter + 1) * K_global + i) * 3 + 2] =
                (centroids_global[((iter_counter + 1) * K_global + i) * 3 + 2] * cluster_count_global[iter_counter][i] + cluster_location[i * 3 + 2]) / (float)(cluster_count_global[iter_counter][i] + cluster_count[i]);
            cluster_count_global[iter_counter][i] += cluster_count[i];
        }
#ifdef USE_SPINLOCK
        pthread_spin_unlock(&spinlock);
#else
        pthread_mutex_unlock(&mutex1);
#endif
        printf("centroid barrier-approached thread-ID:%d\n", *id);
        pthread_barrier_wait(&centroid_update_barrier);
        if (*id == 0)
        {
            printf("\n");
            for (i = 0; i < K_global; i++)
            {
                printf("thread 0's print of centroid #%d: %f,%f,%f\n", i + 1, centroids_global[((iter_counter + 1) * K_global + i) * 3], centroids_global[((iter_counter + 1) * K_global + i) * 3 + 1], centroids_global[((iter_counter + 1) * K_global + i) * 3 + 2]);
            }
        }
        /*Convergence check: Sum of L2-norms over every cluster*/
        if (*id == 0)
        {
            double temp_delta = 0.0;
            for (i = 0; i < K_global; i++)
            {
                temp_delta += (centroids_global[((iter_counter + 1) * K_global + i) * 3] - centroids_global[((iter_counter)*K_global + i) * 3]) * (centroids_global[((iter_counter + 1) * K_global + i) * 3] - centroids_global[((iter_counter)*K_global + i) * 3]) + (centroids_global[((iter_counter + 1) * K_global + i) * 3 + 1] - centroids_global[((iter_counter)*K_global + i) * 3 + 1]) * (centroids_global[((iter_counter + 1) * K_global + i) * 3 + 1] - centroids_global[((iter_counter)*K_global + i) * 3 + 1]) + (centroids_global[((iter_counter + 1) * K_global + i) * 3 + 2] - centroids_global[((iter_counter)*K_global + i) * 3 + 2]) * (centroids_global[((iter_counter + 1) * K_global + i) * 3 + 2] - centroids_global[((iter_counter)*K_global + i) * 3 + 2]);
            }
            delta_global = temp_delta;
            printf("Thread-id:%d delta_global:%f\n", *id, delta_global);
            num_iterations_global++;
        }
        printf("DELTA-CHECK barrier-approached thread-ID:%d\n", *id);
        pthread_barrier_wait(&delta_check_barrier);
        iter_counter++;
    }
    printf("Thread:%d iter-count:%d\n", *id, iter_counter);

    /*Assign points to final choice for cluster centroids:*/
    for (i = start; i < range; i++)
    {
        //assign points to clusters
        data_point_cluster_global[i * 4] = data_points_global[i * 3];
        data_point_cluster_global[i * 4 + 1] = data_points_global[i * 3 + 1];
        data_point_cluster_global[i * 4 + 2] = data_points_global[i * 3 + 2];
        data_point_cluster_global[i * 4 + 3] = point_to_cluster[i - start];
        assert(point_to_cluster[i - start] >= 0 && point_to_cluster[i - start] < K_global);
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
    num_iterations_global = 0;
    K_global = K;
    data_points_global = data_points;
    /*Allocating space for data_points_cluster:*/
    *data_point_cluster = (int *)malloc(N * 4 * sizeof(int));
    data_point_cluster_global = *data_point_cluster;
    /*Allocating space for centroids:*/
    centroids_global = (float *)calloc((MAX_ITER + 1) * K * 3, sizeof(float));
    /*Assigning first K points to be initial centroids:*/
    for (i = 0; i < K; i++)
    {
        centroids_global[i * 3] = data_points[i * 3];
        centroids_global[i * 3 + 1] = data_points[i * 3 + 1];
        centroids_global[i * 3 + 2] = data_points[i * 3 + 2];
    }
    /*Printing initial centroids:*/
    for (i = 0; i < K; i++)
    {
        printf("initial centroid #%d: %f,%f,%f\n", i + 1, centroids_global[i * 3], centroids_global[i * 3 + 1], centroids_global[i * 3 + 2]);
    }
    /*Allocating space for cluster_count_global:*/
    cluster_count_global = (int **)malloc(MAX_ITER * sizeof(int *));
    for (i = 0; i < MAX_ITER; i++)
    {
        cluster_count_global[i] = (int *)calloc(K, sizeof(int));
    }
    /*Creating threads:*/
    pthread_t kmeans_thread[num_threads];
/*Locks init:*/
#ifdef USE_SPINLOCK
    pthread_spin_init(&spinlock, 0);
#else
    pthread_mutex_init(&mutex1, NULL);
#endif
    pthread_barrier_init(&centroid_update_barrier, NULL, num_threads);
    pthread_barrier_init(&delta_check_barrier, NULL, num_threads);
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

    /*Record *num_iterations & write values to centroids from centroids_global:*/
    *num_iterations = num_iterations_global;
    int centroids_size = (*num_iterations + 1) * K * 3;
    printf("centroids_size:%d\n", centroids_size);
    *centroids = (float *)calloc(centroids_size, sizeof(float));
    for (i = 0; i < centroids_size; i++)
    {
        (*centroids)[i] = centroids_global[i];
    }

#ifdef USE_SPINLOCK
    pthread_spin_destroy(&spinlock);
#else
    pthread_mutex_destroy(&mutex1);
#endif
    pthread_barrier_destroy(&centroid_update_barrier);
    pthread_barrier_destroy(&delta_check_barrier);
    /*Printing final centroids:*/
    for (i = 0; i < K; i++)
    {
        printf("centroid #%d: %f,%f,%f\n", i + 1, (*centroids)[((*num_iterations) * K + i) * 3], (*centroids)[((*num_iterations) * K + i) * 3 + 1], (*centroids)[((*num_iterations) * K + i) * 3 + 2]);
    }
};