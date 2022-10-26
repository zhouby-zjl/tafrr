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

#ifndef SRC_NDNSIM_NFD_DAEMON_FW_LLTC_FS_HPP_
#define SRC_NDNSIM_NFD_DAEMON_FW_LLTC_FS_HPP_

#include "NFD/daemon/face/face-common.hpp"
#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ptr.h"
#include "NFD/daemon/fw/forwarder.hpp"
#include "NFD/daemon/face/face-common.hpp"
#include "model/lltc/lltc-resilient-routes-subgraph.hpp"

#include <list>
#include <unordered_map>

using namespace ns3::ndn;
using namespace std;
using namespace nfd::face;

namespace nfd {
namespace fw {

struct LltcFIBEntry {
	uint32_t pathId;
	Face* face_nextHop;
	int metric;
	int direction;  // 0 stands for the downstream, and 1 for the upstream
//	int remoteLltcNodeId;   // this is for debugging
	int retransTimeout;
	bool isRetransPoint;   // indicates whether it is a retrans point or a intermediate transit point.
};

struct LltcFaceEntry {
	Face* face_nextHop;
	int remoteLltcNodeId;
	int status;       // the values listed in the FACE_STATUS_* macros below
	ns3::Time lastCheckTime;
	vector<int> num_received_Capsules_in_recent_secs;
	int last_count_secs;
	int init_secs;
};

#define FACE_STATUS_NORMAL    0
#define FACE_STATUS_FAILURE   1

typedef uint32_t rsg_id;

struct ForwardingStateForRSG {
	list<string> prefixList;
	list<LltcFIBEntry*> fibEntries;
	uint32_t primaryPathId;
	uint64_t preferredDownPathId;
	uint64_t preferredUpPathId;
	bool isDownPathInRspState;
	bool isUpPathInRspState;
	int waitingTimeoutOnNextPacket;
	int waitingTimeoutOnNextPacketForLostReport;
	//int maxTimeout_waitingRetransFromUpRouterInUs;
	unordered_map<int, int> waitingTimeoutOnRetransReport;

	lltc::rsg_struct rsg;
};

struct ForwardingStateCollection {
	unordered_map<rsg_id, ForwardingStateForRSG> fssMap;
	list<LltcFaceEntry*> faceEntries;
	Face* appFace;
	uint32_t localNodeId;
};

class LltcFsManager {
public:
	LltcFsManager();
	static void boundForwarderWithLltcFS(Forwarder* forwarder, uint32_t nodeId, ForwardingStateCollection* fsColl);
	static void registerNonPitApp(uint32_t nodeId, Face* appFace);
	static ForwardingStateCollection* getLltcFsCollectionByForwarder(Forwarder* forwarder);
	static ForwardingStateCollection* getLltcFsCollectionByNodeId(uint32_t nodeId);
	static void dumpLltcFS(ForwardingStateCollection *fsColl);

private:
	static unordered_map<Forwarder*, ForwardingStateCollection*> fsCollMap;
	static unordered_map<uint32_t, ForwardingStateCollection*> fsCollMapByNodeId;
};

}
}


#endif /* SRC_NDNSIM_NFD_DAEMON_FW_LLTC_FS_HPP_ */
