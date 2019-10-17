#include "crystal.h"

#include <QDebug>

Atom::Atom(const glm::dvec3 &p) :
    p(p),
    v(0.),
    f(0.)
{
    neighboors.reserve(Crystal::maxNeighboors);
}

Crystal::Crystal(const glm::dvec3 &p, const glm::uvec3 &s) :
    p(p)
{
    sizeInCells = {s.x ? s.x : 1, s.y ? s.y : 1, s.z ? s.z : 1};

    uint amount = sizeInCells.x * sizeInCells.y * sizeInCells.z * cellAtoms;
    atoms.reserve(amount);
    bonds.reserve(amount * 4);

    for (uint y = 0; y < sizeInCells.y; y++)
    {
        for (uint z = 0; z < sizeInCells.z; z++)
        {
            for (uint x = 0; x < sizeInCells.x; x++)
            {
                AddLattice(x, y, z);
            }
        }
    }
    for (uint i = 0; i < atoms.size(); i++)
    {
        int bCount = 0;
        for(uint j = 0; j < bonds.size(); j++)
        {
            if (bonds[j].first == i || bonds[j].second == i)
            {
               bCount++;
            }
        }
        if (bCount < 4)
        {
           atoms[i].isBoundary = true;
        }
        for (uint j = i + 1; j < atoms.size(); j++)
        {
            if (glm::length(atoms[i].p - atoms[j].p) < 1.5 * Crystal::potentialCutoffR)
            {
                atoms[i].neighboors.push_back(&atoms[j]);
                atoms[j].neighboors.push_back(&atoms[i]);
            }
        }
    }
}

void Crystal::AddLattice(uint i, uint j, uint k)
{
    glm::dvec3 p = this->p + glm::dvec3(i * latticeConst, j * latticeConst, k * latticeConst);
    uint o = atoms.size();
    atoms.emplace_back(p);                                                                           //0
    atoms.emplace_back(p + glm::dvec3(.50 * latticeConst, .00 * latticeConst, .50 * latticeConst));  //1
    atoms.emplace_back(p + glm::dvec3(.25 * latticeConst, .25 * latticeConst, .25 * latticeConst));  //2
    atoms.emplace_back(p + glm::dvec3(.75 * latticeConst, .25 * latticeConst, .75 * latticeConst));  //3
    atoms.emplace_back(p + glm::dvec3(.50 * latticeConst, .50 * latticeConst, .00 * latticeConst));  //4
    atoms.emplace_back(p + glm::dvec3(.00 * latticeConst, .50 * latticeConst, .50 * latticeConst));  //5
    atoms.emplace_back(p + glm::dvec3(.75 * latticeConst, .75 * latticeConst, .25 * latticeConst));  //6
    atoms.emplace_back(p + glm::dvec3(.25 * latticeConst, .75 * latticeConst, .75 * latticeConst));  //7

    bonds.emplace_back(o, o + 2);
    bonds.emplace_back(o + 1, o + 2);
    bonds.emplace_back(o + 4, o + 2);
    bonds.emplace_back(o + 5, o + 2);
    bonds.emplace_back(o + 1, o + 3);
    bonds.emplace_back(o + 4, o + 6);
    bonds.emplace_back(o + 5, o + 7);

    int l = GetLatticeIndex(i + 1, j, k);
    if (l >= 0)
    {
        bonds.emplace_back(o + 6, l + 5);
        bonds.emplace_back(o + 3, l + 5);
        //bonds.push_back(std::pair<uint, uint>(o, l));
    }
    l = GetLatticeIndex(i, j + 1, k);
    if (l >= 0)
    {
        bonds.emplace_back(o + 6, l + 1);
        bonds.emplace_back(o + 7, l + 1);
        //bonds.push_back(std::pair<uint, uint>(o, l));
    }
    l = GetLatticeIndex(i + 1, j + 1, k);
    if (l >= 0)
    {
        bonds.emplace_back(o + 6, l);
    }
    l = GetLatticeIndex(i, j + 1, k + 1);
    if (l >= 0)
    {
        bonds.emplace_back(o + 7, l);
    }
    l = GetLatticeIndex(i, j, k + 1);
    if (l >= 0)
    {
        bonds.emplace_back(o + 7, l + 4);
        bonds.emplace_back(o + 3, l + 4);
        //bonds.push_back(std::pair<uint, uint>(o, l));
    }
    l = GetLatticeIndex(i + 1, j, k + 1);
    if (l >= 0)
    {
        bonds.emplace_back(o + 3, l);
    }
}

int Crystal::GetLatticeIndex(uint i, uint j, uint k)
{
    if (i >= sizeInCells.x || j >= sizeInCells.y || k >= sizeInCells.z)
    {
       return -1;
    }
    return cellAtoms * (j * sizeInCells.x * sizeInCells.z + k * sizeInCells.x + i);
}
