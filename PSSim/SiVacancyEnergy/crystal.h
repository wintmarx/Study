#ifndef ATOM_H
#define ATOM_H

#include "glm/glm.hpp"
#include <vector>

typedef unsigned int uint;

class Atom
{
public:
    Atom(const glm::dvec3 &p);
    glm::dvec3 p;
    glm::dvec3 v;
    bool isBoundary = false;
};

class Crystal
{
    public:
    static constexpr double cellSize = 1.;
    static constexpr double atomSize = 0.04 * cellSize;
    static constexpr int cellAtoms = 8;
    glm::uvec3 sizeInCells;
    Crystal(const glm::dvec3 &p, const glm::uvec3 &s);
    void AddCell(uint i, uint j, uint k);
    int GetCellIndex(uint i, uint j, uint k);

    glm::dvec3 p;
    std::vector<Atom> atoms;
    std::vector<std::pair<uint, uint>> bonds;
};

#endif // ATOM_H
