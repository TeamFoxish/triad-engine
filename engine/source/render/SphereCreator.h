#pragma once

#include <unordered_map>
#include "core/Math.h"
#include <cmath>
#include <vector>

class IcoSphereCreator
{
public:
    struct VertData {
        Math::Vector3 pos;
        Math::Vector3 normal;
        Math::Vector2 uv = Math::Vector2::Zero;
    };
    using MeshGeometry3D = std::vector<VertData>;

    struct TriangleIndices
    {
       uint32_t v1;
       uint32_t v2;
       uint32_t v3;
    };

    struct SphereGeom {
        MeshGeometry3D geometry;
        std::vector<TriangleIndices> indices;
    };

    MeshGeometry3D geometry;
    uint32_t index;
    std::unordered_map<int64_t, uint32_t> middlePointIndexCache;

    // add vertex to mesh, fix position to be on unit sphere, return index
    uint32_t addVertex(Math::Vector3 p)
    {
        const double length = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
        const Math::Vector3 pos(p.x, p.y, p.z);
        if (length < 0.01) {
            geometry.push_back({ pos });
            return index++;
        }
        geometry.push_back({ pos / length });
        return index++;
    }

    // return index of point in the middle of p1 and p2
    uint32_t getMiddlePoint(uint32_t p1, uint32_t p2)
    {
        // first check if we have it already
        bool firstIsSmaller = p1 < p2;
        int64_t smallerIndex = firstIsSmaller ? p1 : p2;
        int64_t greaterIndex = firstIsSmaller ? p2 : p1;
        int64_t key = (smallerIndex << 32) + greaterIndex;

        uint32_t ret;
        auto iter = middlePointIndexCache.find(key);
        if (iter != middlePointIndexCache.end())
        {
            return (*iter).second;
        }

        // not in cache, calculate it
        Math::Vector3 point1 = geometry[p1].pos;
        Math::Vector3 point2 = geometry[p2].pos;
        Math::Vector3 middle = Math::Vector3(
            (point1.x + point2.x) / 2.0,
            (point1.y + point2.y) / 2.0,
            (point1.z + point2.z) / 2.0);

        // add vertex makes sure point is on unit sphere
        uint32_t i = addVertex(middle);

        // store it, return index
        middlePointIndexCache[key] = i;
        return i;
    }

    SphereGeom Create(int recursionLevel)
    {
        SphereGeom res;
        index = 0;

        // create 12 vertices of a icosahedron
        auto t = (1.0 + std::sqrt(5.0)) / 2.0;

        addVertex(Math::Vector3(-1, t, 0));
        addVertex(Math::Vector3(1, t, 0));
        addVertex(Math::Vector3(-1, -t, 0));
        addVertex(Math::Vector3(1, -t, 0));

        addVertex(Math::Vector3(0, -1, t));
        addVertex(Math::Vector3(0, 1, t));
        addVertex(Math::Vector3(0, -1, -t));
        addVertex(Math::Vector3(0, 1, -t));

        addVertex(Math::Vector3(t, 0, -1));
        addVertex(Math::Vector3(t, 0, 1));
        addVertex(Math::Vector3(-t, 0, -1));
        addVertex(Math::Vector3(-t, 0, 1));

        // create 20 triangles of the icosahedron
        auto faces = std::vector<TriangleIndices>();

        // 5 faces around point 0
        faces.push_back({ 0, 11, 5 });
        faces.push_back({ 0, 5, 1 });
        faces.push_back({ 0, 1, 7 });
        faces.push_back({ 0, 7, 10 });
        faces.push_back({ 0, 10, 11 });

        // 5 adjacent faces 
        faces.push_back({ 1, 5, 9 });
        faces.push_back({ 5, 11, 4 });
        faces.push_back({ 11, 10, 2 });
        faces.push_back({ 10, 7, 6 });
        faces.push_back({ 7, 1, 8 });

        // 5 faces around point 3
        faces.push_back({ 3, 9, 4 });
        faces.push_back({ 3, 4, 2 });
        faces.push_back({3, 2, 6});
        faces.push_back({3, 6, 8});
        faces.push_back({3, 8, 9});

        // 5 adjacent faces 
        faces.push_back({ 4, 9, 5 });
        faces.push_back({ 2, 4, 11 });
        faces.push_back({ 6, 2, 10 });
        faces.push_back({ 8, 6, 7 });
        faces.push_back({ 9, 8, 1 });

        // refine triangles
        for (int i = 0; i < recursionLevel; i++)
        {
            auto faces2 = std::vector<TriangleIndices>();
            for (auto tri : faces)
            {
                // replace triangle by 4 triangles
                uint32_t a = getMiddlePoint(tri.v1, tri.v2);
                uint32_t b = getMiddlePoint(tri.v2, tri.v3);
                uint32_t c = getMiddlePoint(tri.v3, tri.v1);

                faces2.push_back({ tri.v1, a, c });
                faces2.push_back({ tri.v2, b, a });
                faces2.push_back({ tri.v3, c, b });
                faces2.push_back({ a, b, c });
            }
            faces = std::move(faces2);
        }
        for (VertData& vert : geometry) {
            vert.pos.Normalize();
            vert.normal = vert.pos;
        }

        res.geometry = std::move(geometry);
        res.indices = std::move(faces);

        return res;
    }
};
