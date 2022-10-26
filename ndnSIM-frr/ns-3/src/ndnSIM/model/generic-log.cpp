#include "generic-log.hpp"

#include <string>

ofstream* GenericLog::logEEDT_N = NULL;
ofstream* GenericLog::logEEDT_R = NULL;
ofstream* GenericLog::logEEFR = NULL;
ofstream* GenericLog::logOOR = NULL;
string* GenericLog::logDirPath = NULL;

unordered_map<uint32_t, ns3::Time> GenericLog::TimeSendingData;
unordered_map<uint32_t, ns3::Time> GenericLog::TimeReceivedData;
list<uint32_t> GenericLog::dataID_sent;
list<uint32_t> GenericLog::dataID_recv;

using namespace ns3;

void GenericLog::setLogDirPath(const char* logDirPath) {
	GenericLog::logDirPath = new string(logDirPath);
}

void GenericLog::openLogs() {
	stringstream ss;

	ss.str("");
	ss << *logDirPath << "log-eedt-n";
	logEEDT_N = new ofstream;
	logEEDT_N->open(ss.str(), ios::trunc);

	ss.str("");
	ss << *logDirPath << "log-eedt-r";
	logEEDT_R = new ofstream;
	logEEDT_R->open(ss.str(), ios::trunc);

	ss.str("");
	ss << *logDirPath << "log-eefr";
	logEEFR = new ofstream;
	logEEFR->open(ss.str(), ios::trunc);

	ss.str("");
	ss << *logDirPath << "log-oor";
	logOOR = new ofstream;
	logOOR->open(ss.str(), ios::trunc);
}

ofstream* GenericLog::getLogEEDT_N() {
	return logEEDT_N;
}

ofstream* GenericLog::getLogEEDT_R() {
	return logEEDT_R;
}

ofstream* GenericLog::getLogEEFR() {
	return logEEFR;
}

ofstream* GenericLog::getLogOOR() {
	return logOOR;
}

void GenericLog::closeLogs() {
	logEEDT_N->close();
	logEEDT_R->close();
	logEEFR->close();
	logOOR->close();
}
