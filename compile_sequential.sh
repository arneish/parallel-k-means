rm kmeans-seq
gcc lab1_io.c main_sequential.c a1_sequential.c -o kmeans-seq -lm -fopenmp -O3
echo "Successful compilation.\nTo execute: 'sh run_sequential.sh <#clusters> <#threads> <input_file> <output_file> <centroids_file>'"

