'''
Render.py

Description: 

COMPENG 2DX4: Course Project

The following code utilizes open3D to open, filter,
and create a mesh given a 3D point cloud. 

Requires:  Open3D 0.9.0
Author: Elston Almeida
Year: 2020
'''

import open3d as o3d
import numpy as np
import pyrender
import trimesh
import math as m



if __name__ == "__main__":

    # Read point cloud data from xyz file
    pointCloud = o3d.io.read_point_cloud("room.xyz", format = "xyz")

    # Create voxels to prevent data clumping
    voxelPointCloud = pointCloud.voxel_down_sample(voxel_size=0.02)

    # Samplify cloud data using voxels
    cl, ind = voxelPointCloud.remove_statistical_outlier(nb_neighbors=10, std_ratio=1.3)

    # Get the points in the point cloud within the specified std_dev
    inlierCloud = voxelPointCloud.select_down_sample(ind)

    # Estimate normals and get distances
    inlierCloud.estimate_normals()
    distances = inlierCloud.compute_nearest_neighbor_distance()

    # Get the average distance between points
    avg_dist = np.mean(distances)

    # Radius of ball being used for ball pivoting alg.
    radius = 4*avg_dist

    # Create mesh from ball pivoting algorithm
    mesh = o3d.geometry.TriangleMesh.create_from_point_cloud_ball_pivoting(inlierCloud, o3d.utility.DoubleVector([radius, 2*radius]))


    # Setup shaders
    shader = o3d.visualization.MeshShadeOption(255)
    col    = o3d.visualization.MeshColorOption(255)

    # Start a visualizer
    vis = o3d.visualization.Visualizer()
    vis.create_window()

    # Add geometry
    vis.add_geometry(mesh)
    
    # Get the renderer
    opt = vis.get_render_option()
    # Enable this option to stop seeing half cut meshes
    opt.mesh_show_back_face = True
    opt.mesh_shade_option = shader

    # Other options for views
    ctrl = vis.get_view_control()
    ctrl.scale(0.2)
    ctrl.change_field_of_view(step=90)
    # View the visualizer 
    vis.run()
    # Save the triangular mesh upon closing the visualizer
    o3d.io.write_triangle_mesh("room312.ply", mesh)