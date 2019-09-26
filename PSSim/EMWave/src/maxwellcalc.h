#ifndef MAXWELLCALC_H
#define MAXWELLCALC_H

#include <vector>
#include <QVector3D>

typedef unsigned int uint;

struct SystemParams
{
    float gridStep;
    float gridSize;
    uint gridStepsCount;
    float dt;
    uint timeStepsCount;

    uint pmlCount;

    float mPermMetal; //mu
    float dPermMetal; //epsilon
    float eCondMetal; //sigma

    float mPermVacuum; //mu
    float dPermVacuum; //epsilon
    float eCondVacuum; //sigma

    float mPermPML; //mu
    float dPermPML; //epsilon
    float eCondPML; //sigma

    float chargeEx;
    float chargeEy;
};

typedef enum {
    VACUUM,
    CHARGE,
    METAL
} GridPointType;

struct GridPoint
{
    GridPoint() : e(0.,0.,0.), h(0.,0.,0.), mPerm(0), dPerm(0), eCond(0) {}
    QVector3D e;
    QVector3D h;
    float mPerm; //mu
    float dPerm; //epsilon
    float eCond; //sigma
    GridPointType type;
};

class MaxwellCalc
{
public:
    MaxwellCalc();
    bool Calc(SystemParams &p);
    SystemParams p;
    std::vector<std::vector<std::vector<GridPoint>>> grid;
    bool isReady;
private:
    float Hz(uint x, uint y, uint t);
    float Ex(uint x, uint y, uint t);
    float Ey(uint x, uint y, uint t);
    float HzyPML(uint x, uint y, uint t);
    float HzxPML(uint x, uint y, uint t);
    float ExPML(uint x, uint y, uint t);
    float EyPML(uint x, uint y, uint t);
};

#endif // MAXWELLCALC_H
