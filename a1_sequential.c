#include "lab1_io.h"
#include "lab1_omp.h"
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <assert.h>

#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

int num_points_global;
int num_iterations_global;
int K_global;
int *data_points_global;
float *centroids_global;
int *data_point_cluster_global;

void kmeans_sequential_execution()
{
    printf("Sequential k-means start\n");
    int i = 0, j = 0;
    double min_dist, current_dist;
    int iter_counter = 0;
    int *point_to_cluster = (int *)malloc(num_points_global * sizeof(int));
    float *cluster_location = (float *)malloc(K_global * 3 * sizeof(float));
    int *cluster_count = (int *)malloc(K_global * sizeof(int));
    while (iter_counter < num_iterations_global + 1) //+1 is for the last assignment to cluster centroids (from previous iter)
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
            if (iter_counter == num_iterations_global)
            {
                //assign points to clusters
                data_point_cluster_global[i * 4] = data_points_global[i * 3];
                data_point_cluster_global[i * 4 + 1] = data_points_global[i * 3 + 1];
                data_point_cluster_global[i * 4 + 2] = data_points_global[i * 3 + 2];
                data_point_cluster_global[i * 4 + 3] = point_to_cluster[i];
                assert(point_to_cluster[i] >= 0 && point_to_cluster[i] < K_global);
                continue;
            }
            //add to local cluster_loc coordinates
            cluster_count[point_to_cluster[i]] += 1;
            cluster_location[point_to_cluster[i] * 3] += (float)data_points_global[i * 3];
            cluster_location[point_to_cluster[i] * 3 + 1] += (float)data_points_global[i * 3 + 1];
            cluster_location[point_to_cluster[i] * 3 + 2] += (float)data_points_global[i * 3 + 2];
        }
        if (iter_counter == num_iterations_global)
        {
            printf("Sequential k-means returning\n");
            return;
        }
        //write cluster_location to centroids_global and cluster_count_global
        for (i = 0; i < K_global; i++)
        {
            assert(cluster_count[i] != 0);
            centroids_global[((iter_counter + 1) * K_global + i) * 3] = cluster_location[i * 3] / cluster_count[i];
            centroids_global[((iter_counter + 1) * K_global + i) * 3 + 1] = cluster_location[i * 3 + 1] / cluster_count[i];
            centroids_global[((iter_counter + 1) * K_global + i) * 3 + 2] = cluster_location[i * 3 + 2] / cluster_count[i];
        }
        printf("sequential centroid-update about to execute:\n");
        for (i = 0; i < K_global; i++)
        {
            printf("thread 0's print of centroid # \033[1;31m%d: %f,%f,%f\n\033[0m", i + 1, centroids_global[((iter_counter + 1) * K_global + i) * 3], centroids_global[((iter_counter + 1) * K_global + i) * 3 + 1], centroids_global[((iter_counter + 1) * K_global + i) * 3 + 2]);
        }
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

    /*Executing k-means sequential:*/
    kmeans_sequential_execution();

    /*Printing final centroids:*/
    for (i = 0; i < K; i++)
    {
        printf("centroid #%d: %f,%f,%f\n", i + 1, (*centroids)[((*num_iterations) * K + i) * 3], (*centroids)[((*num_iterations) * K + i) * 3 + 1], (*centroids)[((*num_iterations) * K + i) * 3 + 2]);
    }
};