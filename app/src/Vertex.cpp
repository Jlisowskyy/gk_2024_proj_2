//
// Created by Jlisowskyy on 06/11/24.
//

/* internal includes */
#include "../include/PrimitiveData/Vertex.h"
#include "../include/Rendering/Mesh.h"
#include "../include/ManagingObjects/ObjectMgr.h"

Vertex::Vertex(const QVector3D &p,
               const QVector3D &pu,
               const QVector3D &pv,
               const QVector3D &n,
               const float u,
               const float v,
               const float alpha,
               const float beta) : position(p),
                                   puVector(pu),
                                   pvVector(pv),
                                   normal(n),
                                   rotatedPosition(p),
                                   rotatedPuVector(pu),
                                   rotatedPvVector(pv),
                                   rotatedNormal(n),
                                   u(u),
                                   v(v) {
    Mesh::rotate(rotatedPosition, alpha, beta);
    Mesh::rotate(rotatedPuVector, alpha, beta);
    Mesh::rotate(rotatedPvVector, alpha, beta);
    Mesh::rotate(rotatedNormal, alpha, beta);
}
