#ifndef SRC_NDNSIM_MODEL_GENERIC_LOG_HPP_
#define SRC_NDNSIM_MODEL_GENERIC_LOG_HPP_

#include "NFD/daemon/face/face-common.hpp"

#include "ns3/nstime.h"

#include <string>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <fstream>

using namespace std;
using namespace nfd;

class GenericLog {
public:
	static void setLogDirPath(const char* logDirPath);
	static void openLogs();
	static ofstream* getLogEEDT_N();
	static ofstream* getLogEEDT_R();
	static ofstream* getLogEEFR();
	static ofstream* getLogOOR();
	static void closeLogs();

	static unordered_map<uint32_t, ns3::Time> TimeSendingData;
	static unordered_map<uint32_t, ns3::Time> TimeReceivedData;
	static list<uint32_t> dataID_sent;
	static list<uint32_t> dataID_recv;

private:
	static ofstream* logEEDT_N;
	static ofstream* logEEDT_R;
	static ofstream* logEEFR;
	static ofstream* logOOR;
	static string* logDirPath;
};


#endif /* SRC_NDNSIM_MODEL_GENERIC_LOG_HPP_ */
