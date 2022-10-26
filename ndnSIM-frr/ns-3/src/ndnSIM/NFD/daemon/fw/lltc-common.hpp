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

#ifndef SRC_NDNSIM_NFD_DAEMON_FW_LLTC_COMMON_HPP_
#define SRC_NDNSIM_NFD_DAEMON_FW_LLTC_COMMON_HPP_

#include "strategy.hpp"
#include "algorithm.hpp"
#include "common/global.hpp"
#include "common/logger.hpp"
#include "ndn-cxx/tag.hpp"

#include <list>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <fstream>

using namespace std;

namespace nfd {
namespace fw {

#define LLTC_MSG_OP_CAPSULE           				"Capsule"
#define LLTC_MSG_OP_CAPSULE_ON_RSP          		"CapsuleOnRSP"
#define LLTC_MSG_OP_DOWN_SIDE_CONTROL				"DownSideControl"
#define LLTC_MSG_OP_UP_SIDE_CONTROL					"UpSideControl"

#define LLTC_MSG_OP_RETRANS_IN_SINGLE 				"RetranS"
#define LLTC_MSG_OP_RETRANS_IN_MULTIPLE  			"RetranM"
#define LLTC_MSG_OP_REQUEST_IN_SINGLE 				"RequestS"
#define LLTC_MSG_OP_REQUEST_IN_MULTIPLE 			"RequestM"
#define LLTC_MSG_OP_REPORT            				"Report"

#define LLTC_MSG_OP_LSA								"LSA"
#define LLTC_MSG_OP_ECHO		    				"Echo"
#define LLTC_MSG_OP_BACK			    			"Back"

typedef unsigned int LltcDataID;
typedef uint64_t DataAndPathID;
typedef uint64_t RetransInMultipleID;

struct RetransInSingleEvent {
	ns3::EventId e;
	ns3::Time reqTime;
};

struct WaitCapsuleForActiveFailoverEvent {
	ns3::EventId e;
	ns3::Time waitTime;
};

struct RetransInMultipleEvent {
	ns3::EventId e;
	ns3::Time time;
	list<LltcDataID> retransDataIds;
};

class LltcDataIdQueue {
public:
	LltcDataIdQueue();
	~LltcDataIdQueue();

	void setLen(size_t n);
	void push(LltcDataID x, uint64_t time);
	LltcDataID pop();
	bool contains(LltcDataID x);
	size_t size();

	uint64_t getDataIdAddTime(LltcDataID dataID);

private:
	queue<LltcDataID> q;
	unordered_set<LltcDataID> set;
	unordered_map<LltcDataID, uint64_t> dataIdAddTimeMap;
	size_t n;
};

struct LinkState {
	int neighboredNodeId;
	int status;
};


struct EchoUri {
	uint32_t echoId;
	uint32_t sourceNodeId;
};

struct BackUri {
	uint32_t echoId;
	uint32_t sourceNodeId;
	uint32_t destNodeId;
};

struct LsaUri {
	uint32_t lsaId;
	uint32_t sourceNodeId;
	uint32_t updateSeqNo;
	string type;
};

struct ReportUri {
	string prefix;
	uint32_t sourceNodeId;
	uint32_t reportUID;
};

struct RequestSUri {
	string prefix;
	LltcDataID lostDataId;
	uint32_t sourceNodeId;
	string type;
};

struct RetranSUri {
	string prefix;
	LltcDataID dataId;
	uint32_t sourceNodeId;
	int type;  // 0 for exact, 1 for closest, 2 for mismatch
	LltcDataID requestedDataId;
	uint64_t driftingDelta;
};

struct RetranMUri {
	string prefix;
	RetransInMultipleID bri;
	uint32_t sourceNodeId;
	bool isExact;
	LltcDataID dataId;
	LltcDataID requestedDataId;
};

struct RequestMUri {
	string prefix;
	RetransInMultipleID rbi;
	uint32_t sourceNodeId;
	string type;
};

struct CapsuleUri {
	string prefix;
	LltcDataID dataId;
	bool isRetrans;
	LltcDataID requestedDataId;
};

struct DownSideControlUri {
	string prefix;
	uint32_t sourceNodeId;
	uint32_t dstNodeId;
};

struct UpSideControlUri {
	string prefix;
	uint32_t sourceNodeId;
	uint32_t dstNodeId;
};

struct ChangePathUri {
	string prefix;
	uint32_t nodeIdToChangePath;
	uint32_t newPathId;
	string ununused;
};

struct CapsuleOnRSPUri {
	string prefix;
	LltcDataID dataId;
	bool isRetrans;
	LltcDataID requestedDataId;
};


}
}
#endif /* SRC_NDNSIM_NFD_DAEMON_FW_LLTC_COMMON_HPP_ */
