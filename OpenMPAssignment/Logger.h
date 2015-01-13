/* 
*  Jonathan Stewart, Kevin Meergans; 2014
*  Logger.h
*  Class that is used to write recorded timings to a file in a format that
*  is easy to import into Excel.
*/

#pragma once

#include <string>

using namespace std;

enum logSeparator
{
	none,
	tab,
	comma
};

class Logger
{
	private:
		string filename;
		string extension;
		logSeparator separator;
		bool reuseFile;
		bool initialised;
		
	public:
		//
		//	
		//
		Logger(string filename_, string extension_, logSeparator separator_, bool reuseFile_);
		~Logger(void);

		//
		//	Initialise the logger
		//
		bool init();

		//
		//	Write to the log
		//
		void write(string& logInfo, bool appendNewLine);

		//
		//	Get the file name
		//
		const string getFilename() const;
};

