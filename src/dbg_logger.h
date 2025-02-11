#pragma once

#include <string>
#include <iosfwd>

#ifdef DO_DEBUG

using namespace std;

class DbgLogger {

public:
	DbgLogger(string filePath)
	{

	}

	~DbgLogger()
	{
		/*if (logFile.is_open()) {
			Log("Closing log");
			logFile.close();
		}*/
	}

	void Log(string message) {
		
	}

private:
	//ofstream logFile;

};





#endif // DO_DEBUG
