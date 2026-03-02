#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Author : Zoheb Abai
# Repo : https://github.com/ZohebAbai/mobile_sensing_robotics/

import numpy as np
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.pyplot as plt

from sklearn.neighbors import NearestNeighbors

# icp_known_corresp: performs icp given that the input datasets
# are aligned so that Line1(:, QInd(k)) corresponds to Line2(:, PInd(k))
def icp_known_corresp(Line1, Line2, QInd, PInd):
    Q = Line1[:, QInd]
    P = Line2[:, PInd]

    MuQ = compute_mean(Q)
    MuP = compute_mean(P)
    #print(f"{MuQ = }")
    #print(f"{MuP = }")
    
    W = compute_W(Q, P, MuQ, MuP)
    #print(f"{W = }")

    #print("\nCompute R (rotation matrix (dim, dim)) and t (translation vector (dim, 1)")
    R, t = compute_R_t(W, MuQ, MuP)
    #print(f"{R = }")
    #print(f"{t = }")

    
    # Compute the new positions of the points after
    # applying found rotation and translation to them
    # print("""\nCompute the new line by: 
    #         rotating P (a set of points (dim, num_points)) through matrix multiplication (dim, dim) * (dim, num_points) = (dim, num_points)
    #         and adding the translation vector (1)
    #       """)
    NewLine = R.dot(P) + t
    #print(f"{NewLine.shape = }")
    
    E = compute_error(Q, NewLine)

    return NewLine, E


# compute_W: compute matrix W to use in SVD
def compute_W(Q, P, MuQ, MuP):
    #print("\nCompute the centered points")
    mP = P - MuP
    mQ = Q - MuQ
    #print(f"{mP.shape = }")
    #print(f"{mQ.shape = }")

    #print("\nCompute the cross correlation matrix")
    W = mQ.dot(mP.T) # considering weight as 1
    #print(f"{W.shape = }")
    return W

    
# compute_R_t: compute rotation matrix and translation vector
# based on the SVD as presented in the lecture
def compute_R_t(W, MuQ, MuP):
    U, S, V_T = np.linalg.svd(W)
    R = U.dot(V_T)
    t = MuQ - R.dot(MuP)

    return R, t


# compute_mean: compute mean value for a [M x N] matrix
def compute_mean(M):
    center = np.array([M.mean(axis=1)]).T # considerig weight as 1
    return center


# compute_error: compute the icp error
def compute_error(Q, OptimizedPoints):
    return np.sum(np.linalg.norm(Q - OptimizedPoints)**2)


# simply show the two lines
def show_figure(Line1, Line2):
    plt.figure(figsize=(15,8))
    plt.scatter(Line1[0], Line1[1], marker='o', s=8, label='Line 1')
    plt.scatter(Line2[0], Line2[1], marker='o', s=4, label='Line 2')
    
    plt.xlim([-8, 8])
    plt.ylim([-8, 8])
    plt.legend()  
    
    plt.show()


# initialize figure
def init_figure():
    fig = plt.gcf()
    fig.show()
    fig.canvas.draw()
    
    line1_fig = plt.scatter([], [], marker='o', s=8, label='Line 1')
    line2_fig = plt.scatter([], [], marker='o', s=4, label='Line 2')
    #plt.title(title)
    plt.figure(figsize=(15,8))
    plt.xlim([-8, 8])
    plt.ylim([-8, 8])
    plt.legend()
    return fig, line1_fig, line2_fig


# update_figure: show the current state of the lines
def update_figure(fig, line1_fig, line2_fig, Line1, Line2, hold=False):
    line1_fig.set_offsets(Line1.T)
    line2_fig.set_offsets(Line2.T)
    if hold:
        plt.show()
    else:
        fig.canvas.flush_events()
        fig.canvas.draw()
        plt.pause(0.5)


def compute_iterative_icp(line_1: np.ndarray, line_2: np.ndarray):
    """Computes the iterative ICP algorithm to align line_2 to line_1 

    Used when the correspondences are unknown.

    Args:
        line_1: The first line used as the reference.
        line_2: The second line to be aligned to line_1.
    """

    MaxIter = 100
    Epsilon = 0.001
    E = np.inf

    # KNN for line_1
    # n_neighbors -> the number of nearest neighbors we wnat to find for each point
    # i.e., we only want 1 since we want a 1 to 1 correspondence
    neigh = NearestNeighbors(n_neighbors=1)

    # preprares the data to be indexed accordingly to the fit strategy;
    # this is not really learning since it has no parameters to learn, distances
    # not computed until you call neigh.kneighbors
    # NOTE: nearest neighbors expects rows to be points and columns to be the dimensions,
    #       so we need to transpose both lines (2, 100) -> (100, 2) then we can compute
    #       the distances between every pair to get the closest point
    neigh.fit(line_1.T)

    for i in range(MaxIter):

        # point with index QInd(1, k) from line_1 corresponds to
        # point with index PInd(1, k) from line_2
        # Find correspondences using KNN
        # i[0] takes the 
        #QInd = np.array([i[0] for i in neigh.kneighbors(line_2.T, return_distance=False)]) 
        QInd = neigh.kneighbors(line_2.T, return_distance=False)[:, 0] 
        PInd = np.arange(line_2.shape[1])

        # update line_2 and error
        # Now that you know the correspondences, use your implementation
        # of icp with known correspondences and perform an update
        EOld = E
        line_2, E = icp_known_corresp(line_1, line_2, QInd, PInd)

        if i % 10 == 0:
            show_figure(line_1, line_2)
            print('Error value on ' + str(i) + ' iteration is: ', E)

        if E < Epsilon:
            break