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
    double u;
    bool isBoundary = false;
    std::vector<Atom*> neighboors;
};

class Crystal
{
    public:
    double maxEnergy = 0;
    static constexpr uint maxNeighboors = 20;
    static constexpr double zeroPotentialR = 0.2095;
    static constexpr double potentialCutoffR = 1.8 * zeroPotentialR;
    static constexpr double latticeConst = 0.543;
    static constexpr double atomSize = 0.04 * latticeConst;
    static constexpr int cellAtoms = 8;
    static constexpr double A = 7.05;
    static constexpr double B = 0.602;
    static constexpr double gamma = 1.20;
    static constexpr double lambda = 21.;
    static constexpr double epsilon = 2.17;
    static constexpr double r = 4;
    static constexpr double q = 0;
    glm::uvec3 sizeInCells;
    Crystal(const glm::dvec3 &p, const glm::uvec3 &s);
    void AddLattice(uint i, uint j, uint k);
    int GetLatticeIndex(uint i, uint j, uint k);

    glm::dvec3 p;
    std::vector<Atom> atoms;
    std::vector<std::pair<uint, uint>> bonds;
};

#endif // ATOM_H
