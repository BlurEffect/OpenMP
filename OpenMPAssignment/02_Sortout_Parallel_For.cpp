/* 
*  Kevin Meergans, Jonathan Stewart; 2014
*  01_Sortout_Parallel_For.cpp
*  Parallelised version of the original program using OMP Parallel FOR
*/

#include <fstream>			// for file output
#include <iostream>			// for console output
#include <conio.h>			// for kbhit
#include <sstream>			// used to convert float to string
#include "hr_time.h"		// for timing
#include "Logger.h"			// for saving recorded times in a file
#include <omp.h>			// for parallelisation

using namespace std;

#define MAX_ROWS  2000
#define MAX_COLS  1000
#define	MAX_ELEMENTS MAX_ROWS * MAX_COLS
#define MAX_CHARS 6



// Each value describes a point in the execution of the program and is used as an argument to the testData
// function to test correctness of the program at the given stage.
enum ProgramStage
{
	SortRows, OutputSortedRows, SortAll, OutputSortedAll, CalcMovingAve, OutputAveRows
};



// global variables

// the data array, on which the sorting and output operations will be performed on
int data[MAX_ROWS][MAX_COLS];
// the array, in which the calculated moving average will be stored
int averages[MAX_ROWS][MAX_COLS];
// the array that will hold the wholly sorted data
int sortedAll[MAX_ELEMENTS];
// the array that will hold the wholly sorted data
int sortedAllOutput[MAX_ELEMENTS];

// used to store recorded timings in the specified file
Logger logger("OpenMPTimings", "", comma, true);

// stop watches for each of the six functions to be timed
CStopWatch sSortRows, sOutSortRows, sSortAll, sOutSortAll, sCalcAve, sOutAveRows;

// Create an output char array, in which the whole output will be assembled and sent to the file as a whole afterwards.
// The required size is calculated as follows:
// MAX_ROWS * MAX_COLS * MAX_CHARS  -> number of bytes required for the numbers (all of MAX_CHARS size)
// + MAX_ROWS * MAX_COLS			-> add one byte for the '\t' after every number
// + MAX_ROWS						-> add one byte for every '\n' after every row	
// + 1								-> null terminator
// Has to be global, otherwise stack overflow when local in function
char output[MAX_ROWS * MAX_COLS * MAX_CHARS + MAX_ROWS * MAX_COLS + MAX_ROWS + 1];


// forward declarations

void getData(void);
void testData(ProgramStage);
void outputTimes(void);

void sortRows(void);
void sortAll(void);
void calcMovingAve(void);

void outputSortedRows(void);
void outputSortedAll(void);
void outputAveRows(void);

//-----------------------------------------------------------------------
// Entry point of the program, calls and times sort/calculate and output functions.
//-----------------------------------------------------------------------
int main(void)
{

	// initialise the logger
	logger.init();

	getData();

	cout << "\nSorting rows...";
	sSortRows.startTimer();
	sortRows();
	sSortRows.stopTimer();
	cout << "\nSorting rows done.";

	testData(SortRows);

	cout << "\nPrinting sorted rows to 'soDataRows.txt'...";
	sOutSortRows.startTimer();
	outputSortedRows();
	sOutSortRows.stopTimer();
	cout << "\nPrinting done.";

	testData(OutputSortedRows);

	cout << "\nSorting all data...";
	sSortAll.startTimer();
	sortAll();
	sSortAll.stopTimer();
	cout << "\nSorting all data done.";

	testData(SortAll);

	cout << "\nPrinting sorted data to 'soAllData.txt'...";
	sOutSortAll.startTimer();
	outputSortedAll();
	sOutSortAll.stopTimer();
	cout << "\nPrinting done.";

	testData(OutputSortedAll);

	cout << "\nCalculating moving average...";
	sCalcAve.startTimer();
	calcMovingAve();
	sCalcAve.stopTimer();
	cout << "\nCalculating averages done.";

	testData(CalcMovingAve);

	cout << "\nPrinting averages to 'soAveData.txt'...";
	sOutAveRows.startTimer();
	outputAveRows();
	sOutAveRows.stopTimer();
	cout << "\nPrinting done.";

	testData(OutputAveRows);

	// record the measured times
	outputTimes();

	cout << "\n\nAll Done.";
	system("pause");

	while (! _kbhit());  //to hold console	
}

