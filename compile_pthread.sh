rm kmeans-pthread
gcc main_pthread.c lab1_io.c a1_pthread.c -o kmeans-pthread -O3 -lpthread -fopenmp -lm
echo "Successful compilation of k-means-pthread.\nTo execute: 'sh run_pthread.sh <#clusters> <#threads> <input_file> <output_file> <centroids_file>'"
echo "NOTE: Default compilation with pthread_mutex_lock() implementation. If pthread_spin_lock() is desired, include '-DUSE_SPINLOCK' flag during compilation"


