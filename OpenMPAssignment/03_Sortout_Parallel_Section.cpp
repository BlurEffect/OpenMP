/* 
*  Jonathan Stewart, Kevin Meergans; 2014
*  03_Sortout_Parallel_Section.cpp
*  Parallelised version of the original program using OMP Parallel Sections
*/

#include <fstream>			// for file output
#include <iostream>			// for console output
#include <conio.h>			// for kbhit
#include <sstream>			// used to convert float to string
#include "hr_time.h"		// for timing
#include "Logger.h"			// for saving recorded times in a file
#include <omp.h>			//	for OpenMP
#include <string>			//	For the output
#include <sstream>			//	For the output

using namespace std;

#define MAX_ROWS  2000
#define MAX_COLS  1000
#define	MAX_ELEMENTS MAX_ROWS * MAX_COLS
#define MAX_CHARS 6

// Create an output char array, in which the whole output will be assembled and sent to the file as a whole afterwards.
// The required size is calculated as follows:
// MAX_ROWS * MAX_COLS * MAX_CHARS  -> number of bytes required for the numbers (all of MAX_CHARS size)
// + MAX_ROWS * MAX_COLS			-> add one byte for the '\t' after every number
// + MAX_ROWS						-> add one byte for every '\n' after every row	
// + 1								-> null terminator
// Has to be global, otherwise stack overflow when local in function
char output[MAX_ROWS * MAX_COLS * MAX_CHARS + MAX_ROWS * MAX_COLS + MAX_ROWS + 1];

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
int sortedAll[MAX_ROWS * MAX_COLS];

int firstSorted[(MAX_ROWS/8) * MAX_COLS];
int secondSorted[(MAX_ROWS/8) * MAX_COLS];
int thirdSorted[(MAX_ROWS/8) * MAX_COLS];
int fourthSorted[(MAX_ROWS/8) * MAX_COLS];
int fifthSorted[(MAX_ROWS/8) * MAX_COLS];
int sixthSorted[(MAX_ROWS/8) * MAX_COLS];
int seventhSorted[(MAX_ROWS/8) * MAX_COLS];
int eighthSorted[(MAX_ROWS/8) * MAX_COLS];

int temp1[(MAX_ROWS/4) * MAX_COLS];
int temp2[(MAX_ROWS/4) * MAX_COLS];
int temp3[(MAX_ROWS/4) * MAX_COLS];
int temp4[(MAX_ROWS/4) * MAX_COLS];

int temp5[(MAX_ROWS/2) * MAX_COLS];
int temp6[(MAX_ROWS/2) * MAX_COLS];

const int first = (MAX_ROWS/8);
const int second = first * 2;
const int third = first * 3;
const int fourth = first * 4;
const int fifth = first * 5;
const int sixth = first * 6;
const int seventh = first * 7;



// used to store recorded timings in the specified file
Logger logger("OpenMPTimings", "", comma, true);