//-----------------------------------------------------------------------
// Sort each row of the data array in ascending order
//-----------------------------------------------------------------------
void sortRows()
{
	// static schedule seems to be slower
	#pragma omp parallel for schedule(dynamic, 100) shared(data)
	for(int i = 0; i < MAX_ROWS; ++i){
			
		//bubble sort row i (code of original bubble sort function inlined below)
		for(int n = MAX_COLS -1; n >= 0; --n){   
            for(int j = 0; j < n; ++j){   
                if(data[i][j] > data[i][j+1]){   
                    int temp = data[i][j];   
                    data[i][j] = data[i][j+1];   
                    data[i][j+1] = temp;   
                }   
            }  
		}
	}
}


//-----------------------------------------------------------------------
// Output the sorted rows to "soDataRows.txt"
//-----------------------------------------------------------------------
void outputSortedRows()
{
	// fill the output string with empty characters (zero memory would fill it with null terminators which would be a problem upon printing
	// the string to a file (see below)
	memset ( output, ' ', MAX_ELEMENTS * MAX_CHARS + MAX_ELEMENTS + MAX_ROWS);

	char numString[MAX_CHARS];

	#pragma omp parallel for schedule(dynamic, 100) firstprivate(numString) shared(data)
	for(int i = 0; i < MAX_ROWS; ++i)
	{
		for(int j = 0; j < MAX_COLS; ++j)
		{
			_itoa_s(data[i][j],numString,10);
			
			// Originally I used memcpy to get the contents of numString into output but it seems the itoa operation appends
			// a '\0' termination character to the number when putting it into the numString above. So, simply outputting the
			// output array lead to only the first number being printed as the output process would be stopped on first occurrence of 
			// a terminator.
			int k = 0;
			char temp = numString[k];
			while(temp != '\0')
			{
				output[i * (MAX_COLS * MAX_CHARS + MAX_COLS + 1) + j * (MAX_CHARS + 1) + k] = temp;
				temp = numString[++k];
			}

			// append a tab after each number
			output[i * (MAX_COLS * MAX_CHARS + MAX_COLS + 1) + j * (MAX_CHARS + 1) + MAX_CHARS] = '\t';
		}
		// append a new line after each row
		output[i * (MAX_COLS * MAX_CHARS + MAX_COLS + 1) + MAX_COLS * (MAX_CHARS + 1)] = '\n';
	}

	// append the null terminator to the end of the string
	output[MAX_ELEMENTS * MAX_CHARS + MAX_ELEMENTS + MAX_ROWS] = '\0';
	
	FILE * soDataRows;
	fopen_s (&soDataRows, "soDataRows.txt","w");
	
	// output the whole data at once
	fputs(output, soDataRows);

	fclose (soDataRows);
}

//-----------------------------------------------------------------------
// Helper function used to compare two elements of the data array when
// using the qsort function in sortAll()
//-----------------------------------------------------------------------
int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

// Merge sorts two passed in arrays of the specified sizes and stores the sorted elements in a third array
// (currently doesn't account for the third array being too small to hold all elements)
// Partially taken/adapted from here: http://stackoverflow.com/questions/18141065/merge-sort-implementation
void merge(int sortedArray1[], int arraySize1, int sortedArray2[], int arraySize2, int outputArray[])
{
	// positions to be compared in the two arrays and position in the output array
	// (used as indices)
	int pos1 = 0;
	int pos2 = 0;
	int pos3 = 0;

	// check which of both sorted arrays has the smaller element at the currently pointed to
	// positions and store that element in the output array, while incrementing the indices afterwards
	while (pos1 < arraySize1 && pos2 < arraySize2) {
        if (sortedArray1[pos1] < sortedArray2[pos2]) {
            outputArray[pos3++] = sortedArray1[pos1++];
        } 
        else {
            outputArray[pos3++] = sortedArray2[pos2++];
        }
    }

	// array 2 exhausted, copy the rest of array 1 to the output array
    while (pos1 < arraySize1) {   
        outputArray[pos3++] = sortedArray1[pos1++];
    }
	// array 1 exhausted, copy the rest of array 2 to the output array
    while (pos2 < arraySize2) { 
        outputArray[pos3++] = sortedArray2[pos2++];
    }
	
}

