rm kmeans-omp
gcc lab1_io.c main_omp.c a2_openmp.c -o kmeans-omp -lm -fopenmp 
echo "Successful compilation of k-means-OpenMP.\nTo execute: 'sh run_omp.sh <#clusters> <#threads> <input_file> <output_file> <centroids_file>'"

