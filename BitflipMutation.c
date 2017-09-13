/* ————————————————————————————————————————————————————————————————————————————————————————————————
Bitflip mutation operator.
———————————————————————————————————————————————————————————————————————————————————————————————————
This is a MEX function which loops through all genes of a population of chromosomes (boolean matrix) 
and changes its binary value from 0 to 1 (or 1 to 0) according to a mutation probability.This type 
of mutation is primarily suitable for binary representation GA's.

The function takes a population matrix and a mutation probability (Pm) as input together with 
an scalar (ElitismNo) representing how many rows from the top of the population should be excluded 
from the mutation (in case of elitism) and returns a mutated population matrix of same size as the 
original.

For reference, see p. 52 A.  Eiben and J.  Smith, Introduction to evolutionary computing. 
New York: Springer, 2003.

The function takes 3 inputs:
* Input 1: a [m x n] population matrix of logical values, with one individual per row.
* Input 2: a [1 x 1] scalar 'Pm' specifying a mutation probability between 0 and 1.
* Input 3: a [1 x 1] scalar 'ElitismNo' specifying how many individuals, starting from the top row
should be excluded from the mutation process. If set to 0 all individuals are mutated. If set to 1
the first chromosome of the population is skipped in the mutation process etc.

The function outputs 1 variable:
* Output 1: a [m x n] boolean matrix containing the mutated population.

Example on how to compile and run from Matlab:
% Compile .C to .mexw64
>> mex BitflipMutation.c

% Run from Matlab when compiled:
>> Population = logical(randi([0 1],10000, 256));
>> Pm = 1/256;
>> ElitismNo = 3;

>> [ MutatedPopulation ] = BitflipMutation( Population , Pm, ElitismNo );

Example of compatible C compilers:
* Microsoft Visual C++ 2013 Professional (C)
* Microsoft Visual C++ 2015 Professional (C)
* Intel Parallel Studio XE 2017

Written 2017-09-10 by
petter.stefansson@nmbu.no
———————————————————————————————————————————————————————————————————————————————————————————————— */

#include <mex.h>	// Needed to communicate with matlab.
#include <time.h>   // Needed for counting CPU clock cycle which is used to set seed for rand().
#include <string.h> // Needed to avoid compiler warning due to memcpy when using old compilers.

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{

	/* Before starting set the seed of the RNG to the number of clock cycles since start.        */
	srand(clock());

	/* —————————————————————————— Variable type and name declaration ——————————————————————————— */
	const bool *Population;
	bool *MutatedPopulation;
	const double *Pm;						   
	
	int individual, gene, ElitismNo;
	double RandNr;
	size_t m, n;

	/* ———————————————————————— Get pointers from the input variables —————————————————————————— */
	Population = mxGetLogicals(prhs[0]);      // Input 1 (Population)
	Pm         = mxGetPr(prhs[1]);            // Input 2 (Pm)
	ElitismNo  = (int)mxGetScalar(prhs[2]);   // Input 3 (Elitism rows)

    /* ——————————————————————— Get the dimensions of the input variables ——————————————————————— */
	m = mxGetM(prhs[0]);                      // Number of rows in Population.
	n = mxGetN(prhs[0]);                      // Number of columns in Population.

	/* ——————————————————————————————— Specify Matlab outputs —————————————————————————————————— */
	plhs[0] = mxCreateLogicalMatrix(m, n);
	MutatedPopulation = mxGetLogicals(plhs[0]);
	memcpy(MutatedPopulation, Population, sizeof(bool) * m * n );

	/* ——————————————————————————————————— Bitflip Mutation ———————————————————————————————————— */

	/* Loop all genes.																			 */
	for (gene = 0; gene < n; gene++) {
		/* Loop individuals of population, starting after elites.								 */
		for (individual = ElitismNo; individual < m; individual++) {
			
			/* Trigger mutation if Pm is greater than a random number in the range 0-1.			 */
			RandNr = (double)rand() / RAND_MAX;
			if (RandNr < (*Pm) ) {

				/* If triggered turn active gene into inactive or vice versa.				     */
				if (MutatedPopulation[individual + gene*m] == true) {
					MutatedPopulation[individual + gene*m] = false;
				}
				else {
					MutatedPopulation[individual + gene*m] = true;
				}
			}
		}
	}	
}