//-----------------------------------------------------------------------
// Sort the whole data array in ascending order
//-----------------------------------------------------------------------
void sortAll(void)
{

	// put all sorted rows into one big array	
	// parallelising this part probably hardly has any effect
	#pragma omp parallel for schedule(dynamic, 100) shared(sortedAll, data)
	for(int i = 0; i < MAX_ROWS; ++i)
	{
		memcpy(&sortedAll[i * MAX_COLS], &data[i][0], MAX_COLS * sizeof(int));
	}

	// split the array into sub arrays and sort them in parallel using quick sort
	
	// Determines in how many sorted sub arrays the sortedAll array will be split 
	// (should be chosen to be a value that produces an even number of sub arrays, all with the same size and be a power of 2)
	// possible values: 1,2,4,8,16,32,64,128
	int numOfChunks = 16; // 16 seems to yield the best results when timed over a couple of iterations of the program.
						 // Probably because there should usually be 8 threads running the parallel sections and with 16 chunks
						 // the merge sort process can executed the fastest way possible (8 threads, each merging two chunks)
	// The amount of elements in each sorted sub array
	int chunkSize = MAX_ELEMENTS / numOfChunks;

	// sort the sub arrays, use the number of chunks to offset the arrays
	#pragma omp parallel for schedule(dynamic, 1) shared(sortedAll) firstprivate(numOfChunks, chunkSize)
	for(int i = 0; i < numOfChunks; ++i)
	{
		std::qsort(&sortedAll[chunkSize * i], chunkSize, sizeof(int), compare); 
	}

	// merge the sorted sub arrays into one array holding the whole sorted data
	
	// each iteration the number of sub arrays will be halved as two neighbouring arrays get merged in each iteration;
	// continue merging until only one array is left;
	// i represents the current number of sub arrays
	for(int i = numOfChunks; i != 1; i = i >> 1)
	{
		// parallelisation: each thread takes two sub arrays, merges them and puts them into the sortAllOutput array
		#pragma omp parallel for schedule(dynamic, 1) shared(sortedAll, sortedAllOutput) firstprivate(i)
		for(int j = 0; j < i; j+=2)
		{
			// merge two arrays using the merge function
			merge(&sortedAll[j * (MAX_ELEMENTS) / i], (MAX_ELEMENTS) / i, &sortedAll[(j+1) * (MAX_ELEMENTS) / i], (MAX_ELEMENTS) / i, &sortedAllOutput[((MAX_ELEMENTS) / (i)) * j]);
		}

		// copy sortedAllOutput to sortedAll so the next merge iteration can work on the correct data;
		memcpy(sortedAll, sortedAllOutput, MAX_ELEMENTS * sizeof(int));
	}
}

//-----------------------------------------------------------------------
// Output the sorted data to "soAllData.txt"
//-----------------------------------------------------------------------
void outputSortedAll(void)
{
	// fill the output string with empty characters (zero memory would fill it with null terminators which would be a problem upon printing
	// the string to a file (see below)
	memset ( output, ' ', MAX_ELEMENTS * MAX_CHARS + MAX_ELEMENTS + MAX_ROWS);

	char numString[MAX_CHARS];

	#pragma omp parallel for schedule(dynamic, 100) firstprivate(numString) shared(data)
	for(int i = 0; i < MAX_ROWS; ++i)
	{
		for(int j = 0; j < MAX_COLS; ++j)
		{
			_itoa_s(sortedAll[i * MAX_COLS + j],numString,10);
			
			// Originally I used memcpy to get the contents of numString into output but it seems the itoa operation appends
			// a '\0' termination character to the number when putting it into the numString above. So, simply outputting the
			// output array lead to only the first number being printed as the output process would be stopped on first occurrence of 
			// a terminator.
			int k = 0;
			char temp = numString[k];
			while(temp != '\0')
			{
				output[i * (MAX_COLS * MAX_CHARS + MAX_COLS + 1) + j * (MAX_CHARS + 1) + k] = temp;
				temp = numString[++k];
			}

			// append a tab after each number
			output[i * (MAX_COLS * MAX_CHARS + MAX_COLS + 1) + j * (MAX_CHARS + 1) + MAX_CHARS] = '\t';
		}
		// append a new line after each row
		output[i * (MAX_COLS * MAX_CHARS + MAX_COLS + 1) + MAX_COLS * (MAX_CHARS + 1)] = '\n';
	}

	// append the null terminator to the end of the string
	output[MAX_ELEMENTS * MAX_CHARS + MAX_ELEMENTS + MAX_ROWS] = '\0';

	FILE * soAllData;
	fopen_s(&soAllData, "soAllData.txt","w");

	// output the whole data at once
	fputs(output, soAllData);

	fclose(soAllData);
}


