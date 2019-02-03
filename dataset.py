from sklearn.datasets import make_blobs
from matplotlib import pyplot 
from mpl_toolkits.mplot3d import Axes3D
import random, numpy as np
from sklearn.utils import shuffle

num_range = 700             # base range of data point cordinates
total_points = 1000000          # total number of data points
noise = 25                  # data points that are not generated near a blob_center
K = 4                       # number of blobs
N = total_points - noise    # data points to be generated near blob_center

blob_center = []               # list of K blob centers, each having 3 dimentions
for i in range(K):
    # Closely located blob_center can be used to generate closely located blobs
    # Change the range of generation of blob_center to get closely located blobs
    blob_center.append(random.sample(range(-num_range, num_range), 3))

# diameter is the range around each blob_center in which points are generated
# To have blobs of different sizes, change the elements of list diameter 
diameter = [num_range//(K+1) for i in range(K)]

# list of number of data points in each blob
# To have blobs of different density, change the elements of points_in_blob 
points_in_blob = [N//K for i in range(K)]

# Generate the random blobs
X = np.ndarray((total_points, 3), int)      # 'total_points' integer data points each having 3 dimention
length = 0
# This makes a cube shaped blob of 'points_in_blob' points around 'diameter' area of blob_center
# To make blobs of different size or density, change 'diameter' or 'points_in_blob' respectively
for j in range (K):
    for i in range (points_in_blob[j]):
        dim1 = random.randint(blob_center[j][0]-diameter[j], blob_center[j][0]+diameter[j])
        dim2 = random.randint(blob_center[j][1]-diameter[j], blob_center[j][1]+diameter[j])
        dim3 = random.randint(blob_center[j][2]-diameter[j], blob_center[j][2]+diameter[j])
        X[length] = [dim1, dim2, dim3]
        length = length + 1

# Use this loop to add noise in data. The numbers generated here, can be from anywhere in the num_range.
# Need to change value of 'noise' above
for i in range (length, total_points):
    dim1 = random.randint(-num_range, num_range)
    dim2 = random.randint(-num_range, num_range)
    dim3 = random.randint(-num_range, num_range)
    X[length] = [dim1, dim2, dim3]
    length = length + 1

X = shuffle(X)


# SIMPLE TEST CASES
# More restrictive way of generating 3d classification dataset
# This generates visibly disjoint blobs
# X, y = make_blobs(n_samples=N, centers=K, n_features=3, random_state=0)


# Save the array of data points in file as per the format
filename = "dataset_" + str(total_points) + "_" + str(K) + ".txt"
np.savetxt(filename, X, fmt="%d", header=str(total_points), comments='')

# Shows 3D plot for visualization 
fig = pyplot.figure()
ax = fig.add_subplot(111, projection='3d')
ax.scatter(X[:,0],X[:,1],X[:,2])
pyplot.show()

