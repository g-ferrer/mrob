/* Copyright (c) 2022, Gonzalo Ferrer
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * estimate_plane.cpp
 *
 *  Created on: Dec 28, 2020
 *      Author: Gonzalo Ferrer
 *              g.ferrer@skoltech.ru
 *              Mobile Robotics Lab.
 */


#include "mrob/estimate_plane.hpp"
#include <Eigen/Eigenvalues>
#include <iostream>

using namespace mrob;

Mat41 estimate_plane_centered(MatRefConst X);
Mat41 estimate_plane_homogeneous(MatRefConst X);

Mat41 mrob::estimate_plane(MatRefConst X, bool flagCentered)
{
    // Initialization
    assert(X.cols() == 3  && "Estimate_plane: Incorrect sizing, we expect Nx3");
    assert(X.rows() >= 3  && "Estimate_plane: Incorrect sizing, we expect at least 3 correspondences (not aligned)");

    // Plane estimation, centered approach
    if (flagCentered)
        return estimate_plane_centered(X);
    // Plance estimation, homogenous approach
    return estimate_plane_homogeneous(X);
}

//local function, we also will test the homogeneous plane estimation
Mat41 estimate_plane_centered(MatRefConst X)
{
    uint_t N = X.rows();
    // Calculate center of points:
    Mat13 c =  X.colwise().sum();
    c /= (double)N;


    MatX qx = X.rowwise() - c;
    Mat3 C = qx.transpose() * qx;


    Eigen::SelfAdjointEigenSolver<Mat3> eigs;
    eigs.computeDirect(C);

    Mat31 normal = eigs.eigenvectors().col(0);

    matData_t d = - c*normal;

    Mat41 plane;
    plane << normal, d;
    // error = eigs.eigenvalues()(0);
    return plane;

}

Mat41 estimate_plane_homogeneous(MatRefConst X)
{
    uint_t N = X.rows();
    // Calculate center of points:
    Mat13 c =  X.colwise().sum();


    Mat4 Q;
    Q.topLeftCorner<3,3>() = X.transpose() * X;
    Q.topRightCorner<3,1>() = c;
    Q.bottomLeftCorner<1,3>() = c.transpose();
    Q(3,3) = N;
    //std::cout << "Q= \n" << Q << std::endl;


    Eigen::SelfAdjointEigenSolver<Mat4> eigs(Q);
    //std::cout << "values = \n" << eigs.eigenvalues() << std::endl;
    Mat41 plane = eigs.eigenvectors().col(0);

    double scale = plane.head<3>().norm();
    plane /= scale;

    //std::cout << "Plane = \n" << plane << std::endl;
    // error = eigs.eigenvalues()(0);
    return plane;

}


Mat31 mrob::estimate_normal(MatRefConst X)
{
    Mat41 res = estimate_plane(X);
    return res.head(3);
}


Mat31 mrob::estimate_centroid(MatRefConst X)
{
    // Initialization
    assert(X.cols() == 3  && "Estimate_centroid: Incorrect sizing, we expect Nx3");
    assert(X.rows() >= 3  && "Estimate_centroid: Incorrect sizing, we expect at least 3 correspondences (not aligned)");

    uint_t N = X.rows();
    Mat13 c =  X.colwise().sum();
    c /= (double)N;
    return c;
}
