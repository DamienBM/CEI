import os
import glob
import numpy as np
from sklearn.cluster import KMeans
import time


def get_list_files_filtered(filt, dir):
    cwd = os.getcwd()
    cwd = cwd.replace("python-code\src", "")
    cwd += dir
    list_files = glob.glob(cwd+filt)
    return list_files


def get_points(files):
    final_points = []
    if len(files) != 0:
        points = []
        taille_min = 2**63 - 1  # Max size of an "int" in python

        for f in files:
            tmp = []
            file = open(f, "r")
            file.readline()
            file.readline()
            file.readline()
            file.readline()
            content = file.readlines()
            #print(content)
            content = [x.strip().split() for x in content]
            #print(content)
            tmp.extend(x[1] for x in content)
            if len(tmp) < taille_min:
                taille_min = len(tmp)
            points.append(tmp)
            file.close()

        middle_points = []
        for point in points:
            middle_points.append(point[:taille_min])

        for cpt in range(taille_min):
            tmp = []
            tmp.extend(x[cpt] for x in middle_points)
            final_points.append(tmp)

    return final_points


def save_estimator(est):
    cwd = os.getcwd()
    cwd = cwd.replace("python-code\src", "")
    filename = cwd + "pred\\cluster\\KMeans.txt"  # Add the axis name for differentiating the clusters
    file = open(filename, "a")  # Save the cluster for the active prediction
    # For loop for saving the centroids
    file.close()


def do_clustering(the_list):
    # CLUSTERING TO DO : gaussian Mixture, K-means, ...
    est = KMeans(init='k-means++', n_clusters=8)
    est.fit(the_list)

    # label = est.labels_ # Get labels for each vect
    # test = est.cluster_centers_
    save_estimator(est)
    del est


if __name__ == '__main__':

    start_time = time.time()

    spindle_list_files = []
    spindle_list_files = get_list_files_filtered("*Power*.plt", "pred\\axes\\")
    spindle_list_files.extend(get_list_files_filtered("SpindleTemp*.plt", "out\\"))
    spindle_list_files.extend(get_list_files_filtered("SpindleSpeed*.plt", "out\\"))
    spindle_features_list = get_points(spindle_list_files)
    del spindle_list_files
    do_clustering(spindle_features_list)
    del spindle_features_list

    mot_files = []
    axes_number = []
    mot_files.extend(get_list_files_filtered("*Mcn*.plt", "out\\"))
    axes_number.extend(x.split("\\")[-1].split("_")[1] for x in mot_files)  # Get number of motors
    del mot_files

    my_list_files = []
    motor_features_list = []
    while len(axes_number) != 0:
        index = axes_number.pop()
        str_torque = "*Torque*_" + str(index) + "_*.plt"
        my_list_files.extend(get_list_files_filtered(str_torque, "pred\\axes\\"))
        str_temp = "*ServoTemp*_" + str(index) + "_*.plt"
        my_list_files.extend(get_list_files_filtered(str_temp, "out\\"))
        str_speed = "*ServoSpeed*_" + str(index) + "_*.plt"
        my_list_files.extend(get_list_files_filtered(str_speed, "out\\"))
        motor_features_list = get_points(my_list_files)
        do_clustering(motor_features_list)
        del motor_features_list[:]
        del my_list_files[:]

    del motor_features_list
    del my_list_files
    end_time = time.time()
    print("Execution time : " + str(end_time-start_time))
