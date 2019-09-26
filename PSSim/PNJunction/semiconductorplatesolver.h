#ifndef SEMICONDUCTORPLATESOLVER_H
#define SEMICONDUCTORPLATESOLVER_H

#include <vector>
#include <cmath>

using namespace std;

struct Problem
{
    int size;
    double left;
    double right;
    double length;
    double nA;
    double nD;
    double n0;
    double eps;


    bool is_right_field;
    bool is_left_field;
    bool is_vach;
    bool is_vfch;
    bool is_phch;
};

class SemiconductorPlateSolver
{
public:
    SemiconductorPlateSolver(Problem & problem_desc)
    {
        size = problem_desc.size;
        left = problem_desc.left;
        right = problem_desc.right;

        nA = problem_desc.nA;
        nD = problem_desc.nD;
        n0 = problem_desc.n0;

        Ld = sqrt((epsSi * epsVac * k * T) / (qe * qe * n0 * (nD + nA)));


        length = problem_desc.length * 1e-6 / Ld;

        step = length / size;
        eps = problem_desc.eps;

        is_right_field = problem_desc.is_right_field;
        is_left_field = problem_desc.is_left_field;
        is_vach = problem_desc.is_vach;
        is_vfch = problem_desc.is_vfch;
        is_phch = problem_desc.is_phch;

        curr_fi.resize(size + 1);
        next_fi.resize(size + 1);
        keys.resize(size + 1);
        a.resize(size + 1);
        b.resize(size + 1);
        psi.resize(size + 1);

        for (int i = 0; i <= size; i++)
        {
            keys[i] = length * i / size;
        }

        for (int i = 0; i <= size; i++)
        {
            next_fi[i] = left + (right - left) * i / size;
        }
    }

    void solveVIChar(vector<double> & solve_buff, vector<double> & keys_buff, double vMin, double vMax)
    {
        solve_buff.resize(size + 1);
        keys_buff.resize(size + 1);
        double vStep = (vMax - vMin) / (size + 1);
        double v = vMin;
        right = 0;
        int iter;
        for(unsigned int i = 0; i <= size; i++, v+= vStep)
        {
            left = v;
            solve(iter);
            solve_buff[i] = 0;
            keys_buff[i] = v;
            for(unsigned int j = 0; j <= size; j++)
            {
                solve_buff[i] = nD * qe * derFi(next_fi, keys, j) * exp(-qekT * next_fi[j]);
            }
        }
    }

    void solve(vector<double> & solve_buff, vector<double> & keys_buff, int & iter)
    {
        solve(iter);
        solve_buff = next_fi;
        keys_buff = keys;
    }

private:
    double derFi(std::vector<double> &values, std::vector<double> &keys, unsigned int num)
    {
        return (values[num + (num == (values.size() - 1) ? 0 : 1)] -
                values[num - (num == 0 ? 0 : 1)]) /
                (keys[num + (num == (keys.size() - 1) ? 0 : 1)] -
                keys[num - (num == 0 ? 0 : 1)]);
    }

    void solve(int & iter)
    {
        iter = 0;
        for (int i = 0; i <= size; i++)
        {
            next_fi[i] = left + (right - left) * i / size;
        }

        double curr_eps = 0;
        do
        {
            curr_fi = next_fi;
            forward();
            backward();
            curr_eps = count_difference();
            iter++;
            if (iter > 100) break;
        } while (curr_eps > eps);
    }

    double fi(double x)
    {
        if (x >= length) return curr_fi[size];

        int num = x / length;

        return curr_fi[num] + (curr_fi[num + 1] - curr_fi[num]) * (x - keys[num]) / step;
    }

    double F(double pot)
    {
        return -(nA * (1.0 - exp(pot)) - nD * (1.0 - exp(-pot)));
    }

    double q(double pot)
    {
        return -(nA * exp(pot) + nD * exp(-pot));
    }

    double r(double pot)
    {
        return  F(pot) + q(pot) * pot;
    }

    void forward()
    {
        if (is_left_field)
        {
            a[0] = 0;
            b[0] = left;

            for (int i = 1; i <= size; i++)
            {
                a[i] = a[i - 1] + (-a[i - 1] * a[i - 1] - q(curr_fi[i - 1])) * step;
                b[i] = b[i - 1] - (a[i - 1] * b[i - 1] - r(curr_fi[i - 1])) * step;
            }

            next_fi[size] = (right - b[size]) / a[size];
            //psi[size] = (next_fi[size] - b[size]) / a[size];
        }
        else
        {
            a[0] = 0;
            b[0] = left;

            for (int i = 1; i <= size; i++)
            {
                a[i] = a[i - 1] + (a[i - 1] * a[i - 1] * q(curr_fi[i - 1]) + 1) * step;
                b[i] = b[i - 1] + (a[i - 1] * b[i - 1] * q(curr_fi[i - 1]) - a[i - 1] * r(curr_fi[i - 1])) * step;
            }

            next_fi[size] = right;
            psi[size] = (next_fi[size] - b[size]) / a[size];
        }
    }

    void backward()
    {
        if (is_left_field)
        {
            for (int i = size - 1; i >= 0; i--)
            {
                next_fi[i] = next_fi[i + 1] - (a[i + 1] * next_fi[i + 1] + b[i + 1]) * step;
            }
        }
        else
        {
            for (int i = size - 1; i >= 0; i--)
            {
                psi[i] = psi[i + 1] - (r(curr_fi[i + 1]) - q(curr_fi[i + 1]) * (a[i + 1] * psi[i + 1] + b[i + 1])) * step;
            }           
            for (int i = 0; i <= size; i++)
            {
                next_fi[i] = a[i] * psi[i] + b[i];
            }
        }
    }

    double count_difference()
    {
        double summ = 0;
        double der = 0;
        for (int i = 0; i <= size; i++)
        {
            summ += (next_fi[i] - curr_fi[i]) * (next_fi[i] - curr_fi[i]);
            der += next_fi[i] * next_fi[i];
        }

        return summ;
    }



private:
    const double qe = -1.6e-19;
    const double k = 1.38064852e-23;
    const double T = 300;
    const double qekT = qe/ k/ T;
    const double epsSi = 12;
    const double epsVac = 8.86e-12;
    double Ld;

    int size;
    double left;
    double right;
    double length;
    double nA;

    double n0;
    double nD;

    double step;
    double eps;


    bool is_right_field;
    bool is_left_field;
    bool is_vach;
    bool is_vfch;
    bool is_phch;

    vector <double> curr_fi;
    vector <double> next_fi;
    vector <double> keys;

    vector <double> a;
    vector <double> b;

    vector <double> psi;
};


#endif // SEMICONDUCTORPLATESOLVER_H
