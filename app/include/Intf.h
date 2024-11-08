//
// Created by Jlisowskyy on 11/8/24.
//

#ifndef INTF_H
#define INTF_H

/* Collection of all constants used in the project */
#include "Constants.h"

/* Collection of primitive data used in the project */
#include "PrimitiveData/ActiveEdge.h"
#include "PrimitiveData/Vertex.h"
#include "PrimitiveData/Triangle.h"

/* Additional usefull defines */
using BernsteinTable = std::array<float, BEZIER_CONSTANTS::BERNSTEIN_TABLE_SIZE>;
using ControlPoints = std::array<QVector3D, BEZIER_CONSTANTS::CONTROL_POINTS_COUNT>;
using MeshArr = std::vector<Triangle>;

#endif //INTF_H
