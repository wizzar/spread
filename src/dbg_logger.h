#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdarg.h>

class DbgLogger {

private:
	std::ofstream logFile;

	std::string getTimestamp() {

		time_t rawtime;
		struct tm* timeinfo;
		char buffer[32];

		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(buffer, 32, "%m/%d/%Y - %H:%M:%S", timeinfo);

		time_t now = time(nullptr);
		std::ostringstream oss;

		//#ifdef _WIN32
		//		tm* localTime = localtime(&now);
		//		oss << std::put_time(localTime, "%H:%M:%S");
		//#else
		//		tm tmStruct;
		//		localtime_r(&now, &tmStruct);
		//		oss << std::setfill('0') << std::setw(2) << tmStruct.tm_hour << ":"
		//			<< std::setfill('0') << std::setw(2) << tmStruct.tm_min << ":"
		//			<< std::setfill('0') << std::setw(2) << tmStruct.tm_sec;
		//#endif
		//
		//		return oss.str();

		return std::string(buffer);
	}

public:
	explicit DbgLogger(const std::string& filePath)
	{
		logFile.open(filePath, std::ios::app);
		if (logFile.is_open()) {
			Log("Log started");
		}
		//else {
		//	// Handle error
		//}
	}

	~DbgLogger()
	{
		if (logFile.is_open()) {
			Log("Log closed");
			logFile.close();
		}
	}

	bool IsOpen() {
		return logFile.is_open();
	}

	void Log(const std::string& message) {
		if (logFile.is_open()) {
			logFile << getTimestamp() << ": " << message << std::endl;
			logFile.flush();
		}
	}

	void Log(const char* fmt, ...) {
		if (logFile.is_open()) {
			
			char buffer[2048];

			va_list args;

			va_start(args, fmt);
			vsnprintf(buffer, sizeof(buffer), fmt, args);
			va_end(args);

			logFile << getTimestamp() << ": " << buffer << std::endl;
			logFile.flush();
		}
	}
};
