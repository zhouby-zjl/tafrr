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

#ifndef SRC_NDNSIM_APPS_LLTC_PDC_HPP_
#define SRC_NDNSIM_APPS_LLTC_PDC_HPP_

#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/lltc-messages-helper.hpp"
#include "ns3/ndnSIM/model/lltc/lltc-config.hpp"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"
#include <list>
#include <unordered_map>
#include <queue>
#include <fstream>


using namespace nfd::fw;
using namespace ns3;
using namespace ns3::ndn;

namespace lltc {

class LltcResequenceQueue;
struct DataElement;

struct PmuState {
	uint32_t nodeID;
	string pmuPrefix;
	uint32_t dataFreq;
	int nResequenceRangeInPackets;
	ns3::Time piat;
	LltcResequenceQueue* queue;
};

class LltcPDCApp : public ns3::ndn::App {
public:
	static ns3::TypeId GetTypeId();

	virtual void DoInitialize();
	virtual void DoDispose();
	virtual void StartApplication();
	virtual void StopApplication();

	virtual void OnInterest(std::shared_ptr<const ns3::ndn::Interest> interest);
	virtual void OnData(std::shared_ptr<const ns3::ndn::Data> contentObject);

	void addPMU(uint32_t nodeID, std::string pmuName, ns3::Time piat, int nResequenceRangeInPackets);
	void setPDCName(std::string pdcName);
	void setNodeID(uint32_t nodeID);

	void onDataArrives(string prefix, CapsuleUri du, shared_ptr<const Data> data);

private:
	void SendInterest();
	bool parseDataUri(const Name& dataName, const string& prefix, CapsuleUri& du);

	std::string pdcName;
	uint32_t nodeId;
	unordered_map<string, PmuState*> pmuStateMap;
	std::ofstream outLog_pdc;
	std::ofstream outLog_pdc_resequenced;
	std::ofstream outLog_pdc_q_reseq_size;
};


struct DataElement {
	CapsuleUri du;
	ns3::Time arriveTime;
	shared_ptr<const Data> data;

	friend bool operator < (const DataElement& de1, const DataElement& de2) {
		return de1.du.dataId > de2.du.dataId;
	}
};

class LltcResequenceQueue {
public:
	LltcResequenceQueue(string prefix, int queueSize, ns3::Time maxWaitTime);
	void receiveData(CapsuleUri du, shared_ptr<const Data> data);
	void releaseQueue(ns3::Time curTime);
	void boundApp(LltcPDCApp* app);
	size_t getQueueSize();

private:
	void sendData(CapsuleUri du, shared_ptr<const Data> data);
	void autoDequeueTask(LltcResequenceQueue* rq);

	size_t size;
	ns3::Time maxWaitTime;
	int64_t lastDataId;

	priority_queue<DataElement> q_seq;
	queue<DataElement> q_time;

	ns3::EventId curAutoDequeueTaskEvent;
	string prefix;
	LltcPDCApp* app;
};


}



#endif /* SRC_NDNSIM_APPS_LLTC_PDC_HPP_ */
