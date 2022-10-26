/*
 * This work is licensed under CC BY-NC-SA 4.0
 * (https://creativecommons.org/licenses/by-nc-sa/4.0/).
 * Copyright (c) 2021 Boyang Zhou
 *
 * This file is a part of "Disruption Resilient Transport Protocol"
 * (https://github.com/zhouby-zjl/drtp/).
 * Written by Boyang Zhou (zhouby@zhejianglab.com)
 *
 * This software is protected by the patents numbered with PCT/CN2021/075891,
 * ZL202110344405.7 and ZL202110144836.9, as well as the software copyrights
 * numbered with 2020SR1875227 and 2020SR1875228.
 */

#ifndef SRC_NDNSIM_MODEL_LLTC_LLTC_UTILS_HPP_
#define SRC_NDNSIM_MODEL_LLTC_LLTC_UTILS_HPP_

#include "ns3/ndnSIM/model/lltc/lltc-resilient-routes-generation.hpp"
#include "ns3/ndnSIM/model/lltc/lltc-resilient-routes-subgraph.hpp"

#include "ns3/ndnSIM/NFD/daemon/fw/lltc-messages-helper.hpp"
#include "NFD/daemon/face/face-common.hpp"

#include "ns3/nstime.h"

#include <string>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <fstream>

using namespace lltc;
using namespace std;
using namespace ns3::ndn;
using namespace nfd;

#define LOG_IN_MSGS   0
#define LOG_OUT_MSGS  1

struct retran_event {
	uint32_t nodeId;
	uint32_t rsgId;
	uint32_t pathId;
	uint32_t dataId;
	uint32_t type;
};

struct num_retran_paths {
	uint32_t nodeId;
	uint32_t rsgId;
	uint32_t numRetranPaths;
};

class LltcLog {
public:
	static void setLogDirPath(const char* logDirPath);
	static void openLogs(int nRouters);
	static ofstream* getLogOutput(uint32_t routerID, int logType);
	static ofstream* getLogRetranEvents();
	static ofstream* getLogNumRetranPaths();
	static ofstream* getLogControlPlane();
	static ofstream* getLogFailovers();
	static ofstream* getLogFailureEvents();
	static ofstream* getLogFailoversActiveChecks();
	static ofstream* getLinkTestMsgs();
	static ofstream* getLogRoutesDump();
	static ofstream* getLogRrLossRatesEvaluation();
	static ofstream* getLogRrDelaysEvaluation();
	static void flushRouterLogs(uint32_t routerID);
	static void closeLogs();
	static void cleanLogDir();

	static list<retran_event>* getRetranEvents();
	static list<num_retran_paths>* getNumRetranPaths();

private:
	static unordered_map<uint32_t, ofstream*> logsInMsgs;
	static unordered_map<uint32_t, ofstream*> logsOutMsgs;
	static ofstream* logRetranEvents;
	static ofstream* logNumRetranPaths;
	static ofstream* logControlPlane;
	static ofstream* logFailovers;
	static ofstream* logFailureEvents;
	static ofstream* logFailoversActiveChecks;
	static ofstream* logRoutesDump;
	static ofstream* logRrLossRatesEvaluation;
	static ofstream* logRrDelaysEvaluation;
	static ofstream* logLinkTestMsgs;
	static string* logDirPath;

	static list<retran_event>* retranEvents;
	static list<num_retran_paths>* numRetranPaths;
};


class LltcRoutesDumper {
public:
	static string dumpPath(Path* path);
	static string dumpRR(ResilientRoutes* rr);
	static string dumpMsg(bool isPIT, bool isRSG, uint32_t rsgId, uint32_t pathId, const FaceEndpoint& port, const Data& data);
};




#endif /* SRC_NDNSIM_MODEL_LLTC_LLTC_UTILS_HPP_ */
