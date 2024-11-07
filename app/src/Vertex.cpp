//
// Created by Jlisowskyy on 06/11/24.
//

/* internal includes */
#include "../include/PrimitiveData/Vertex.h"
#include "../include/ManagingObjects/ObjectMgr.h"

Vertex::Vertex(const QVector3D &p,
               const QVector3D &pu,
               const QVector3D &pv,
               const QVector3D &n,
               const float u,
               const float v,
               const double alpha,
               const double beta) : position(p),
                                    puVector(pu),
                                    pvVector(pv),
                                    normal(n),
                                    rotatedPosition(p),
                                    rotatedPuVector(pu),
                                    rotatedPvVector(pv),
                                    rotatedNormal(n),
                                    u(u),
                                    v(v) {
    ObjectMgr::rotateZ(ObjectMgr::rotateX(rotatedPosition, beta), alpha);
    ObjectMgr::rotateZ(ObjectMgr::rotateX(rotatedPuVector, beta), alpha);
    ObjectMgr::rotateZ(ObjectMgr::rotateX(rotatedPvVector, beta), alpha);
    ObjectMgr::rotateZ(ObjectMgr::rotateX(rotatedNormal, beta), alpha);
}