//-----------------------------------------------------------------------
// Calculate the 100 number moving integer average for each number in a row of data 
//-----------------------------------------------------------------------
void calcMovingAve(void)
{
	int m = 0;
	int sum = 0;

	#pragma omp parallel for schedule(dynamic, 100) shared(averages) firstprivate(m, sum)
	for(int i = 0; i < MAX_ROWS; ++i){
		for(int j = 0; j < MAX_COLS; ++j){

			// determine how many numbers will be used to form the average
			if(j <= MAX_COLS - 100)
				m = 100;
			else
				m = MAX_COLS - j;

			sum = 0;
			for(int k = j; k < j + m; ++k)
			{
				sum += data[i][k];
			}

			// calculate and store the average
			averages[i][j] = sum / m;
		}
	}
}

//-----------------------------------------------------------------------
// Output the calculated averages to "soAveData.txt"
//-----------------------------------------------------------------------
void outputAveRows(void)
{
	char numString[MAX_CHARS];

	#pragma omp parallel for schedule(dynamic, 100) firstprivate(numString) shared(data)
	for(int i = 0; i < MAX_ROWS; ++i)
	{
		for(int j = 0; j < MAX_COLS; ++j)
		{
			_itoa_s(averages[i][j],numString,10);
			
			// Originally I used memcpy to get the contents of numString into output but it seems the itoa operation appends
			// a '\0' termination character to the number when putting it into the numString above. So, simply outputting the
			// output array lead to only the first number being printed as the output process would be stopped on first occurrence of 
			// a terminator.
			int k = 0;
			char temp = numString[k];
			while(temp != '\0')
			{
				output[i * (MAX_COLS * MAX_CHARS + MAX_COLS + 1) + j * (MAX_CHARS + 1) + k] = temp;
				temp = numString[++k];
			}

			// append a tab after each number
			output[i * (MAX_COLS * MAX_CHARS + MAX_COLS + 1) + j * (MAX_CHARS + 1) + MAX_CHARS] = '\t';
		}
		// append a new line after each row
		output[i * (MAX_COLS * MAX_CHARS + MAX_COLS + 1) + MAX_COLS * (MAX_CHARS + 1)] = '\n';
	}

	// append the null terminator to the end of the string
	output[MAX_ELEMENTS * MAX_CHARS + MAX_ELEMENTS + MAX_ROWS] = '\0';
	  
	FILE * soAveData;
	fopen_s(&soAveData, "soAveData.txt","w");

	// output the whole data at once
	fputs(output, soAveData);

	fclose(soAveData);
}


//-----------------------------------------------------------------------
// Fill the data array with random numbers
//-----------------------------------------------------------------------
void getData()
{
	srand(123); //arbitrary random number seed
	for(int i=0; i<MAX_ROWS; i++)
		for(int j=0; j<MAX_COLS; j++)
			data[i][j] = rand(); //RAND_MAX = 32767
}

//-----------------------------------------------------------------------
// Print the recorded times for the six functions to a file
//-----------------------------------------------------------------------
void outputTimes(void)
{
	// output to console
	cout << "\n\nTimes achieved: ";
	cout << "\nSortRows:         " << sSortRows.getElapsedTime();
	cout << "\nOutputSortedRows: " << sOutSortRows.getElapsedTime();
	cout << "\nSortAll:          " << sSortAll.getElapsedTime();
	cout << "\nOutputSortedAll:  " << sOutSortAll.getElapsedTime();
	cout << "\nCalcMovingAve:    " << sCalcAve.getElapsedTime();
	cout << "\nOutputAveRows:    " << sOutAveRows.getElapsedTime();
	cout << "\nTotalTime:        " << sSortRows.getElapsedTime() + sOutSortRows.getElapsedTime() + 
									  sSortAll.getElapsedTime() + sOutSortAll.getElapsedTime() + 
		                              sCalcAve.getElapsedTime() + sOutAveRows.getElapsedTime();
		
	// output to file using the Logger class
	
	ostringstream oss;

	oss << sSortRows.getElapsedTime();
	logger.write(oss.str(), false);
	oss.str("");

	oss << sOutSortRows.getElapsedTime();
	logger.write(oss.str(), false);
	oss.str("");

	oss << sSortAll.getElapsedTime();
	logger.write(oss.str(), false);
	oss.str("");

	oss << sOutSortAll.getElapsedTime();
	logger.write(oss.str(), false);
	oss.str("");

	oss << sCalcAve.getElapsedTime();
	logger.write(oss.str(), false);
	oss.str("");

	oss << sOutAveRows.getElapsedTime();
	logger.write(oss.str(), false);
	oss.str("");

	oss << (sSortRows.getElapsedTime() + sOutSortRows.getElapsedTime() + 
			sSortAll.getElapsedTime() + sOutSortAll.getElapsedTime() + 
		    sCalcAve.getElapsedTime() + sOutAveRows.getElapsedTime());
	logger.write(oss.str(), true);
	oss.str("");

}

