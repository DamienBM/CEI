import os
import glob
import numpy as np
from sklearn.cluster import KMeans


def get_list_files_filtered(filt,dir):
    cwd = os.getcwd()
    cwd = cwd.replace("python-code\src", "")
    cwd += dir
    list_files = glob.glob(cwd+filt)
    return list_files


def get_points(files):
    points = []
    for f in files:
        tmp = []
        file = open(f, "r")
        file.readline()
        file.readline()
        file.readline()
        file.readline()
        content = file.readlines()
        content = [x.strip().split() for x in content]
        tmp.extend(x[1] for x in content)
        points.append(tmp)
        file.close()
    taille_min = np.infty
    for point in points:
        if len(point) < taille_min:
            taille_min = len(point)

    final_points = []
    for point in points:
        final_points.append(point[:taille_min])

    return final_points


def save_estimator(cluster_list):
    cwd = os.getcwd()
    cwd = cwd.replace("python-code\src", "")
    filename = cwd + "\\pred\\cluster\\KMeans.txt"  # Add the axis name for differentiating the clusters
    fichier = open(filename, "w")  # Save the cluster for the active prediction
    #  For loop for saving the centroids

def do_clustering(the_list):
    # CLUSTERING TO DO : gaussian Mixture, K-means, ...
    est = KMeans(init='k-means++', n_clusters=8)
    est.fit(the_list)

    # label = est.labels_ # Get labels for each points
    test = est.cluster_centers_
    save_estimator(test)
    return test


if __name__ == '__main__':

    my_list_files = []
    my_list_files = get_list_files_filtered("*Power*.plt", "pred\\axes\\")
    my_list_files.extend(get_list_files_filtered("*Torque*.plt", "pred\\axes\\"))
    my_list_files.extend(get_list_files_filtered("*Temp*.plt", "out\\"))

    # Get all points in list like [ [...], [...], [...], ..., [...]] in same order as file in the \out dir
    # From there vectors of list_points have the same size and are synchronized
    list_points = get_points(my_list_files)

    do_clustering(list_points)


