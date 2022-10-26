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

#include "lltc-fs.hpp"

#include "ns3/ptr.h"

using namespace ns3::ndn;
using namespace std;


namespace nfd {
namespace fw {

unordered_map<Forwarder*, ForwardingStateCollection*> LltcFsManager::fsCollMap;
unordered_map<uint32_t, ForwardingStateCollection*> LltcFsManager::fsCollMapByNodeId;

LltcFsManager::LltcFsManager() {

}

void LltcFsManager::boundForwarderWithLltcFS(Forwarder* forwarder, uint32_t nodeId, ForwardingStateCollection* fs) {
	fsCollMap[forwarder] = fs;
	fsCollMapByNodeId[nodeId] = fs;
}

void LltcFsManager::registerNonPitApp(uint32_t nodeId, Face* appFace) {
	if (fsCollMapByNodeId.find(nodeId) == fsCollMapByNodeId.end()) return;
	fsCollMapByNodeId[nodeId]->appFace = appFace;
}

ForwardingStateCollection* LltcFsManager::getLltcFsCollectionByForwarder(Forwarder* forwarder) {
	if (fsCollMap.find(forwarder) == fsCollMap.end()) return nullptr;
	ForwardingStateCollection* fsColl = fsCollMap[forwarder];
	return fsColl;
}

ForwardingStateCollection* LltcFsManager::getLltcFsCollectionByNodeId(uint32_t nodeId) {
	if (fsCollMapByNodeId.find(nodeId) == fsCollMapByNodeId.end()) return nullptr;
	ForwardingStateCollection* fsColl = fsCollMapByNodeId[nodeId];
	return fsColl;
}

void LltcFsManager::dumpLltcFS(ForwardingStateCollection *fsColl) {
	cout << "FIB DUMP for with LltcNodeID " << fsColl->localNodeId << " as follows: " << endl;
	for (pair<rsg_id, ForwardingStateForRSG> item : fsColl->fssMap) {
		cout << "--> DUMP FOR RSG ID: " << item.first << endl;
		ForwardingStateForRSG& fs = item.second;

		for (list<LltcFIBEntry*>::iterator iter = fs.fibEntries.begin(); iter != fs.fibEntries.end(); ++iter) {
			LltcFIBEntry* fe = (*iter);
			if (fe->face_nextHop == nullptr) {
				cout << "null, null, " << fe->metric << ", " << fe->pathId << ", " << fe->direction << endl;
			} else {
				cout << fe->face_nextHop->getId() << ", " << fe->face_nextHop->getLocalUri()
									<< ", " << fe->metric << ", " << fe->pathId << ", " << fe->direction << endl;
			}
		}
	}

}

}
}
