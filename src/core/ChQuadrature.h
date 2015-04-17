//
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2013 Project Chrono
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file at the top level of the distribution
// and at http://projectchrono.org/license-chrono.txt.
//
//
// File author: Alessandro Tasora

#ifndef CHQUADRATURE
#define CHQUADRATURE

#include <vector>
#include "core/ChMath.h"

namespace chrono {

/// Class to store polynomial roots and weights for the Gauss-Legendre
/// quadrature. It is automatically managed by ChQuadrature.
class ChApi ChQuadratureTables {
  public:
    ChQuadratureTables(int order_from = 1, int order_to = 10);

    std::vector<std::vector<double> > Weight;
    std::vector<std::vector<double> > Lroots;

    void PrintTables();

  private:
    void glege_coef(ChMatrix<>& lcoef, int N);
    void glege_roots(ChMatrix<>& lcoef, int N, int ntable);
    double glege_eval(int n, double x, ChMatrix<>& lcoef);
    double glege_diff(int n, double x, ChMatrix<>& lcoef);
};

/// Base class for 1D integrand T=f(x) to be used in ChQuadrature.
/// Since the class is templated, the computed valued can be either
/// a simple 'double' or a more complex object, like ChMatrixNM<..>.
/// You must inherit your custom class from this base class, and
/// implement your own Evaluate() method, for example:
///
/// class MySine : public ChIntegrable1D<double>
/// {
///  public:
///		void Evaluate (double& result, const double x)  { result = sin(x); }
/// };
///
template <class T = double>
class ChIntegrable1D {
  public:
    /// Evaluate the function at point x , that is
    /// result T = f(x)
    virtual void Evaluate(T& result, const double x) = 0;
};

/// As ChIntegrable1D, but for 2D integrand T=f(x,y)
/// to be used in ChQuadrature.
template <class T = double>
class ChIntegrable2D {
  public:
    /// Evaluate the function at point x,y , that is
    /// result T = f(x,y)
    virtual void Evaluate(T& result, const double x, const double y) = 0;
};

/// As ChIntegrable1D, but for 3D integrand T=f(x,y,z)
/// to be used in ChQuadrature.
template <class T = double>
class ChIntegrable3D {
  public:
    /// Evaluate the function at point x,y,z , that is
    /// result T = f(x,y,z)
    virtual void Evaluate(T& result, const double x, const double y, const double z) = 0;
};

/// Class to perform Gauss-Legendre quadrature, in 1D, 2D, 3D.
/// It integrates a function on a nD domain using the Gauss quadrature; this is mostly
/// useful in the case that the integrand is polynomial, because the result is exact
/// if the order of quadrature is greater or equal to the degree of the polynomial.
/// Integrate() functions are static; no need to allocate an instance of this class.

class ChApi ChQuadrature {
  public:
    /// Integrate the integrand T = f(x) over the 1D interval [xA, xB],
    /// with desired order of quadrature. Best if integrand is polynomial.
    /// For order in 1..10, precomputed polynomial coefficients are used for max speed.
    template <class T>
    static void Integrate1D(T& result,                     ///< result is returned here
                            ChIntegrable1D<T>& integrand,  ///< this is the integrand
                            const double a,                ///< min limit for x domain
                            const double b,                ///< min limit for x domain
                            const int order)               ///< order of integration
    {
        ChQuadratureTables* mtables = 0;
        std::vector<double>* lroots;
        std::vector<double>* weight;
        bool static_tables;

        if ((unsigned int)order <= GetStaticTables()->Lroots.size()) {
            mtables = GetStaticTables();
            lroots = &mtables->Lroots[order - 1];
            weight = &mtables->Weight[order - 1];
            static_tables = true;
        } else {
            mtables = new ChQuadratureTables(order, order);
            mtables->PrintTables();
            lroots = &mtables->Lroots[0];
            weight = &mtables->Weight[0];
            static_tables = false;
        }

        double c1 = (b - a) / 2;
        double c2 = (b + a) / 2;

        result *= 0;  // as result = 0, but works also for matrices.
        T val;        // temporary value for loop

        for (unsigned int i = 0; i < lroots->size(); i++) {
            integrand.Evaluate(val, (c1 * lroots->at(i) + c2));
            val *= weight->at(i);
            result += val;
        }
        result *= c1;  // result = c1 * sum;

        if (!static_tables)
            delete mtables;
    }

