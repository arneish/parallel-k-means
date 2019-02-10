#sequential time
import os 
import subprocess
import matplotlib.pyplot as plt

sizes = [50000, 100000, 200000, 400000, 800000, 1000000]
threads = [1, 2, 4, 8, 16]
# sizes = [10000, 50000]
# threads = [1,2]


try:
    subprocess.call('rm ./time_seq.txt', shell=True)
except:
    print("exception")

for size in sizes:
    syntax = 'sh run_sequential.sh 10 dataset_' + str(size)+'_4.txt out_'+str(size)+'.txt cent_'+str(size)+'.txt'
    #syntax = 'sh run_pthread.sh 4 dataset_' + str(size)+'_4.txt out_'+str(size)+'.txt cent_'+str(size)+'.txt'
    print("syntax:", syntax)
    subprocess.call(syntax, shell=True)

try:
    #subprocess.call('rm ./time_pthread.txt', shell=True)
    subprocess.call('rm ./time_openmp.txt', shell=True)
except:
    print("exception")

for size in sizes:
    for thread in threads:
        #syntax = 'sh run_sequential.sh 4 dataset_' + str(size)+'_4.txt out_'+str(size)+'.txt cent_'+str(size)+'.txt'
        syntax = 'sh run_omp.sh 10 '+str(thread)+' dataset_' + str(size)+'_4.txt out_'+str(size)+'.txt cent_'+str(size)+'.txt'
        print("syntax:", syntax)
        subprocess.call(syntax, shell=True)

file_seq ="time_seq.txt"
#file_pthread ="time_pthread.txt"
file_openmp = "time_openmp.txt"
file_s_pthread = open("speedup_pthread.txt", "a")
with open(file_seq) as f:
    sequential = f.readlines()
sequential=[x.strip() for x in sequential]
sequential = list(map(float, sequential))
print('sequential times:', sequential)

# with open(file_pthread) as f:
#     pthread = f.readlines()
# pthread=[x.strip() for x in pthread]
# pthread = list(map(float, pthread))
# print('pthread times:', pthread)
with open(file_openmp) as f:
    openmp = f.readlines()
openmp=[x.strip() for x in openmp]
openmp= list(map(float, openmp))
print('pthread times:', openmp)
i = 0
num_proc = len(threads)
for t_s in sequential:
    print("size:", sizes[i])
    # currentlist = pthread[i*num_proc:(i+1)*num_proc]
    currentlist = openmp[i*num_proc:(i+1)*num_proc]
    print("current:", currentlist)
    speedup = [t_s/x for x in currentlist]
    eff = []
    for j in range (len(speedup)):
        eff.append(speedup[j]/threads[j]) 
    # for speed in speedup:
    #     file_s_pthread.write("%s," % speed)
    file_s_pthread.write("\n")
    plt.plot(threads, eff, label='N='+str(sizes[i]))
    plt.scatter(threads, eff)
    # for x,y in zip(threads, speedup):
    #     plt.text(x, y, str(round(y, 1)))
    print("speedup:", speedup)
    i+=1
# file_s_pthread.close()
plt.title("Efficiency versus Number of threads at varying N")
#plt.title("Efficiency versus Number of threads at varying N")

plt.legend(loc='upper right')
plt.xlabel("Number of threads")
#plt.ylabel("Speedup")
plt.ylabel("Efficiency")
plt.show()