//
// Created by Jlisowskyy on 05/11/24.
//

#ifndef APP_TRIANGLE_H
#define APP_TRIANGLE_H

/* internal includes */
#include "Vertex.h"

template<size_t N>
using PolygonArr = std::array<Vertex, N>;
using Triangle = PolygonArr<3>;

#endif //APP_TRIANGLE_H
