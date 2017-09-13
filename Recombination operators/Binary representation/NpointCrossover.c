/* ————————————————————————————————————————————————————————————————————————————————————————————————
N-point crossover operator.
———————————————————————————————————————————————————————————————————————————————————————————————————
This is a MEX function performs N-point crossover to generate a specified number of children from a
parent pool of binary chromosomes. This type of crossover is valid for other representations than
binary as well, but this function currently only accepts logical inputs.

For reference, see p. 53 A.  Eiben and J.  Smith, Introduction to evolutionary computing. 
New York: Springer, 2003.

The function takes 3 inputs:
* Input 1: a [m x n] Parentpool matrix of logical values, with one individual per row.
* Input 2: a [1 x 1] scalar 'N' specifying how many crossover points should be used. N ∈ [1,size(Parentpool,2)]
* Input 3: a [1 x 1] scalar 'my' specifying how many new individuals should be generated.

The function outputs 1 variable:
* Output 1: a [my x n] matrix containing the generated children.

Example on how to compile and run from Matlab:
% Compile .C to .mexw64
>> mex NpointCrossover.c

% Run from Matlab when compiled:
>> Parentpool = logical(randi([0 1],10000, 256));
>> N = 2;
>> my = round(size(Parentpool,1)/2);

>> [ Children ] = BitflipMutation( Parentpool , N, my );

Example of compatible C compilers:
* Microsoft Visual C++ 2013 Professional (C)
* Microsoft Visual C++ 2015 Professional (C)
* Intel Parallel Studio XE 2017

Written 2017-09-11 by
petter.stefansson@nmbu.no
———————————————————————————————————————————————————————————————————————————————————————————————— */

#include <mex.h>	// Needed to communicate with matlab.
#include <time.h>   // Needed for counting CPU clock cycle which is used to set seed for rand().

/* ——————————————————————————————————— Function declarations ——————————————————————————————————— */
int randr(unsigned int min, unsigned int max);

int cmpfunc(const void * a, const void * b);

/* ——————————————————————————————————— Matlab gateway start ———————————————————————————————————— */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){

	/* Before starting set the seed of the RNG to the number of clock cycles since start.        */
	srand(clock());

	/* —————————————————————————— Variable type and name declaration ——————————————————————————— */
	const bool *Parentpool;
	int N, my;

	bool *Children; // add conditional checks against other data types?
	double RandNr;
	size_t m, n;

	/* ———————————————————————— Get pointers from the input variables —————————————————————————— */
	Parentpool = mxGetLogicals(prhs[0]);      // Input 1 (Parentpool)
	N          = (int)mxGetScalar(prhs[1]);   // Input 2 (N)
	my         = (int)mxGetScalar(prhs[2]);   // Input 3 (my)

    /* ——————————————————————— Get the dimensions of the input variables ——————————————————————— */
	m = mxGetM(prhs[0]);                      // Number of rows in Parentpool.
	n = mxGetN(prhs[0]);                      // Number of columns in Parentpool.

	/* ——————————————————————————————— Specify Matlab outputs —————————————————————————————————— */
	plhs[0] = mxCreateLogicalMatrix(my, n);
	Children = mxGetLogicals(plhs[0]);

	if (N > n) {
		mexErrMsgIdAndTxt("MATLAB:NpointCrossover:invalidinputs", "Error: Crossover points (N) must be lower than number of genes!");
	}
	if (N <= 0) {
		mexErrMsgIdAndTxt("MATLAB:NpointCrossover:invalidinputs", "Error: Crossover points (N) must be greater or equal to 1!");
	}
	/* ——————————————————————————————————— N-point crossover ———————————————————————————————————— */
	int *CrossOverPoints;
	int j, e, childrow, GeneratedChild;
	int P1, P2, i, CandidatePoint, gene;
	bool AlreadyChosen;
	CrossOverPoints = (int*)malloc(sizeof(int)    * N); // [n-by-1]

	GeneratedChild = 0;
	while (GeneratedChild < my){
		/* ————————————————————————————————————————————————————————————————————————————————————— */
		/* Randomly two pick parents.															 */
		P1 = randr(0, m - 1);
		P2 = randr(0, m - 1);
		while (P1 == P2) {
			P2 = randr(0, m - 1);
		}
		/* ————————————————————————————————————————————————————————————————————————————————————— */
		/* Pick N number of crossover points.													 */
		i = 0;
		while (i < N){
			CandidatePoint = randr(0, n - 1);
			/* Check if the point is already chosen, and in that case dont accept it.			 */
			AlreadyChosen = false;
			for (j = 0; j < i; j++) {
				if (CandidatePoint == CrossOverPoints[j]) {
					AlreadyChosen = true;
				}
			}
			if (AlreadyChosen == false) {
				CrossOverPoints[i] = CandidatePoint;
				i += 1;
			}
		}
		/* ————————————————————————————————————————————————————————————————————————————————————— */
		/* Sort CrossOverPoints from smallest to largest using quicksort.						 */
		qsort(CrossOverPoints, N, sizeof(int), cmpfunc);
		
		/* ————————————————————————————————————————————————————————————————————————————————————— */
		/* Perform crossover while alternating which of the N segments comes from which parent.	 */
		e = 0;
		for (gene = 0; gene < n; gene++) {

			if (gene > CrossOverPoints[e]){
				e++;
			}

			/* Alternate between segments which is parent 1 and 2.								 */
			if ((e % 2) == 0){
				Children[GeneratedChild + gene*my] = Parentpool[P1 + gene*m];
				if (GeneratedChild < my){
					Children[GeneratedChild+1 + gene*my] = Parentpool[P2 + gene*m];
				}
			}
			else {
				Children[GeneratedChild + gene*my] = Parentpool[P2 + gene*m];
				if (GeneratedChild < my) {
					Children[GeneratedChild+1 + gene*my] = Parentpool[P1 + gene*m];
				}
			}
		}
		GeneratedChild++;
	}

	free(CrossOverPoints);
}

/* ————————————————————————————————————————————————————————————————————————————————————————————— */
/* Function for drawing a random integer that lies within range.								 */
int randr(unsigned int min, unsigned int max) {
	return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}
/* ————————————————————————————————————————————————————————————————————————————————————————————— */

/* ————————————————————————————————————————————————————————————————————————————————————————————— */
/* Function used by qsort() to sort vector.														 */
int cmpfunc(const void * a, const void * b){
	return (*(int*)a - *(int*)b);
}
/* ————————————————————————————————————————————————————————————————————————————————————————————— */