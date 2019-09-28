#include "crystal.h"

#include <QDebug>

Atom::Atom(const glm::dvec3 &p) :
    p(p),
    v(0.)
{

}

Crystal::Crystal(const glm::dvec3 &p, const glm::uvec3 &s) :
    p(p)
{
    sizeInCells = {s.x ? s.x : 1, s.y ? s.y : 1, s.z ? s.z : 1};
    /*this->p.x = p.x - sizeInCells.x * cellSize / 2.;
    this->p.y = p.y - sizeInCells.y * cellSize / 2.;
    this->p.z = p.z - sizeInCells.z * cellSize / 2.;*/

    uint amount = sizeInCells.x * sizeInCells.y * sizeInCells.z * cellAtoms;
    atoms.reserve(amount);
    bonds.reserve(amount * 4);

    for (uint y = 0; y < sizeInCells.y; y++)
    {
        for (uint z = 0; z < sizeInCells.z; z++)
        {
            for (uint x = 0; x < sizeInCells.x; x++)
            {
                AddCell(x, y, z);
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
    }
}

void Crystal::AddCell(uint i, uint j, uint k)
{
    glm::dvec3 p = this->p + glm::dvec3(i * cellSize, j * cellSize, k * cellSize);
    uint o = atoms.size();
    atoms.push_back(Atom(p));                                                               //0
    atoms.push_back(Atom(p + glm::dvec3(.50 * cellSize, .00 * cellSize, .50 * cellSize)));  //1
    atoms.push_back(Atom(p + glm::dvec3(.25 * cellSize, .25 * cellSize, .25 * cellSize)));  //2
    atoms.push_back(Atom(p + glm::dvec3(.75 * cellSize, .25 * cellSize, .75 * cellSize)));  //3
    atoms.push_back(Atom(p + glm::dvec3(.50 * cellSize, .50 * cellSize, .00 * cellSize)));  //4
    atoms.push_back(Atom(p + glm::dvec3(.00 * cellSize, .50 * cellSize, .50 * cellSize)));  //5
    atoms.push_back(Atom(p + glm::dvec3(.75 * cellSize, .75 * cellSize, .25 * cellSize)));  //6
    atoms.push_back(Atom(p + glm::dvec3(.25 * cellSize, .75 * cellSize, .75 * cellSize)));  //7

    bonds.push_back(std::pair<uint, uint>(o, o + 2));
    bonds.push_back(std::pair<uint, uint>(o + 1, o + 2));
    bonds.push_back(std::pair<uint, uint>(o + 4, o + 2));
    bonds.push_back(std::pair<uint, uint>(o + 5, o + 2));
    bonds.push_back(std::pair<uint, uint>(o + 1, o + 3));
    bonds.push_back(std::pair<uint, uint>(o + 4, o + 6));
    bonds.push_back(std::pair<uint, uint>(o + 5, o + 7));

    int l = GetCellIndex(i + 1, j, k);
    if (l >= 0)
    {
        bonds.push_back(std::pair<uint, uint>(o + 6, l + 5));
        bonds.push_back(std::pair<uint, uint>(o + 3, l + 5));
        //bonds.push_back(std::pair<uint, uint>(o, l));
    }
    l = GetCellIndex(i, j + 1, k);
    if (l >= 0)
    {
        bonds.push_back(std::pair<uint, uint>(o + 6, l + 1));
        bonds.push_back(std::pair<uint, uint>(o + 7, l + 1));
        //bonds.push_back(std::pair<uint, uint>(o, l));
    }
    l = GetCellIndex(i + 1, j + 1, k);
    if (l >= 0)
    {
        bonds.push_back(std::pair<uint, uint>(o + 6, l));
    }
    l = GetCellIndex(i, j + 1, k + 1);
    if (l >= 0)
    {
        bonds.push_back(std::pair<uint, uint>(o + 7, l));
    }
    l = GetCellIndex(i, j, k + 1);
    if (l >= 0)
    {
        bonds.push_back(std::pair<uint, uint>(o + 7, l + 4));
        bonds.push_back(std::pair<uint, uint>(o + 3, l + 4));
        //bonds.push_back(std::pair<uint, uint>(o, l));
    }
    l = GetCellIndex(i + 1, j, k + 1);
    if (l >= 0)
    {
        bonds.push_back(std::pair<uint, uint>(o + 3, l));
    }
}

int Crystal::GetCellIndex(uint i, uint j, uint k)
{
    if (i >= sizeInCells.x || j >= sizeInCells.y || k >= sizeInCells.z)
    {
       return -1;
    }
    return cellAtoms * (j * sizeInCells.x * sizeInCells.z + k * sizeInCells.x + i);
}
