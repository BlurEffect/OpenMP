/* 
*  Jonathan Stewart, Kevin Meergans; 2014
*  Logger.cpp
*  Class that is used to write recorded timings to a file in a format that
*  is easy to import into Excel.
*/

#include "Logger.h"
#include <iostream>        //	For output and input
#include <fstream>			//	For files
#include <string>			//	For string manipulation
#include <time.h>			//	For time routines

Logger::Logger(string filename_, string extension_, logSeparator separator_, bool reuseFile_) : filename(filename_), extension(extension_), separator(separator_), reuseFile(reuseFile_), initialised(false)
{
}

Logger::~Logger(void)
{
}

bool Logger::init()
{
	if(reuseFile == false)
	{
		//
		//	Get the current date an time to be used in the name of the log file
		//
		time_t rawtime;
		tm timeinfo;

		const int bufferSize(20);
		char buffer[bufferSize];
		memset(buffer, 0, sizeof(char) * bufferSize);

		time(&rawtime);
		localtime_s(&timeinfo, &rawtime);
	
		strftime(buffer, bufferSize, "%H.%M.%S_%d.%m.%Y", &timeinfo);
		std::puts(buffer);

		//
		//	Is this a debug or release build?
		//
		#ifdef _DEBUG
			filename.append("_DEBUG_");
		#else
			filename.append("_RELEASE_");
		#endif
		
		//
		//	Finish the name of the timing log file
		//
		filename.append(buffer);

		//
		//	If there is no extension then return false
		//
		if(extension.empty() == true)
		{
			return(initialised);
		}

		//
		//	Check whether there is a '.' included with the extension.  If there isn't add one
		//
		if(extension.c_str()[0] != '.')
		{
			filename.append(".");
		}

		//
		//	Add the extension to the file name
		//
		filename.append(extension);
	}

	//
	//	Open the file to test whether it can be opened or not
	//
	ofstream logFile;
	logFile.open(filename, ios::app);
	
	if(logFile.is_open() == true)
	{
		//
		//	Close and return true
		//
		logFile.close();
		initialised = true;
		return(initialised);
	}

	return(initialised);
}

void Logger::write(string& logInfo, bool appendNewLine) 
{
	//
	//	If the Logger isn't initialised or there is nothing to write return
	//
	if((initialised == false) || (logInfo.empty() == true))
	{
		return;
	}

	ofstream logFile;
	logFile.open(filename, ios::app);
	logFile << logInfo;

	switch(separator)
	{
		case none:
			//
			//	Do nothing
			//
			break;

		case tab:
			//
			//	Append a tab
			//
			logFile << '\t';
			break;

		case comma:
			//
			//	Append a comma
			//
			logFile << ',';
			break;

		default:
			//
			//	Do nothing
			//
			break;
	}

	if(appendNewLine == true)
	{
		logFile << endl;
	}
}

//
//	Return the filename
//
const string Logger::getFilename() const
{
	return(filename);
}