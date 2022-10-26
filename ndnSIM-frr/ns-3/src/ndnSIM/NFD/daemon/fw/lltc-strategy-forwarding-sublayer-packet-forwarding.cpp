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

#include "lltc-strategy-common.hpp"

#include <cmath>

namespace nfd {
namespace fw {

// ===================================== Link Layer: Forwarding ==========================================
void LltcStrategy::afterReceiveInterest(const FaceEndpoint& ingress, const Interest& interest,
		const shared_ptr<pit::Entry>& pitEntry) {
	std::cout << "received interest with nounce: " << interest.getNonce() << "at lltcNodeId: " << forwardingStateColl->localNodeId << std::endl;

	if (hasPendingOutRecords(*pitEntry)) return;
	uint32_t rsgId = extractRsgId(interest);
	if (rsgId == UINT_MAX) return;
	ForwardingStateForRSG* fs = this->getForwardingState(rsgId);
	if (fs == NULL) return;

	const Name& name = interest.getName();
	LltcFIBEntry* feUp = this->lookupForwardingEntry(rsgId, fs->primaryPathId, 1);
	if (feUp == nullptr) {
		const fib::Entry& feLocal = this->lookupFib(*pitEntry);
		if (feLocal.getNextHops().size() > 0) {
			for (const auto& nexthop : feLocal.getNextHops()) {
				Face& outFace = nexthop.getFace();
				this->sendInterest(pitEntry, FaceEndpoint(outFace, 0), interest);
			}
		} else {
			LltcFsManager::dumpLltcFS(forwardingStateColl);
			this->rejectPendingInterest(pitEntry);
		}
		return;
	}

	this->sendLltcInterest(pitEntry, interest, FaceEndpoint(*feUp->face_nextHop, 0), rsgId);
}


void LltcStrategy::afterReceiveData(const shared_ptr<pit::Entry>& pitEntry,
		const FaceEndpoint& ingress, const Data& data) {
	Name dataName = data.getName();
	int nDataNameComponents = pitEntry->getName().size();
	string operationStr = dataName.get(nDataNameComponents).toUri(name::UriFormat::DEFAULT);
	string pitPrefixStr = pitEntry->getName().toUri(name::UriFormat::DEFAULT);
	uint32_t rsgId = extractRsgId(data);
	if (rsgId == UINT_MAX) return;
	uint32_t pathId = extractPathId(data, rsgId);

	cout << "%%%%%%%>> " << dataName << " @ NodeID: " << forwardingStateColl->localNodeId << ", path ID: " << pathId <<
			", time: " << Simulator::Now().GetSeconds() << endl;


	if (operationStr.compare(LLTC_MSG_OP_CAPSULE) == 0) {
		if (LltcConfig::LLTC_ENABLE_FAILOVER_ACTIVE_CHECK) {
			doActiveLinkConnectivityCheck(ingress);
		}

		*outLog_in_msgs << LltcRoutesDumper::dumpMsg(true, true, rsgId, pathId, ingress, data) << endl;
		this->onReceiveCapsule(pitEntry, ingress, data, rsgId, pathId);
		return;
	}

	int direction = -1;
	if (operationStr.compare(LLTC_MSG_OP_CAPSULE_ON_RSP) == 0) {
		direction = 0;
	} else {
		return;
	}

	bool isInNonPitRegion = this->forwardInNonPitRegionIfNecessary(pitEntry, data, rsgId,
			pathId, direction);
	if (isInNonPitRegion) {
		*outLog_in_msgs << LltcRoutesDumper::dumpMsg(false, true, rsgId, pathId, ingress, data) << endl;
		return;
	}


	*outLog_in_msgs << LltcRoutesDumper::dumpMsg(true, true, rsgId, pathId, ingress, data) << endl;

}

void LltcStrategy::afterReceiveNonPitData(const FaceEndpoint& ingress, const Data& data) {
	const Name& dataName = data.getName();
	int nDataNameComponents = dataName.size();
	string operationStr = dataName.get(nDataNameComponents - 4).toUri(name::UriFormat::DEFAULT);

	if (operationStr.compare(LLTC_MSG_OP_ECHO) == 0) {
		this->onReceiveEcho(ingress, data);
		*outLog_in_msgs << LltcRoutesDumper::dumpMsg(false, false, -1, -1, ingress, data) << endl;
		return;

	} else if (operationStr.compare(LLTC_MSG_OP_BACK) == 0) {
		this->onReceiveBack(ingress, data);
		*outLog_in_msgs << LltcRoutesDumper::dumpMsg(false, false, -1, -1, ingress, data) << endl;
		return;

	} else if (operationStr.compare(LLTC_MSG_OP_LSA) == 0) {
		this->onReceiveLsa(ingress, data);
		*outLog_in_msgs << LltcRoutesDumper::dumpMsg(false, false, -1, -1, ingress, data) << endl;
		return;
	}


	shared_ptr<lp::LltcPathIdTag> pathIdTag = data.getTag<lp::LltcPathIdTag>();
	if (pathIdTag == nullptr) return;
	uint32_t pathId = (uint32_t) pathIdTag->get();
	cout << "######>> " << dataName << " @ NodeID: " << forwardingStateColl->localNodeId << ", path ID: " << pathId << endl;
	uint32_t rsgId = extractRsgId(data);
	if (rsgId == UINT_MAX) return;

	if (operationStr.compare(LLTC_MSG_OP_CAPSULE_ON_RSP) == 0) {
		LltcFIBEntry* feDown = this->lookupForwardingEntry(rsgId, pathId, 0);
		if (feDown != NULL) {
			this->sendLltcNonPitData(data, FaceEndpoint(*feDown->face_nextHop, 0), rsgId, pathId);
		}
	}

	*outLog_in_msgs << LltcRoutesDumper::dumpMsg(false, true, rsgId, pathId, ingress, data) << endl;
}


void LltcStrategy::afterReceiveNack(const FaceEndpoint& ingress, const lp::Nack& nack,
		const shared_ptr<pit::Entry>& pitEntry) {

}

void LltcStrategy::sendLltcInterest(const shared_ptr<pit::Entry>& pitEntry, const Interest& interest,
		const FaceEndpoint& egress, uint32_t rsgId) {
	interest.setTag<lp::LltcRsgIdTag>(make_shared<lp::LltcRsgIdTag>(rsgId));

	this->sendInterest(pitEntry, egress, interest);
}

void LltcStrategy::sendLltcData(const shared_ptr<pit::Entry>& pitEntry, const Data& data,
							const FaceEndpoint& egress, uint32_t rsgId, uint32_t pathId) {
	data.setTag<lp::LltcPathIdTag>(make_shared<lp::LltcPathIdTag>(pathId));
	data.setTag<lp::LltcTransientTag>(make_shared<lp::LltcTransientTag>(0));
	data.setTag<lp::LltcRsgIdTag>(make_shared<lp::LltcRsgIdTag>(rsgId));

	this->sendData(pitEntry, data, egress);

	*outLog_out_msgs << LltcRoutesDumper::dumpMsg(true, true, rsgId, pathId, egress, data) << endl;
}

void LltcStrategy::sendLltcNonPitData(const Data& data, const FaceEndpoint& egress,
										uint32_t rsgId, uint32_t pathId) {
	data.setTag<lp::LltcPathIdTag>(make_shared<lp::LltcPathIdTag>(pathId));
	data.setTag<lp::LltcTransientTag>(make_shared<lp::LltcTransientTag>(1));
	data.setTag<lp::LltcRsgIdTag>(make_shared<lp::LltcRsgIdTag>(rsgId));

	this->sendNonPitData(data, egress);

	*outLog_out_msgs << LltcRoutesDumper::dumpMsg(false, true, rsgId, pathId, egress, data) << endl;
}

void LltcStrategy::sendLltcNonPitData(const Data& data, const FaceEndpoint& egress) {
	data.setTag<lp::LltcTransientTag>(make_shared<lp::LltcTransientTag>(1));

	this->sendNonPitData(data, egress);

	*outLog_out_msgs << LltcRoutesDumper::dumpMsg(false, false, -1, -1, egress, data) << endl;
}

LltcFIBEntry* LltcStrategy::lookupForwardingEntry(uint32_t rsgId, uint32_t pathId, int direction) {
	if (forwardingStateColl->fssMap.find(rsgId) == forwardingStateColl->fssMap.end()) return NULL;
	ForwardingStateForRSG& forwardingSublayerForwardingState = forwardingStateColl->fssMap[rsgId];

	for (list<LltcFIBEntry*>::iterator iter = forwardingSublayerForwardingState.fibEntries.begin(); iter != forwardingSublayerForwardingState.fibEntries.end(); ++iter) {
		LltcFIBEntry* fe = *iter;
		if (fe->pathId != pathId || fe->direction != direction) continue;
		return fe;
	}
	return NULL;
}

uint32_t LltcStrategy::extractPathId(const Data& data, uint32_t rsgId) {
	shared_ptr<lp::LltcPathIdTag> pathIdTag = data.getTag<lp::LltcPathIdTag>();
	uint32_t pathId = this->getForwardingState(rsgId)->primaryPathId;
	if (pathIdTag != nullptr) {
		pathId = (uint32_t) pathIdTag->get();
	}
	return pathId;
}

uint32_t LltcStrategy::extractRsgId(const Interest& interest) {
	shared_ptr<lp::LltcRsgIdTag> rsgIdTag = interest.getTag<lp::LltcRsgIdTag>();
	uint32_t rsgId = 0;
	if (rsgIdTag != nullptr) {
		rsgId = (uint32_t) rsgIdTag->get();
	} else {
		rsgId = lookupRsgID(interest.getName().toUri(name::UriFormat::DEFAULT));
	}
	return rsgId;
}

uint32_t LltcStrategy::extractRsgId(const Data& data) {
	shared_ptr<lp::LltcRsgIdTag> rsgIdTag = data.getTag<lp::LltcRsgIdTag>();
	uint32_t rsgId = 0;
	if (rsgIdTag != nullptr) {
		rsgId = (uint32_t) rsgIdTag->get();
	} else {
		rsgId = lookupRsgID(data.getName().toUri(name::UriFormat::DEFAULT));
	}
	return rsgId;
}

uint32_t LltcStrategy::lookupRsgID(string prefix) {
	for (pair<rsg_id, ForwardingStateForRSG> item : forwardingStateColl->fssMap) {
		for (string& _prefix : item.second.prefixList) {
			if (_prefix.compare(prefix) == 0) {
				return item.first;
			}
		}
	}
	return UINT_MAX;
}

ForwardingStateForRSG* LltcStrategy::getForwardingState(uint32_t rsgId) {
	if (forwardingStateColl->fssMap.find(rsgId) == forwardingStateColl->fssMap.end()) return NULL;
	return &forwardingStateColl->fssMap[rsgId];
}

// ======================================================================================================


}
}
