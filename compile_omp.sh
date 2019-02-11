rm kmeans-omp
gcc lab1_io.c main_omp.c lab1_omp.c -o kmeans-omp -lm -fopenmp -O3 
echo "Successful compilation of k-means-OpenMP.\nTo execute: 'sh run_omp.sh <#clusters> <#threads> <input_file> <output_file> <centroids_file>'"