    /// Integrate the integrand T = f(x,y) over the 2D interval [xA, xB][yA, yB],
    /// with desired order of quadrature. Best if integrand is polynomial.
    /// For order in 1..10, precomputed polynomial coefficients are used for max speed.
    template <class T>
    static void Integrate2D(T& result,                     ///< result is returned here
                            ChIntegrable2D<T>& integrand,  ///< this is the integrand
                            const double Xa,               ///< min limit for x domain
                            const double Xb,               ///< min limit for x domain
                            const double Ya,               ///< min limit for y domain
                            const double Yb,               ///< min limit for y domain
                            const int order)               ///< order of integration
    {
        ChQuadratureTables* mtables = 0;
        std::vector<double>* lroots;
        std::vector<double>* weight;
        bool static_tables;

        if ((unsigned int)order <= GetStaticTables()->Lroots.size()) {
            mtables = GetStaticTables();
            lroots = &mtables->Lroots[order - 1];
            weight = &mtables->Weight[order - 1];
            static_tables = true;
        } else {
            mtables = new ChQuadratureTables(order, order);
            mtables->PrintTables();
            lroots = &mtables->Lroots[0];
            weight = &mtables->Weight[0];
            static_tables = false;
        }

        double Xc1 = (Xb - Xa) / 2;
        double Xc2 = (Xb + Xa) / 2;
        double Yc1 = (Yb - Ya) / 2;
        double Yc2 = (Yb + Ya) / 2;

        result *= 0;  // as result = 0, but works also for matrices.
        T val;        // temporary value for loop

        for (unsigned int ix = 0; ix < lroots->size(); ix++)
            for (unsigned int iy = 0; iy < lroots->size(); iy++) {
                integrand.Evaluate(val, (Xc1 * lroots->at(ix) + Xc2), (Yc1 * lroots->at(iy) + Yc2));
                val *= (weight->at(ix) * weight->at(iy));
                result += val;
            }
        result *= (Xc1 * Yc1);

        if (!static_tables)
            delete mtables;
    }

    /// Integrate the integrand T = f(x,y,z) over the 3D interval [xA, xB][yA, yB][zA, zB],
    /// with desired order of quadrature. Best if integrand is polynomial.
    /// For order in 1..10, precomputed polynomial coefficients are used for max speed.
    template <class T>
    static void Integrate3D(T& result,                     ///< result is returned here
                            ChIntegrable3D<T>& integrand,  ///< this is the integrand
                            const double Xa,               ///< min limit for x domain
                            const double Xb,               ///< min limit for x domain
                            const double Ya,               ///< min limit for y domain
                            const double Yb,               ///< min limit for y domain
                            const double Za,               ///< min limit for z domain
                            const double Zb,               ///< min limit for z domain
                            const int order)               ///< order of integration
    {
        ChQuadratureTables* mtables = 0;
        std::vector<double>* lroots;
        std::vector<double>* weight;
        bool static_tables;

        if ((unsigned int)order <= GetStaticTables()->Lroots.size()) {
            mtables = GetStaticTables();
            lroots = &mtables->Lroots[order - 1];
            weight = &mtables->Weight[order - 1];
            static_tables = true;
        } else {
            mtables = new ChQuadratureTables(order, order);
            mtables->PrintTables();
            lroots = &mtables->Lroots[0];
            weight = &mtables->Weight[0];
            static_tables = false;
        }

        double Xc1 = (Xb - Xa) / 2;
        double Xc2 = (Xb + Xa) / 2;
        double Yc1 = (Yb - Ya) / 2;
        double Yc2 = (Yb + Ya) / 2;
        double Zc1 = (Zb - Za) / 2;
        double Zc2 = (Zb + Za) / 2;

        result *= 0;  // as result = 0, but works also for matrices.
        T val;        // temporary value for loop

        for (unsigned int ix = 0; ix < lroots->size(); ix++)
            for (unsigned int iy = 0; iy < lroots->size(); iy++)
                for (unsigned int iz = 0; iz < lroots->size(); iz++) {
                    integrand.Evaluate(val, (Xc1 * lroots->at(ix) + Xc2), (Yc1 * lroots->at(iy) + Yc2),
                                       (Zc1 * lroots->at(iz) + Zc2));
                    val *= (weight->at(ix) * weight->at(iy) * weight->at(iz));
                    result += val;
                }
        result *= (Xc1 * Yc1 * Zc1);

        if (!static_tables)
            delete mtables;
    }

    /// For debugging/testing, prints the Legendre weights/roots
    /// in cout.
    static ChQuadratureTables* GetStaticTables();
};
}

#endif
