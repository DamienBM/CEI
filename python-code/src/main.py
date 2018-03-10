import os
import glob
import numpy as np

def get_list_files_filtered(filt):
    cwd = os.getcwd()
    cwd = cwd.replace("python-code\src","")
    cwd += "pred\\axes\\"
    list_files = glob.glob(cwd+filt)
    return list_files

def get_points(list_files):
    points = []
    for f in list_files :
        tmp = []
        file = open(f,"r")
        file.readline()
        file.readline()
        file.readline()
        file.readline()
        content = file.readlines()
        content = [x.strip().split() for x in content]
        tmp.extend(x[1] for x in content)
        points.append(tmp)
        file.close()
    taille_min=np.infty
    for point in points :
        if(len(point)<taille_min):
            taille_min = len(point)

    final_points = []
    for point in points:
        final_points.append(point[:taille_min])

    return final_points

def do_clustering(list_points):
    #CLUSTERING TO DO : gaussian Mixture, Kmeans, ...
    a = 2

if __name__ == '__main__':

    filtre = "*Torque*.plt"
    list_files = get_list_files_filtered(filtre)

    list_points = get_points(list_files)#Get all points in list like [ [...], [...], [...], ..., [...]] in same order as file in the \out dir
                                        #From there vectors of list_points have the same size and are synchronized

    do_clustering(list_points)


