import os
import glob
from matplotlib import pyplot
from mpl_toolkits.mplot3d import Axes3D
from pylab import *


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

        for ff in files:
            tmp = []
            file = open(ff, "r")
            file.readline()
            file.readline()
            file.readline()
            file.readline()
            content = file.readlines()
            # print(content)
            content = [x.strip().split() for x in content]
            # print(content)
            tmp.extend(float(x[1]) for x in content)
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


def plot_the_points(the_list, tuple_nom_valnom):

    fig = pyplot.figure()
    ax = Axes3D
    # ax = fig.add_axes([0.1, 0.1, 0.8, 0.8])  # left, bottom, width, height (range 0 to 1)
    XX = []
    YY = []
    ZZ = []

    if tuple_nom_valnom[1] == 5.5:
        XX_continuous = [0, 7000, 10000]
        YY_continuous = [0, 5.5, 5.5]
        ZZ_continuous = [20, 20 , 20]

        XX_1min = [0, 7000, 10000]
        YY_1min = [0, 26, 26]
        ZZ_1min = [20, 20, 20]

    else:
        XX_continuous = [0, 3000, 4000]
        YY_continuous = [8, 8, 5.5]
        ZZ_continuous = [20, 20, 20]

        XX_1min = [0, 2000, 4000]
        YY_1min = [32, 32, 21]
        ZZ_1min = [20, 20, 20]

    the_list = the_list[:800]  # Constraint the list for plotting
    for x, y, z in the_list:
        XX.append(math.fabs(x))
        YY.append(math.fabs(y*tuple_nom_valnom[1]/100))
        ZZ.append(math.fabs(z))

    ZZ = np.linspace(0, len(XX), len(XX), dtype='int')

    ax.scatter(XX, YY, ZZ)
    # ax.plot(XX, YY, '.r', label="data")
    # ax.plot(XX_continuous, YY_continuous, 'yellow', label="1min. rated power")
    # ax.plot(XX_1min, YY_1min, 'black', label="Continuous rated power")
    # ax.set_xlabel('Speed (tr/min)')
    # ax.set_ylabel('Power (kW) or Torque (Nm)')
    # ax.set_zlabel('Temps (indice)')
    ax.set_title(tuple_nom_valnom[0])
    pyplot.show()


def plot_the_labelized_points(the_list, label):
    fig = pyplot.figure()

    LABEL_COLOR_MAP = {0: 'red',
                       1: 'blue',
                       2: 'green',
    }

    label_color = [LABEL_COLOR_MAP[l] for l in label]

    # ax = Axes3D
    ax = fig.add_axes([0.1, 0.1, 0.8, 0.8])  # left, bottom, width, height (range 0 to 1)
    XX = []
    YY = []
    ZZ = []

    for x, y, z in the_list:
        XX.append(math.fabs(x))
        YY.append(math.fabs(y))
        ZZ.append(math.fabs(z))


    # ax.scatter(XX, YY, ZZ)
    ax.plot(XX, YY, '.r', label="data",c=label_color)
    ax.set_xlabel('Speed (tr/min)')
    ax.set_ylabel('Power (kW) or Torque (Nm)')
    # ax.set_zlabel('Temp (°C)')
    pyplot.show()


def do_clustering(the_list, nom_val):

    plot_the_points(the_list, nom_val)

    # CLUSTERING :check

    # label = est.labels_ # Get labels for each vector of points
    # centroids = est.cluster_centers_
    
    # plot_the_labelized_points(the_list, label)
    save_estimator(est)
    del est


if __name__ == '__main__':

    start_time = time.time()

    spindle_list_files = []
    spindle_list_files.extend(get_list_files_filtered("SpindleSpeed*.plt", "out\\"))
    spindle_list_files.extend(get_list_files_filtered("*Power*.plt", "pred\\axes\\"))
    spindle_list_files.extend(get_list_files_filtered("SpindleTemp*.plt", "out\\"))
    spindle_features_list = get_points(spindle_list_files)
    del spindle_list_files
    do_clustering(spindle_features_list, ("Spindle", 5.5))
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
        str_speed = "*ServoSpeed*_" + str(index) + "_*.plt"
        my_list_files.extend(get_list_files_filtered(str_speed, "out\\"))
        str_torque = "*Torque*_" + str(index) + "_*.plt"
        my_list_files.extend(get_list_files_filtered(str_torque, "pred\\axes\\"))
        str_temp = "*ServoTemp*_" + str(index) + "_*.plt"
        my_list_files.extend(get_list_files_filtered(str_temp, "out\\"))
        motor_features_list = get_points(my_list_files)
        do_clustering(motor_features_list, ("Servo_"+str(index), 8.0))
        del motor_features_list[:]
        del my_list_files[:]

    del motor_features_list
    del my_list_files
    end_time = time.time()
    print("Execution time : " + str(end_time-start_time))
