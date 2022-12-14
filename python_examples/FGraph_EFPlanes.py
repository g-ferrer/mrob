import mrob
import numpy as np
import open3d


def draw_planes(synthetic,traj=[]):
    pcds = []
    poses = synthetic.get_number_poses()
    for i in range(poses):
        pc = open3d.geometry.PointCloud()
        pc.points = open3d.utility.Vector3dVector(np.array(synthetic.get_point_cloud(i)))
        s = i/poses
        pc.paint_uniform_color([0.5,1-s, (1-s)/2])
        if traj!=[]:
            if type(traj[i]) == mrob.geometry.SE3:
                pc.transform(traj[i].T()) #traj is a list of SE3
            else:
                pc.transform(traj[i]) #traj is a list of 4x4 np arrays
        pcds.append(pc)
    open3d.visualization.draw_geometries(pcds)

N_points = 500
N_planes = 4
N_poses = 5

# Generate the problem
T0 = mrob.geometry.SE3(np.random.randn(6)*1e0) # Test this
synthetic = mrob.registration.CreatePoints(N_points,N_planes,N_poses, 0.05, 0.1, T0) #point noise, bias noise
T_gt = synthetic.get_ground_truth_last_pose()
#T_gt.print()
gt_traj = synthetic.get_trajectory()
for Ti in gt_traj:
    Ti.print()
draw_planes(synthetic)


# create the problem in old optim structure
problem = mrob.registration.PlaneRegistration() #empty creator
synthetic.create_plane_registration(problem)
#problem.solve(mrob.registration.INITIALIZE)


# Solution 1: FG point, gives an initial solution. The error can NOT be compared with others
# -----------------------------------------------------------------------------------

graph = mrob.FGraph()

# Adding all empty EF planes
for t in range(N_planes):
    ef1 = graph.add_eigen_factor_point()
    # It is an ordered progression 0:N-1, no need for dict
    print('EFactor id = ', ef1)

# Initializing the trajectories. It requires an initial solution
n1 = graph.add_node_pose_3d(T0, mrob.NODE_ANCHOR)
print('Initial Pose node id = ', n1)


for t in range(1,N_poses):
    #NOTE this starts alsmot at 0, while T is at ||t||=1e3
    # if the initialization was better, then the solution will be faster to converge
    n1 = graph.add_node_pose_3d(mrob.geometry.SE3())
    # It is an ordered progression 0:N-1, no need for dict
    print('Pose node id = ', n1)

for t in range(N_poses):
    print('Processing pose ', t)
    points = synthetic.get_point_cloud(t)
    indexes = synthetic.get_point_plane_ids(t)
    for p,i in zip(points,indexes):     
        #print('point ', p, 'index ', i)
        graph.eigen_factor_plane_add_point(planeEigenId = i,
                                   nodePoseId = t,
                                   point = p,
                                   W = 1.0)

print('\n\n\n\nInitial error EF plane= ', graph.chi2(True))
print('solve iters = ', graph.solve(mrob.LM_ELLIPS,20))
print('Chi2 = ', graph.chi2())
traj1 = graph.get_estimated_state()
draw_planes(synthetic, graph.get_estimated_state())

# Solution 2: Old plane aligment routing optimization. It is not in the form of factors (EFs)
# -----------------------------------------------------------------------------------
problem.reset_solution()
problem.solve(mrob.registration.INITIALIZE)
print('\n\n\n\nOld EF joint Optm error INI= ', problem.get_error())
#problem.solve(mrob.registration.GN_HESSIAN)
#problem.solve(mrob.registration.LM_ELLIP)
problem.solve(mrob.registration.LM_SPHER)
print('Chi2 Lm Sphere= ', problem.get_error())
draw_planes(problem, problem.get_trajectory())
print(problem.print_evaluate())


# Solution 3: FG Planes
# -----------------------------------------------------------------------------------
graph = mrob.FGraph()

# Adding all empty EF planes
for t in range(N_planes):
    ef1 = graph.add_eigen_factor_plane()

# Initializing the trajectories. It requires an initial solution
n1 = graph.add_node_pose_3d(T0, mrob.NODE_ANCHOR)


# The previous solution from EF point should be used in here
for t in range(1,N_poses):
    n1 = graph.add_node_pose_3d(mrob.geometry.SE3(traj1[t]))
    # It is an ordered progression 0:N-1, no need for dict
    print('Pose node id = ', n1)


for t in range(N_poses):
    print('Processing pose ', t)
    points = synthetic.get_point_cloud(t)
    indexes = synthetic.get_point_plane_ids(t)
    for p,i in zip(points,indexes):
        #print('point ', p, 'index ', i)
        graph.eigen_factor_plane_add_point(planeEigenId = i,
                                   nodePoseId = t,
                                   point = p,
                                   W = 1.0)

print('\n\n\n\nInitial error EF plane = ', graph.chi2(True))
graph.solve(mrob.LM_ELLIPS,50)
print('Chi2 = ', graph.chi2())
draw_planes(synthetic, graph.get_estimated_state())
traj2 = graph.get_estimated_state()


# Solution 4: FG Planes Centered
# -----------------------------------------------------------------------------------
graph = mrob.FGraph()

# Adding all empty EF planes
for t in range(N_planes):
    ef1 = graph.add_eigen_factor_plane_center()

# Initializing the trajectories. It requires an initial solution
n1 = graph.add_node_pose_3d(T0, mrob.NODE_ANCHOR)


# The previous solution from EF point should be used in here
for t in range(1,N_poses):
    n1 = graph.add_node_pose_3d(mrob.geometry.SE3(traj1[t]))
    # It is an ordered progression 0:N-1, no need for dict
    print('Pose node id = ', n1)


for t in range(N_poses):
    print('Processing pose ', t)
    points = synthetic.get_point_cloud(t)
    indexes = synthetic.get_point_plane_ids(t)
    for p,i in zip(points,indexes):
        #print('point ', p, 'index ', i)
        graph.eigen_factor_plane_add_point(planeEigenId = i,
                                   nodePoseId = t,
                                   point = p,
                                   W = 1.0)

print('\n\n\n\nInitial error EF plane Center= ', graph.chi2(True))
graph.solve(mrob.LM_ELLIPS,50)
print('Chi2 = ', graph.chi2())
draw_planes(synthetic, graph.get_estimated_state())
traj3 = graph.get_estimated_state()


# Eval two trajectoryes:
for i,T in enumerate(traj1):
    print(T)
    print(traj2[i])


# Showing the Information Matrix
if 0:
    import matplotlib.pyplot as plt
    L = graph.get_information_matrix()
    plt.spy(L, marker='o', markersize=5)
    plt.title('Information matrix $\Lambda$')
    plt.show()
#graph.print(True)