//-----------------------------------------------------------------------
// Print a selection of data values for comparison with the expected values (can be used to detect errors).
// The numbers in brackets display the expected values for a random number generator seed of 123.
//-----------------------------------------------------------------------
void testData(ProgramStage stage)
{
	cout << "\nTest data: ";

	switch(stage)
	{
	case SortRows:
		cout << "\ndata[0][0]                   = " << data[0][0] << "    (87)";					//=0 for srand(87)			
		cout << "\ndata[MAX_ROWS/2][MAX_COLS/2] = " << data[MAX_ROWS/2][MAX_COLS/2] << " (16440)";	//=0 for srand(16440)
		cout << "\ndata[MAX_ROWS-1][MAX_COLS-1] = " << data[MAX_ROWS-1][MAX_COLS-1] << " (32760)";	//=0 for srand(32760)
		break;
	case OutputSortedRows:
		cout << "\ndata[0][0]                   = " << data[0][0] << "    (87)";					//=0 for srand(87)	
		cout << "\ndata[MAX_ROWS/2][MAX_COLS/2] = " << data[MAX_ROWS/2][MAX_COLS/2] << " (16440)";	//=0 for srand(16440)	
		cout << "\ndata[MAX_ROWS-1][MAX_COLS-1] = " << data[MAX_ROWS-1][MAX_COLS-1] << " (32760)";	//=0 for srand(32760)	
		break;
	case SortAll:
		cout << "\nsortedAll[0]                           = " << sortedAll[0] << "      (0)";							//=0 for srand(123)
		cout << "\nsortedAll[(MAX_COLS * MAX_ROWS)/2 - 1] = " << sortedAll[(MAX_COLS * MAX_ROWS)/2 - 1] << " (16411)";	//=16411 for srand(123)
		cout << "\nsortedAll[(MAX_COLS * MAX_ROWS)-1]     = " << sortedAll[(MAX_COLS * MAX_ROWS)-1] << " (32767)";		//=32767 for srand(123)
		break;
	case OutputSortedAll:
		cout << "\nsortedAll[0]                           = " << sortedAll[0] << "      (0)";							//=0 for srand(123)
		cout << "\nsortedAll[(MAX_COLS * MAX_ROWS)/2 - 1] = " << sortedAll[(MAX_COLS * MAX_ROWS)/2 - 1] << " (16411)";	//=16411 for srand(123)
		cout << "\nsortedAll[(MAX_COLS * MAX_ROWS)-1]     = " << sortedAll[(MAX_COLS * MAX_ROWS)-1] << " (32767)";		//=32767 for srand(123)
		break;
	case CalcMovingAve:
		cout << "\naverages[0][0]                   = " << averages[0][0] << "  (1991)";					//=1991 for srand(123)
		cout << "\naverages[MAX_ROWS/2][MAX_COLS/2] = " << averages[MAX_ROWS/2][MAX_COLS/2] << " (18215)";	//=18215 for srand(123)
		cout << "\naverages[MAX_ROWS-1][MAX_COLS-1] = " << averages[MAX_ROWS-1][MAX_COLS-1] << " (32760)";	//=32760 for srand(123)
		break;
	case OutputAveRows:
		cout << "\naverages[0][0]                   = " << averages[0][0] << "  (1991)";					//=1991 for srand(123)
		cout << "\naverages[MAX_ROWS/2][MAX_COLS/2] = " << averages[MAX_ROWS/2][MAX_COLS/2] << " (18215)";	//=18215 for srand(123)
		cout << "\naverages[MAX_ROWS-1][MAX_COLS-1] = " << averages[MAX_ROWS-1][MAX_COLS-1] << " (32760)";	//=32760 for srand(123)
		break;
	}

	cout << "\n";
}
