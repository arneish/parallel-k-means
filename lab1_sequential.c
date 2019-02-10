#include "lab1_io.h"
#include "lab1_omp.h"
#include <stdlib.h>
#include <time.h>
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
int num_iterations_global;
double delta_global = THRESHOLD + 1;
int K_global;
int *data_points_global;
float *centroids_global;
int *data_point_cluster_global;

void kmeans_sequential_execution()
{
    printf("Sequential k-means start\n");
    int i = 0, j = 0;
    double min_dist, current_dist;
    int *point_to_cluster = (int *)malloc(num_points_global * sizeof(int));
    float *cluster_location = (float *)malloc(K_global * 3 * sizeof(float));
    int *cluster_count = (int *)malloc(K_global * sizeof(int));
    int iter_counter = 0;
    double temp_delta = 0.0;
    while ((delta_global > THRESHOLD) && (iter_counter < MAX_ITER)) //+1 is for the last assignment to cluster centroids (from previous iter)
    {
        for (i = 0; i < K_global * 3; i++)
            cluster_location[i] = 0.0;
        for (i = 0; i < K_global; i++)
            cluster_count[i] = 0;
        for (i = 0; i < num_points_global; i++)
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
                    point_to_cluster[i] = j;
                }
            }

            //add to local cluster_loc coordinates
            cluster_count[point_to_cluster[i]] += 1;
            cluster_location[point_to_cluster[i] * 3] += (float)data_points_global[i * 3];
            cluster_location[point_to_cluster[i] * 3 + 1] += (float)data_points_global[i * 3 + 1];
            cluster_location[point_to_cluster[i] * 3 + 2] += (float)data_points_global[i * 3 + 2];
        }
        //write cluster_location to centroids_global
        for (i = 0; i < K_global; i++)
        {
            assert(cluster_count[i] != 0);
            centroids_global[((iter_counter + 1) * K_global + i) * 3] = cluster_location[i * 3] / cluster_count[i];
            centroids_global[((iter_counter + 1) * K_global + i) * 3 + 1] = cluster_location[i * 3 + 1] / cluster_count[i];
            centroids_global[((iter_counter + 1) * K_global + i) * 3 + 2] = cluster_location[i * 3 + 2] / cluster_count[i];
        }
        // for (i = 0; i < K_global; i++)
        // {
        //     printf("Sequential print of centroid # \033[1;31m%d: %f,%f,%f\n\033[0m", i + 1, centroids_global[((iter_counter + 1) * K_global + i) * 3], centroids_global[((iter_counter + 1) * K_global + i) * 3 + 1], centroids_global[((iter_counter + 1) * K_global + i) * 3 + 2]);
        // }
        /*Convergence check: Sum of L2-norms over every cluster*/
        temp_delta = 0.0;
        for (i = 0; i < K_global; i++)
        {
            temp_delta += (centroids_global[((iter_counter + 1) * K_global + i) * 3] - centroids_global[((iter_counter)*K_global + i) * 3]) * (centroids_global[((iter_counter + 1) * K_global + i) * 3] - centroids_global[((iter_counter)*K_global + i) * 3]) + (centroids_global[((iter_counter + 1) * K_global + i) * 3 + 1] - centroids_global[((iter_counter)*K_global + i) * 3 + 1]) * (centroids_global[((iter_counter + 1) * K_global + i) * 3 + 1] - centroids_global[((iter_counter)*K_global + i) * 3 + 1]) + (centroids_global[((iter_counter + 1) * K_global + i) * 3 + 2] - centroids_global[((iter_counter)*K_global + i) * 3 + 2]) * (centroids_global[((iter_counter + 1) * K_global + i) * 3 + 2] - centroids_global[((iter_counter)*K_global + i) * 3 + 2]);
        }
        delta_global = temp_delta;
        //printf("Sequential delta_global:%f\n", delta_global);
        iter_counter++;
    }
    num_iterations_global = iter_counter;

    /*Assign points to final choice for cluster centroids:*/
    for (i = 0; i < num_points_global; i++)
    {
        //assign points to clusters
        data_point_cluster_global[i * 4] = data_points_global[i * 3];
        data_point_cluster_global[i * 4 + 1] = data_points_global[i * 3 + 1];
        data_point_cluster_global[i * 4 + 2] = data_points_global[i * 3 + 2];
        data_point_cluster_global[i * 4 + 3] = point_to_cluster[i];
        assert(point_to_cluster[i] >= 0 && point_to_cluster[i] < K_global);
    }
}

void kmeans_sequential(int N,
					int K,
					int* data_points,
					int** data_point_cluster,
					float** centroids,
					int* num_iterations
					)
{
    printf("in kmeans_openmp function number of iters:%d\n", *num_iterations);
    int i = 0;
    num_points_global = N;
    num_iterations_global = *num_iterations;
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
    /*Executing k-means sequential:*/
    kmeans_sequential_execution();

    /*Record *num_iterations & write values to centroids from centroids_global:*/
    *num_iterations = num_iterations_global;
    int centroids_size = (*num_iterations + 1) * K * 3;
    printf("number of iterations:%d\n", num_iterations_global);
    printf("centroids_size:%d\n", centroids_size);
    *centroids = (float *)calloc(centroids_size, sizeof(float));
    for (i = 0; i < centroids_size; i++)
    {
        (*centroids)[i] = centroids_global[i];
    }

    /*Printing final centroids:*/
    for (i = 0; i < K; i++)
    {
        printf("centroid #%d: %f,%f,%f\n", i + 1, (*centroids)[((*num_iterations) * K + i) * 3], (*centroids)[((*num_iterations) * K + i) * 3 + 1], (*centroids)[((*num_iterations) * K + i) * 3 + 2]);
    }
};