// stop watches for each of the six functions to be timed
CStopWatch sSortRows, sOutSortRows, sSortAll, sOutSortAll, sCalcAve, sOutAveRows;

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
	cout << "\nSortRows time:         " << sSortRows.getElapsedTime();

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
	cout << "\nSortAll time:         " << sSortAll.getElapsedTime();

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
	//printf("sortRows, maximum number of threads: %d\n", omp_get_max_threads());
	//printf("sortRows, number of threads: %d\n", omp_get_num_threads());

	// todo: use better sort algorithm? (Quicksort, Mergesort,...)
	#pragma omp parallel sections num_threads(8) firstprivate(first, second, third, fourth, fifth, sixth, seventh) shared(data) 
	{
		//printf("sortRows, number of threads: %d", omp_get_num_threads());

		#pragma omp section
		for(int i = 0; i < first; ++i){
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

		#pragma omp section
		for(int i = first; i < second; ++i){
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

		#pragma omp section
		for(int i = second; i < third; ++i){
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

		#pragma omp section
		for(int i = third; i < fourth; ++i){
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

		#pragma omp section
		for(int i = fourth; i < fifth; ++i){
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

		#pragma omp section
		for(int i = fifth; i < sixth; ++i){
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

		#pragma omp section
		for(int i = sixth; i < seventh; ++i){
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

		#pragma omp section
		for(int i = seventh; i < MAX_ROWS; ++i){
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

	#pragma omp parallel sections num_threads(8) firstprivate(numString, first, second, third, fourth, fifth, sixth, seventh) shared(output, data) 
	{
		#pragma omp section
		{
			for(int i = 0; i < first; ++i)
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
		}

		#pragma omp section
		{
			for(int i = first; i < second; ++i)
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
		}

		#pragma omp section
		{
			for(int i = second; i < third; ++i)
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
		}

		#pragma omp section
		{
			for(int i = third; i < fourth; ++i)
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
		}

		#pragma omp section
		{
			for(int i = fourth; i < fifth; ++i)
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
		}

		#pragma omp section
		{
			for(int i = fifth; i < sixth; ++i)
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
		}

		#pragma omp section
		{
			for(int i = sixth; i < seventh; ++i)
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
		}

		#pragma omp section
		{
			for(int i = seventh; i < MAX_ROWS; ++i)
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
		}
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

	#pragma omp parallel sections num_threads(8) firstprivate(first, second, third, fourth, fifth, sixth, seventh) shared(data) 
	{
		//printf("sortAll, number of threads: %d\n", omp_get_num_threads());
		#pragma omp section
		{
			//printf("sortAll, first, thread number: %d\n", omp_get_thread_num());
			for(int i = 0; i < first; ++i)
			{
				memcpy(&firstSorted[i * MAX_COLS], &data[i][0], MAX_COLS * sizeof(int));
			}

			std::qsort(&firstSorted, MAX_COLS * first, sizeof(int), compare); 

			//printf("sortAll, first sorted\n");
		}

		#pragma omp section
		{
			//printf("sortAll, second, thread number: %d\n", omp_get_thread_num());
			for(int i = 0; i < first; ++i)
			{
				memcpy(&secondSorted[i * MAX_COLS], &data[i + first][0], MAX_COLS * sizeof(int));
			}

			std::qsort(&secondSorted, MAX_COLS * first, sizeof(int), compare); 

			//printf("sortAll, second sorted\n");
		}

		#pragma omp section
		{
			//printf("sortAll, third, thread number: %d\n", omp_get_thread_num());
			for(int i = 0; i < first; ++i)
			{
				memcpy(&thirdSorted[i * MAX_COLS], &data[i + second][0], MAX_COLS * sizeof(int));
			}

			std::qsort(&thirdSorted, MAX_COLS * first, sizeof(int), compare); 

			//printf("sortAll, third sorted\n");
		}

		#pragma omp section
		{
			//printf("sortAll, fourth, thread number: %d\n", omp_get_thread_num());
			for(int i = 0; i < first; ++i)
			{
				memcpy(&fourthSorted[i * MAX_COLS], &data[i + third][0], MAX_COLS * sizeof(int));
			}

			std::qsort(&fourthSorted, MAX_COLS * first, sizeof(int), compare); 

			//printf("sortAll, fourth sorted\n");
		}

		#pragma omp section
		{
			//printf("sortAll, fourth, thread number: %d\n", omp_get_thread_num());
			for(int i = 0; i < first; ++i)
			{
				memcpy(&fifthSorted[i * MAX_COLS], &data[i + fourth][0], MAX_COLS * sizeof(int));
			}

			std::qsort(&fifthSorted, MAX_COLS * first, sizeof(int), compare); 

			//printf("sortAll, fourth sorted\n");
		}

		#pragma omp section
		{
			//printf("sortAll, fourth, thread number: %d\n", omp_get_thread_num());
			for(int i = 0; i < first; ++i)
			{
				memcpy(&sixthSorted[i * MAX_COLS], &data[i + fifth][0], MAX_COLS * sizeof(int));
			}

			std::qsort(&sixthSorted, MAX_COLS * first, sizeof(int), compare); 

			//printf("sortAll, fourth sorted\n");
		}

		#pragma omp section
		{
			//printf("sortAll, fourth, thread number: %d\n", omp_get_thread_num());
			for(int i = 0; i < first; ++i)
			{
				memcpy(&seventhSorted[i * MAX_COLS], &data[i + sixth][0], MAX_COLS * sizeof(int));
			}

			std::qsort(&seventhSorted, MAX_COLS * first, sizeof(int), compare); 

			//printf("sortAll, fourth sorted\n");
		}

		#pragma omp section
		{
			//printf("sortAll, fourth, thread number: %d\n", omp_get_thread_num());
			for(int i = 0; i < first; ++i)
			{
				memcpy(&eighthSorted[i * MAX_COLS], &data[i + seventh][0], MAX_COLS * sizeof(int));
			}

			std::qsort(&eighthSorted, MAX_COLS * first, sizeof(int), compare); 

			//printf("sortAll, fourth sorted\n");
		}
	}

	int maxLen = first * MAX_COLS;
	int maxDiv8 = MAX_ROWS/8;
	int maxDiv8MaxCols = maxDiv8 * MAX_COLS;

	//
	//	Merge the sorted arrays
	//
	#pragma omp parallel sections num_threads(4)
	{
		#pragma omp section
		{
			merge(firstSorted, maxDiv8MaxCols, secondSorted, maxDiv8MaxCols, temp1);
			//for(int i(0), j(0); i < maxLen; ++i, j+=2)
			//{
			//	if(firstSorted[i] < secondSorted[i])
			//	{
			//		temp1[j] = firstSorted[i];
			//		temp1[j+1] = secondSorted[i];
			//	}
			//	else
			//	{
			//		temp1[j] = secondSorted[i];
			//		temp1[j+1] = firstSorted[i];
			//	}
			//}

			////
			////	Sort the numbers
			////
			//std::qsort(&temp1, maxLen, sizeof(int), compare); 
		}

		#pragma omp section
		{
			merge(thirdSorted, maxDiv8MaxCols, fourthSorted, maxDiv8MaxCols, temp2);
			//for(int i(0), j(0); i < maxLen; ++i, j+=2)
			//{
			//	if(thirdSorted[i] < fourthSorted[i])
			//	{
			//		temp2[j] = thirdSorted[i];
			//		temp2[j+1] = fourthSorted[i];
			//	}
			//	else
			//	{
			//		temp2[j] = fourthSorted[i];
			//		temp2[j+1] = thirdSorted[i];
			//	}
			//}

			////
			////	Sort the numbers
			////
			//std::qsort(&temp2, maxLen, sizeof(int), compare); 
		}

		#pragma omp section
		{
			merge(fifthSorted, maxDiv8MaxCols, sixthSorted, maxDiv8MaxCols, temp3);
		}

		#pragma omp section
		{
			merge(seventhSorted, maxDiv8MaxCols, eighthSorted, maxDiv8MaxCols, temp4);
		}
	}

	int maxDiv4 = MAX_ROWS/4;
	int maxDiv4MaxCols = maxDiv4 * MAX_COLS;

	#pragma omp parallel sections num_threads(2)
	{
		#pragma omp section
		{
			merge(temp1, maxDiv4MaxCols, temp2, maxDiv4MaxCols, temp5);
		}

		#pragma omp section
		{
			merge(temp3, maxDiv4MaxCols, temp4, maxDiv4MaxCols, temp6);
		}
	}

	//
	//  Now merge temp1 and temp2
	//
	merge(temp5, (MAX_ROWS/2) * MAX_COLS, temp6, (MAX_ROWS/2) * MAX_COLS, sortedAll);
	//maxLen = second * MAX_COLS;
	//for(int i(0), j(0); i < maxLen; ++i, j+=2)
	//{
	//	if(temp1[i] < temp2[i])
	//	{
	//		sortedAll[j] = thirdSorted[i];
	//		sortedAll[j+1] = fourthSorted[i];
	//	}
	//	else
	//	{
	//		sortedAll[j] = fourthSorted[i];
	//		sortedAll[j+1] = thirdSorted[i];
	//	}			
	//}

	////printf("sortAll finished");

	//// this is way faster than the bubble sort implementation below
	//std::qsort(&sortedAll, MAX_COLS * MAX_ROWS, sizeof(int), compare); 

	/*
	// sort the array with bubble sort
	for(int n = MAX_ROWS * MAX_COLS - 1; n >= 0; --n){   
        for(int j = 0; j < n; ++j){   
            if(sortedAll[j] > sortedAll[j+1]){   
                int temp = sortedAll[j];   
                sortedAll[j] = sortedAll[j+1];   
                sortedAll[j+1] = temp;   
            }   
        }  
	}
	*/
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

	#pragma omp parallel sections num_threads(8) firstprivate(numString, first, second, third, fourth, fifth, sixth, seventh) shared(output, data) 
	{
		#pragma omp section
		{
			for(int i = 0; i < first; ++i)
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
		}

		#pragma omp section
		{
			for(int i = first; i < second; ++i)
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
		}

		#pragma omp section
		{
			for(int i = second; i < third; ++i)
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
		}

		#pragma omp section
		{
			for(int i = third; i < fourth; ++i)
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
		}

		#pragma omp section
		{
			for(int i = fourth; i < fifth; ++i)
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
		}

		#pragma omp section
		{
			for(int i = fifth; i < sixth; ++i)
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
		}

		#pragma omp section
		{
			for(int i = sixth; i < seventh; ++i)
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
		}

		#pragma omp section
		{
			for(int i = seventh; i < MAX_ROWS; ++i)
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
		}
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
	//
	//	Merge the sorted arrays
	//
	#pragma omp parallel sections num_threads(8) firstprivate(first, second, third, fourth, fifth, sixth, seventh) shared(data, averages) 
	{
		#pragma omp section
		{
			int m = 0;
			int sum = 0;

			for(int i = 0; i < first; ++i){
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

		#pragma omp section
		{
			int m = 0;
			int sum = 0;

			for(int i = first; i < second; ++i){
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

		#pragma omp section
		{
			int m = 0;
			int sum = 0;

			for(int i = second; i < third; ++i){
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

		#pragma omp section
		{
			int m = 0;
			int sum = 0;

			for(int i = third; i < fourth; ++i){
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

		#pragma omp section
		{
			int m = 0;
			int sum = 0;

			for(int i = fourth; i < fifth; ++i){
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

		#pragma omp section
		{
			int m = 0;
			int sum = 0;

			for(int i = fifth; i < sixth; ++i){
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

		#pragma omp section
		{
			int m = 0;
			int sum = 0;

			for(int i = sixth; i < seventh; ++i){
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

		#pragma omp section
		{
			int m = 0;
			int sum = 0;

			for(int i = seventh; i < MAX_ROWS; ++i){
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
	}
}

//-----------------------------------------------------------------------
// Output the calculated averages to "soAveData.txt"
//-----------------------------------------------------------------------
void outputAveRows(void)
{
	// fill the output string with empty characters (zero memory would fill it with null terminators which would be a problem upon printing
	// the string to a file (see below)
	memset ( output, ' ', MAX_ELEMENTS * MAX_CHARS + MAX_ELEMENTS + MAX_ROWS);

	char numString[MAX_CHARS];

	#pragma omp parallel sections num_threads(8) firstprivate(numString, first, second, third, fourth, fifth, sixth, seventh) shared(output, data) 
	{
		#pragma omp section
		{
			for(int i = 0; i < first; ++i)
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
		}

		#pragma omp section
		{
			for(int i = first; i < second; ++i)
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
		}

		#pragma omp section
		{
			for(int i = second; i < third; ++i)
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
		}

		#pragma omp section
		{
			for(int i = third; i < fourth; ++i)
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
		}

		#pragma omp section
		{
			for(int i = fifth; i < sixth; ++i)
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
		}

		#pragma omp section
		{
			for(int i = sixth; i < seventh; ++i)
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
		}

		#pragma omp section
		{
			for(int i = seventh; i < MAX_ROWS; ++i)
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
		}
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
		cout << "\ndata[0][0]                   = " << data[0][0] << "    (87)";						
		cout << "\ndata[MAX_ROWS/2][MAX_COLS/2] = " << data[MAX_ROWS/2][MAX_COLS/2] << " (16440)";
		cout << "\ndata[MAX_ROWS-1][MAX_COLS-1] = " << data[MAX_ROWS-1][MAX_COLS-1] << " (32760)";	
		break;
	case OutputSortedRows:
		cout << "\ndata[0][0]                   = " << data[0][0] << "    (87)";
		cout << "\ndata[MAX_ROWS/2][MAX_COLS/2] = " << data[MAX_ROWS/2][MAX_COLS/2] << " (16440)";
		cout << "\ndata[MAX_ROWS-1][MAX_COLS-1] = " << data[MAX_ROWS-1][MAX_COLS-1] << " (32760)";
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

