/* 
*  Kevin Meergans, Jonathan Stewart; 2014
*  00_Sortout_Original.cpp
*  Base version of the program, no parallelization
*/

#include <fstream>			// for file output
#include <iostream>			// for console output
#include <conio.h>			// for kbhit
#include <sstream>			// used to convert float to string
#include "hr_time.h"		// for timing
#include "Logger.h"			// for saving recorded times in a file

using namespace std;

#define MAX_ROWS  2000
#define MAX_COLS  1000
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
int sortedAll[MAX_ROWS * MAX_COLS];

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
	// todo: use better sort algorithm? (Quicksort, Mergesort,...)

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
	char numString[MAX_CHARS];

	FILE * soDataRows;
	fopen_s (&soDataRows, "soDataRows.txt","w");

	for(int i = 0; i < MAX_ROWS; ++i){
		for(int j = 0; j < MAX_COLS; ++j){
			_itoa_s(data[i][j],numString,10);
			fputs(numString, soDataRows);
			fputs("\t", soDataRows);
		}
		fputs("\n", soDataRows);
	}

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

//-----------------------------------------------------------------------
// Sort the whole data array in ascending order
//-----------------------------------------------------------------------
void sortAll(void)
{
	// todo: use better sort algorithm? (Quicksort, Mergesort,...)

	// put all rows into one big array

	for(int i = 0; i < MAX_ROWS; ++i)
	{
		memcpy(&sortedAll[i * MAX_COLS], &data[i][0], MAX_COLS * sizeof(int));
	}

	// this is way faster than the bubble sort implementation below
	std::qsort(&sortedAll, MAX_COLS * MAX_ROWS, sizeof(int), compare); 

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
	// todo: file output is garbage

	char numString[MAX_CHARS];
	string dataString = "";

	for(int i = 0; i < 2000; ++i)
	{
		for(int j = 0; j < 1000; ++j){
			_itoa_s(sortedAll[i * MAX_COLS + j], numString, 10);
			dataString += numString;
			dataString += "\t";
		}
		dataString += "\n";
	}

	FILE * soAllData;
	fopen_s(&soAllData, "soAllData.txt","w");
	fputs(dataString.c_str(), soAllData);
	fclose(soAllData);
}


//-----------------------------------------------------------------------
// Calculate the 100 number moving integer average for each number in a row of data 
//-----------------------------------------------------------------------
void calcMovingAve(void)
{
	int m = 0;
	int sum = 0;

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
	string dataString;

	for(int i = 0; i < MAX_ROWS; ++i){
		for(int j = 0; j < MAX_COLS; ++j){
			_itoa_s(averages[i][j], numString, 10);
			dataString += numString;
			dataString += "\t";
		}
		dataString += "\n";
	}
	  
	FILE * soAveData;
	fopen_s(&soAveData, "soAveData.txt","w");
	fputs(dataString.c_str(), soAveData);
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

