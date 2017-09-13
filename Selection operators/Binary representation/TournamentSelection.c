/* ————————————————————————————————————————————————————————————————————————————————————————————————
Tournament selection operator.
———————————————————————————————————————————————————————————————————————————————————————————————————
This is a MEX function which takes a population of chromosomes as input, together with their
fitness values, and performs tournament selection to select who dies and who survives. The 
survivors are then returnes together with their fitness values.

% For reference, see p. 84-85 A.E Eiben Introduction to evoulutionary computing.

The function takes 5 inputs:
* Input 1: a [1 x 1] scalar 'k' specifying how many contenders are involved in each tournament.
* Input 2: a [m x 1] vector 'Fitness' containing the fitness of each individual, higher better.
* Input 3: a [m x n] boolean matrix 'Population' containing the population.
* Input 4: a [1 x 1] scalar 'NoSurvivors' specifying the number of survivors after selection.
* Input 5: a [1 x 1] scalar 'Eliterows' specifying how many rows, starting from the top, should
be excluded from the selection process due to elitism.

The function outputs 2 variables:
* Output 1: a [NoSurvivors x n] boolean matrix containing the survivors.
* Output 2: a [NoSurvivors x 1] vector with the fitness of the survivors.

Example on how to compile and run from Matlab:
% Compile .C to .mexw64
>> mex TournamentSelection.c

% Run from Matlab when compiled:
>> k = 3;
>> Fitness = rand(100,1);
>> Population = logical(randi([0 1],100, 256));
>> NoSurvivors = 50;
>> Eliterows = 2;

>> [ Survivors, SurvivorFitness ] = TournamentSelection( k, Fitness, Population, NoSurvivors, Eliterows );

Example of compatible C compilers:
* Microsoft Visual C++ 2013 Professional (C)
* Microsoft Visual C++ 2015 Professional (C)
* Intel Parallel Studio XE 2017

Written 2017-09-12 by
petter.stefansson@nmbu.no
———————————————————————————————————————————————————————————————————————————————————————————————— */

#include <mex.h>	// Needed to communicate with matlab
#include <time.h>   // Needed for counting CPU clock cycle which is used to set seed for rand()

/* ——————————————————————————————————— Function declarations ——————————————————————————————————— */
void TourSel(int k, const double *Fitness, const bool *Population, int NoSurvivors, int Eliterows, size_t m, size_t n, bool *Survivors, double *SurvivorFitness);

int randr(unsigned int min, unsigned int max);

/* ——————————————————————————————————— Matlab gateway start ———————————————————————————————————— */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){

	/* Before starting set the seed of the RNG to the number of clock cycles since start.        */
	srand(clock());

	/* —————————————————————————— Variable type and name declaration ——————————————————————————— */
	int k, NoSurvivors, Eliterows;
	const double *Fitness;
	const bool *Population;

	double *SurvivorFitness;
	bool *Survivors;

	size_t m, n;

	/* ———————————————————————— Get pointers from the input variables —————————————————————————— */
	k           = (int)mxGetScalar(prhs[0]);  // Input 1 (k)
	Fitness     = mxGetPr(prhs[1]);           // Input 2 (Fitness)
	Population  = mxGetLogicals(prhs[2]);     // Input 3 (Population)
	NoSurvivors = (int)mxGetScalar(prhs[3]);  // Input 4 (Number of survivors)
	Eliterows   = (int)mxGetScalar(prhs[4]);  // Input 5 (Number of elitism rows)

    /* ——————————————————————— Get the dimensions of the input variables ——————————————————————— */
	m = mxGetM(prhs[2]);                      // Number of rows in Population.
	n = mxGetN(prhs[2]);                      // Number of columns in Population.

	/* ——————————————————————————————— Specify Matlab outputs —————————————————————————————————— */
	plhs[0] = mxCreateLogicalMatrix(NoSurvivors, n);
	Survivors = mxGetLogicals(plhs[0]);
	plhs[1] = mxCreateDoubleMatrix(NoSurvivors, 1, mxREAL);
	SurvivorFitness = mxGetPr(plhs[1]);
	
	/* ——————————————————————————————— Tournament selection ———————————————————————————————————— */
	
	            TourSel(k,
		          Fitness, 
		       Population, 
		      NoSurvivors, 
		        Eliterows, 
		                m, 
		                n, 
		        Survivors,
		  SurvivorFitness);
	
}

/* ————————————————————————————————————————————————————————————————————————————————————————————— */
/* Function for performing tournament selection.												 */
void TourSel(int k, const double *Fitness, const bool *Population, int NoSurvivors, int Eliterows, size_t m, size_t n, bool *Survivors, double *SurvivorFitness){

	int Tournament, Contender, ContenderIndex, row, col, WinnerIndex;
	int *ContenderList;
	bool AlreadyInTour;
	double Winner;
	double *ContenderFitnessList;

	ContenderList        = (int*)malloc(sizeof(int)          * k); 
	ContenderFitnessList = (double*)malloc(sizeof(double)    * k);

	/* Hold tournaments until 'NoSurvivors' has been found.										 */
	for (Tournament = 0; Tournament < NoSurvivors; Tournament++) {
		
		/* Randomly pick k contenders.															 */
		Contender = 0;
		while (Contender < k){
			/* Draw a contender.																 */
			ContenderIndex = randr(Eliterows, m-1);
			
			/* Check if it is already in the contender list.									 */
			AlreadyInTour = false;
			for (row = 0; row < Contender; row++) {
				if (ContenderIndex == ContenderList[row]) {
					AlreadyInTour = true;
				}
			}

			/* If the contender was not already on the list - add it, and its fitness.           */
			if (AlreadyInTour == false) {
				ContenderList[Contender] = ContenderIndex;
				ContenderFitnessList[Contender] = Fitness[ContenderIndex];
				Contender++;
			}
		}


		/* Find winner of the tournament.														 */
		for (row = 0; row < k; row++) {
			if (row == 0) {
				Winner = ContenderFitnessList[row];
				WinnerIndex = ContenderList[row];
			}
			else {
				if (ContenderFitnessList[row] > Winner) {
					Winner = ContenderFitnessList[row];
					WinnerIndex = ContenderList[row];
				}
			}
		}

		/* Extract the winner and place it in the pool of Survivors together with its fitness.   */
		SurvivorFitness[Tournament] = Winner;
		for (col = 0; col < n; col++){
			Survivors[Tournament + NoSurvivors * col] = Population[WinnerIndex + m * col];
		}

	}

	free(ContenderList);
	free(ContenderFitnessList);
}
/* ————————————————————————————————————————————————————————————————————————————————————————————— */
/* Function for drawing a random integer that lies within range.								 */
int randr(unsigned int min, unsigned int max) {
	return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}
/* ————————————————————————————————————————————————————————————————————————————————————————————— */