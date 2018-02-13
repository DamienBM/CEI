#code for machine learning algorithm
import os,glob
import mdp
from mpl_toolkits import mplot3d
import numpy as np
import matplotlib.pyplot as plt
import pylab

def get_data():
    all_data = []
    tmp = []
    list_to_append = []
    cur_dir = os.path.dirname(os.path.abspath(__file__))
    cur_dir += "\out"
    os.chdir(cur_dir)
    for file in glob.glob("*.txt"):#go through all the txt files
        print(file)#display the filename
        txt_file = open(file,"r")#open the txt file given by glob
        str_list = txt_file.read()#get all the data in the file at once
        tmp = str_list.split()
        list_to_append = [int(i) for i in tmp]
        all_data.append(list_to_append)#sparse the data and fit them into one list for each file
        txt_file.close()#close the file
    return all_data

def ml_algo(data):
    gng = mdp.nodes.GrowingNeuralGasNode(max_nodes=75)

    gng.train(data)

    gng.strop_trainig()
    n_obj = len(gng.graph.connected_components())
    return n_obj


if __name__ == '__main__':
    '''read data files part'''
    val_global = []
    val_global = get_data()

    for elt in val_global:#check if val_global is correctly filled
        print(elt)

    fig = plt.figure()
    ax = plt.axes(projection='3d')
    ax.scatter3D(val_global[0],val_global[1],val_global[2],c=val_global[2],cmap='Greens')

    '''mandatory to plot figures'''
    pylab.show()
    test = [val_global[0],val_global[1],val_global[2]]
    x = mdp.numx.concatenate(test, axis=0)
    #x = mdp.numx.take(x, mdp.numx_rand.permutation(x.shape[0]), axis=0)

    #kohonen part
    nb_cluster = ml_algo(x)
    print(nb_cluster)


