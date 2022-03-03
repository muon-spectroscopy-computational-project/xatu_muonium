#pragma once
#include <armadillo>
#include <complex>
#include <omp.h>
#include <stdlib.h>

#include "Zigzag.hpp"
#include "System.hpp"
#include "ExcitonConfiguration.hpp"
#include "Result.hpp"

#ifndef constants
#define PI 3.141592653589793
#define ec 1.6021766E-19
#define eps0 8.8541878E-12
#endif


// Routines to initialize BSE elements
class GExciton : public System {

    // ----------------------------------- Attributes -----------------------------------
    private:
        // Read-only parameters
        int ncell_, totalCells_, nbands_, nrmbands_, excitonbasisdim_;
        double eps_m_, eps_s_, r0_;
        arma::ivec bands_, valenceBands_, conductionBands_;
        arma::uvec bandList_;
        arma::imat basisStates_;
        arma::rowvec Q_;
        double cutoff_;
        arma::cx_mat HBS_;

        // Internal attributes
        arma::mat eigvalKStack_, eigvalKQStack_;
        arma::cx_cube eigvecKStack_, eigvecKQStack_;
        arma::cx_mat ftStack;
        std::complex<double> ftX;
        arma::mat potentialMat;
        arma::mat HK_;
        
        double pairEnergy;
        

    public:
        // Number of unit cells along one axis
        const int& ncell = ncell_;
        // Total number of unit cells
        const int& totalCells = totalCells_;
        // Dimension of electron-hole pair basis used to build excitons
        const int& excitonbasisdim = excitonbasisdim_;
        // Number of bands participating in exciton formation, starting from the Fermi level
        const int& nbands = nbands_;
        // Remove bands starting from the Fermi level to build excited excitons directly
        const int& nrmbands = nrmbands_;
        // List of bands used to build the exciton
        const arma::ivec& bands = bands_;
        // List of bands used to build the exciton relative to the Fermi level
        const arma::uvec& bandList = bandList_;
        // 3d array with the center-of-mass momentum of the exciton
        const arma::rowvec& Q = Q_;
        // List of valence bands that form the exciton relative to the Fermi level
        const arma::ivec& valenceBands = valenceBands_;
        // List of conduction bands that form the exciton relative to the Fermi level
        const arma::ivec& conductionBands = conductionBands_;
        // Returns Bethe-Salpeter Hamiltonian
        const arma::cx_mat& HBS = HBS_;
        // Returns kinetic term of BSE
        const arma::mat& HK = HK_;
        // Returns dielectric constant of embedding medium
        const double& eps_m = eps_m_;
        // Returns dielectric constante of substrate
        const double& eps_s = eps_s_;
        // Returns effective screening length r0
        const double& r0 = r0_;
        const arma::mat& eigvalKStack = eigvalKStack_;
        const arma::mat& eigvalKQStack = eigvalKQStack_;
        const arma::cx_cube& eigvecKStack = eigvecKStack_;
        const arma::cx_cube& eigvecKQStack = eigvecKQStack_;
        const arma::imat& basisStates = basisStates_;

        // BEWARE: This dictionary had to be exposed to be able to access it,
        // do not call from outside of methods.
        std::map<int, int> bandToIndex;

    // ----------------------------------- Methods -----------------------------------
    // Constructor & Destructor
    public:
        //// Overload constructor:
        // Default constructor can not be called (system file is always required)
        GExciton();

        // Specify number of bands participating (int)
        GExciton(std::string filename, int ncell = 20, int nbands = 1, int nrmbands = 0, 
                const arma::rowvec& parameters = {1, 5, 1}, const arma::rowvec& Q = {0., 0., 0.});

        // Specify which bands participate (vector with band numbers)
        GExciton(std::string filename, int ncell = 200, const arma::ivec& bands = {0, 1}, 
                const arma::rowvec& parameters = {1, 5, 1}, const arma::rowvec& Q = {0., 0., 0.});
        
        // Use two files: the mandatory one for system config., and one for exciton config.
        GExciton(std::string systemfile, std::string excitonfile);
        ~GExciton();

        // Setters
        void setUnitCells(int);
        void setBands(int, int);
        void setBands(const arma::ivec&);
        void setQ(const arma::rowvec&);
        void setParameters(const arma::rowvec&);
        void setParameters(double, double, double);
        void setCutoff(double);

    private:
        // Methods for BSE matrix initialization
        void STVH0(double, double*);
        double potential(double);
        std::complex<double> fourierTransform(arma::rowvec k, const arma::mat&, bool useApproximation = true);
        std::complex<double> tDirect(std::complex<double>,
                                     const arma::cx_vec&, 
                                     const arma::cx_vec&,
                                     const arma::cx_vec&, 
                                     const arma::cx_vec&);
        std::complex<double> tExchange(std::complex<double>, 
                                    const arma::cx_vec&, 
                                    const arma::cx_vec&,
                                    const arma::cx_vec&, 
                                    const arma::cx_vec&);
        std::complex<double> exactInteractionTerm(const arma::cx_vec&, 
                                 const arma::cx_vec&,
                                 const arma::cx_vec&, 
                                 const arma::cx_vec&,
                                 const arma::rowvec&);

        // Initializers
        void initializeExcitonAttributes(const ExcitonConfiguration&);
        void initializeBasis();
        void initializeResultsH0();
        void initializePotentialMatrix();
        
        // Utilities
        void generateBandDictionary();
        void createMesh();
        void fixBandCrossing(arma::vec&, arma::cx_mat&);
        int determineKIndex(double k);
        arma::cx_cube atomicGCoefs(const arma::cx_cube&);

        // Routines to compute Fermi Golden Rule
        arma::cx_mat fixDegeneracyIteration(const arma::cx_vec&, const arma::cx_vec&);
        arma::cx_vec ehPairCoefs(double, const arma::vec&, bool zone = true);

    public:
        arma::imat createBasis(const arma::ivec&, const arma::ivec&);
        arma::imat specifyBasisSubset(const arma::ivec& bands);
        void useSpinfulBasis();
        
        // BSE initialization and energies
        void initializeHamiltonian(bool useApproximation = true);
        void BShamiltonian(const arma::imat& basis = {}, bool useApproximation = true);
        Result diagonalize();

        // Fermi golden rule        
        arma::cx_vec wavePacket(double, double);
        arma::cx_mat fixDegeneracy(const arma::cx_vec&, const arma::cx_vec&, int iterations = 5);
        double pairDensityOfStates(const arma::ivec&, const arma::ivec&, double, double);
        double fermiGoldenRule(const arma::cx_vec&, double);  
};


