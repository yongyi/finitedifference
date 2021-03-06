/*
 * Declaration of the Heat PDE Solver that solves for u(x, tau) in the partial differential equation
 * d(u)/d(tau) = d^2(u)/d(x^2). It will use three methods to solve the PDE:
 *
 *      Forward Euler
 *      Backward Euler
 *      Crank-Nicolson
 *
 * Yongyi Ye
 */

#ifndef HEATPDESOLVER_HPP
#define HEATPDESOLVER_HPP

#include"../linearalgebra/linearsolver.hpp"
#include"../utils/wrapper.hpp"
#include"../blackscholes/checkearlyexercise.hpp"
#include"../blackscholes/gleft.hpp"
#include"../blackscholes/gright.hpp"
#include"../blackscholes/ftau.hpp"

#include<eigen3/Eigen/Dense>
using namespace Eigen;

class HeatPdeSolver{
    /* defining an interface */

    protected:
        double xleft;
        double xright;
        double taufinal;
        Wrapper<Gleft> gleft;
        Wrapper<Gright> gright;
        Wrapper<Ftau> f;

    public:
        /* constructor and destructor */
        HeatPdeSolver(double xleft_, double xright_, double taufinal_,
                        const Gleft &gleft_, const Gright &gright_, const Ftau &f_);
        HeatPdeSolver(const HeatPdeSolver &input);
        virtual ~HeatPdeSolver();
        virtual HeatPdeSolver& operator= (const HeatPdeSolver &input);

        /* the function to solve the pde given boundary conditions by building a (m+1)*(n+1) mesh such that
         * delta-x = (x-right - x-left)/n
         * delta-tau = tau-final/m
         */
        virtual MatrixXd solve_pde(int n, int m) = 0;

        /* virtual copy */
        virtual HeatPdeSolver* clone() const = 0;
};

class ForwardEuler: public HeatPdeSolver{

    public:
        ForwardEuler(double xleft_, double xright_, double taufinal_,
                        const Gleft &gleft_, const Gright &gright_, const Ftau &f_);
        ForwardEuler(const ForwardEuler &input);
        virtual ~ForwardEuler();
        virtual ForwardEuler& operator= (const ForwardEuler &input);

        virtual MatrixXd solve_pde(int n, int m);
        virtual HeatPdeSolver* clone() const;
};

class BackwardEuler: public HeatPdeSolver{

    private:
        Wrapper<LinearSolver> solver;

    public:
        BackwardEuler(double xleft_, double xright_, double taufinal_,
                        const Gleft &gleft_, const Gright &gright_, const Ftau &f_,
                        const LinearSolver &solver_);
        BackwardEuler(const BackwardEuler &input);
        virtual ~BackwardEuler();
        virtual BackwardEuler& operator= (const BackwardEuler &input);

        virtual MatrixXd solve_pde(int n, int m);
        virtual HeatPdeSolver* clone() const;
};

class CrankNicolson: public HeatPdeSolver{

    private:
        Wrapper<LinearSolver> solver;

    public:
        CrankNicolson(double xleft_, double xright_, double taufinal_,
                        const Gleft &gleft_, const Gright &gright_, const Ftau &f_,
                        const LinearSolver &solver_);
        CrankNicolson(const CrankNicolson &input);
        virtual ~CrankNicolson();
        virtual CrankNicolson& operator= (const CrankNicolson &input);

        virtual MatrixXd solve_pde(int n, int m);
        virtual HeatPdeSolver* clone() const;
};


/*** Heat PDE Solver For Early Exercise ***/

class EarlyExerciseSolver: public HeatPdeSolver{
/* solver that takes early exercise into account - used to solve heat pde for American options
 * still an abstract class
 */

    protected:
        Wrapper<CheckEarlyExercise> checker;

    public:
        EarlyExerciseSolver(double xleft_, double xright_, double taufinal_,
                              const Gleft &gleft_, const Gright &gright_, const Ftau &f_,
                              const CheckEarlyExercise &checker_);
        EarlyExerciseSolver(const EarlyExerciseSolver &input);
        virtual ~EarlyExerciseSolver();
        virtual EarlyExerciseSolver& operator= (const EarlyExerciseSolver &input);

        // the solve_pde and clone functions will be defined in its child classes
};

// only forward euler and crank nicolson (with entry-by-entry sor) is suitable for checking and
// updating the nodes for early exercise situations

class EarlyExForwardEuler: public EarlyExerciseSolver{

    public:
        EarlyExForwardEuler(double xleft_, double xright_, double taufinal_,
                              const Gleft &gleft_, const Gright &gright_, const Ftau &f_,
                              const CheckEarlyExercise &checker_);
        EarlyExForwardEuler(const EarlyExForwardEuler &input);
        virtual ~EarlyExForwardEuler();
        virtual EarlyExForwardEuler& operator= (const EarlyExForwardEuler &input);

        virtual MatrixXd solve_pde(int n, int m);
        virtual HeatPdeSolver* clone() const;
};

class EarlyExCrankNicolson: public EarlyExerciseSolver{
    private:
        double w;   // omega for sor for fast convergence
        double tol; // tolerance for sor for the consecutive guess

        MatrixXd projected_sor(MatrixXd b, double alpha, int N, int M, int m);
       /* Projected entry-by-entry SOR iterative method only for use in the Crank-Nicolson Heat PDE solver with
        *      early exercise. The SOR initial guess is hard coded as the early exercise premium values for the specific
        *      pairs of x and tau. The omega and tolerance of SOR are provided with default values of 1.2 and 10^-6.
        *
        * input: b: matrix b such that A * u_(m+1) = b_(m+1), used to calcule u(x, tau) at time m+1, m = 0:M-1
        *      alpha: the Courant constant, on which the entries of the tridiagonal matrix A depends.
        *      N, M : the partition size of the grid of the finite difference scheme (M+1)*(N+1)
        *        m: the current row of tau that sor is trying to solve for u(x_i, tau_m) for i = 1:n-1
        *
        * output: values for u(x, tau) at time m+1
        */

    public:
        EarlyExCrankNicolson(double xleft_, double xright_, double taufinal_,
                              const Gleft &gleft_, const Gright &gright_, const Ftau &f_,
                              const CheckEarlyExercise &checker_,
                              double w_ = 1.2, double tol_ = 0.000001);
        EarlyExCrankNicolson(const EarlyExCrankNicolson &input);
        virtual ~EarlyExCrankNicolson();
        virtual EarlyExCrankNicolson& operator= (const EarlyExCrankNicolson &input);

        virtual MatrixXd solve_pde(int n, int m);
        virtual HeatPdeSolver* clone() const;
};




#endif
