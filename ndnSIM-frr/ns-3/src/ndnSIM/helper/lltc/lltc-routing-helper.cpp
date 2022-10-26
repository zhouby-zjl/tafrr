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

#include "lltc-routing-helper.hpp"

#include "model/lltc/lltc-router.hpp"
#include "model/ndn-l3-protocol.hpp"
#include "model/lltc/lltc-graph.hpp"
#include "model/ndn-net-device-transport.hpp"
#include "NFD/daemon/fw/face-table.hpp"
#include "NFD/daemon/face/face-common.hpp"
#include "NFD/daemon/fw/lltc-common.hpp"
#include <utility>
#include "../../NFD/daemon/fw/lltc-fs.hpp"
#include "model/lltc/lltc-utils.hpp"


namespace lltc {

using namespace std;

LltcRoutingHelper::LltcRoutingHelper(LltcGraph* lltcGraph, LltcConfiguration* config, int routingType) {
	rrg = new ResilientRouteGenerationForReroute();

	this->lltcGraph = lltcGraph;
	this->config = config;
	this->lastCommPairID = 0;
}

LltcRoutingHelper::~LltcRoutingHelper() {
}

ResilientRouteGenerationGeneric* LltcRoutingHelper::getRRG() {
	return this->rrg;
}

LltcGraph* LltcRoutingHelper::getGraph() {
	return this->lltcGraph;
}

LltcConfiguration* LltcRoutingHelper::getConfig() {
	return this->config;
}

void LltcRoutingHelper::Install(Ptr<Node> node, LltcNode* lltcNode) {
	Ptr<L3Protocol> proto = node->GetObject<L3Protocol>();
	Ptr<LltcRouter> router = node->GetObject<LltcRouter>();

	if (router != 0) {
		return;
	}

	router = CreateObject<LltcRouter>(lltcNode);
	node->AggregateObject(router);

	NodeContainer nodes = NodeContainer::GetGlobal();
	nfd::FaceTable& table = proto->getFaceTable();
	for (nfd::FaceTable::const_iterator iter = table.begin(); iter != table.end(); ++iter) {
		auto transport = dynamic_cast<NetDeviceTransport*>(iter->getTransport());
		nfd::face::Face* face = table.get(iter->getId());
		if (transport == nullptr) {
			continue;
		}
		Ptr<NetDevice> nd = transport->GetNetDevice();
		if (nd == 0) continue;
		Ptr<Channel> ch = nd->GetChannel();
		if (ch == 0) continue;

		if (ch->GetNDevices() == 2) {
			Ptr<NetDevice> firstDevice = ch->GetDevice(0);
			Ptr<NetDevice> otherSide = (firstDevice == nd) ? ch->GetDevice(1) : firstDevice;
			Ptr<Node> otherNode = otherSide->GetNode();

			int otherSideNodeId = -1;
			for (unsigned int i = 0; i < nodes.size(); ++i) {
				Ptr<Node> _node = nodes.Get(i);
				if (otherNode == _node) {
					otherSideNodeId = i;
					break;
				}
			}
			if (otherSideNodeId == -1) continue;
			int linkId = lltcGraph->getLinkByNodeIds(lltcNode->nodeId, otherSideNodeId)->linkId;
			router->faceMap[linkId] = face;
		}
	}

	ForwardingStateCollection* fsColl = new ForwardingStateCollection;
	fsColl->localNodeId = lltcNode->nodeId;
	fsColl->appFace = NULL;

	for (unordered_map<int, nfd::face::Face*>::const_iterator iter2 = router->faceMap.begin();
						iter2 != router->faceMap.end(); ++iter2) {
		LltcFaceEntry* faceEntry = new LltcFaceEntry;
		faceEntry->face_nextHop = iter2->second;
		faceEntry->lastCheckTime = Simulator::Now();
		faceEntry->status = FACE_STATUS_NORMAL;
		faceEntry->remoteLltcNodeId = -1;
		faceEntry->last_count_secs = -1;
		faceEntry->init_secs = -1;
		fsColl->faceEntries.push_back(faceEntry);
	}

	LltcFsManager::boundForwarderWithLltcFS(proto->getForwarder().get(), lltcNode->nodeId, fsColl);
}

void LltcRoutingHelper::InstallAll() {
	NodeContainer nodes = NodeContainer::GetGlobal();
	for (unsigned int i = 0; i < nodes.size(); ++i) {
		Install(nodes.Get(i), lltcGraph->nodes[i]);
	}
}

void LltcRoutingHelper::AddOrigin(const std::string& prefix, Ptr<Node> node) {
	Ptr<LltcRouter> router = node->GetObject<LltcRouter>();
	if (router == 0) {
		return;
	}
	auto name = make_shared<Name>(prefix);
	router->addLocalPrefix(name);
}

void LltcRoutingHelper::computeRoutesVectors() {
	for (list<comm_pair*>::iterator iter = commPairs.begin(); iter != commPairs.end(); ++iter) {
			Ptr<Node> producerNode = (*iter)->nodes.first;
			Ptr<LltcRouter> producerRouter = producerNode->GetObject<LltcRouter>();
			int producerNodeId = producerRouter->node->nodeId;

			if (rrvMap.find(producerNodeId) == rrvMap.end()) {
				LltcResilientRouteVectors* rrv = rrg->genResilientRoutesVectors(lltcGraph, producerNodeId, config);
				rrvMap[producerNodeId] = rrv;
			}
	}

}

void LltcRoutingHelper::computeRoutesForCommPairs() {
	fs_coll_map fsCollMap;

	ofstream* logOut_controlPlane = LltcLog::getLogControlPlane();
	ofstream* logOut_routesDump = LltcLog::getLogRoutesDump();

	stringstream ss;
	for (list<comm_pair*>::iterator iter = commPairs.begin(); iter != commPairs.end(); ++iter) {
		Ptr<Node> producerNode = (*iter)->nodes.first;
		Ptr<Node> consumerNode = (*iter)->nodes.second;
		Ptr<LltcRouter> producerRouter = producerNode->GetObject<LltcRouter>();
		Ptr<LltcRouter> consumerRouter = consumerNode->GetObject<LltcRouter>();
		int producerNodeId = producerRouter->node->nodeId;
		int consumerNodeId = consumerRouter->node->nodeId;

		if (rrvMap.find(producerNodeId) == rrvMap.end()) continue;
		LltcResilientRouteVectors* rrv = rrvMap[producerNodeId];
		ResilientRoutes* rr = rrg->constructResilientRoutes(lltcGraph, rrv, consumerNodeId, config);

		(*iter)->rr = rr;
		ss.str("");
		ss << "----------- RR DUMP (" << producerNodeId << ", " << consumerNodeId << ") ----------------";
		string rrDumpTitle = ss.str();
		string* rrDumpContent = rr->toString();
		cout << rrDumpTitle << endl;
		cout << *rrDumpContent << endl;

		rsg_struct* rsg = LltcRrSubgraph::genLltcRrSubgraph(rr, lltcGraph);

		*logOut_controlPlane << rrDumpTitle << endl << *rrDumpContent << endl;
		logOut_controlPlane->flush();
		*logOut_routesDump << consumerNodeId << "," << producerNodeId << "," << rsg->rsgId << "," << rr->expectedRrLossRate << "," << rr->maxRrDelay << "," <<
							rr->primPathDelay << "," <<	LltcRoutesDumper::dumpPath(rr->primaryPath) << "," << LltcRoutesDumper::dumpRR(rr) << endl;
		logOut_routesDump->flush();

		fs_map* fsMap = convertToFSes(rr, producerRouter->getLocalPrefixList(), rsg);

		for (pair<uint32_t, ForwardingStateForRSG*> fsPair : *fsMap) {
			ForwardingStateCollection* fsColl = NULL;
			if (fsCollMap.find(fsPair.first) == fsCollMap.end()) {
				fsColl = new ForwardingStateCollection;
				fsColl->appFace = NULL;
				fsColl->localNodeId = fsPair.first;
				fsCollMap[fsPair.first] = fsColl;
			} else {
				fsColl = fsCollMap[fsPair.first];
			}

			fsColl->fssMap[rsg->rsgId] = *fsPair.second;
		}
	}

	enforceFs(&fsCollMap);
}


LltcRoutingHelper::fs_map* LltcRoutingHelper::convertToFSes(ResilientRoutes* rr,
								LocalPrefixList* producerNodePrefixList, rsg_struct* rsg) {
	fs_map* fsMap = new fs_map;
	uint32_t primPathId = rr->primaryPath->pathID;
	addFibsFromPath(rr->primaryPath, fsMap, primPathId, true, primPathId, -1, rsg);

	for (unsigned int i = 1; i < rr->primaryPath->nodeIds->size(); ++i) {
		vector<RedundantSubPath*>* rsps = rr->getRSPsByNodeId((*rr->primaryPath->nodeIds)[i]);
		if (rsps == NULL) continue;
		for (RedundantSubPath* rsp : *rsps) {
			addFibsFromPath(rsp->path, fsMap, rsp->path->pathID, false, primPathId, rsp->retransDelayInSingleTime, rsg);
		}
	}

	for (unsigned int i = 0; i < rr->primaryPath->nodeIds->size(); ++i) {
		int nodeId = (*rr->primaryPath->nodeIds)[i];
		ForwardingStateForRSG* fs = (*fsMap)[nodeId];
		fs->waitingTimeoutOnNextPacket = (*rr->waitingTimeoutOnNextPacketInUs)[i];
		fs->waitingTimeoutOnNextPacketForLostReport = (*rr->waitingTimeoutOnNextPacketForLostReportInUs)[i];
		fs->waitingTimeoutOnRetransReport = *(rr->waitingTimeoutOnRetransReportInUs[i]);
	}

	for (pair<uint32_t, ForwardingStateForRSG*> item : *fsMap) {
		for (shared_ptr<Name> prefix : *producerNodePrefixList) {
			item.second->prefixList.push_back(prefix->toUri(name::UriFormat::DEFAULT));
		}
	}

	//dumpFs(fsMap);
	return fsMap;
}



// TODO: primaryPathID and pathID should be saved in map structure with consideration on the comm pair.
// TODO: The entries should be merged during the add operation below.
void LltcRoutingHelper::addFibsFromPath(Path* path, fs_map* fsMap, uint32_t pathId,	bool isPrimPath, uint32_t primPathId,
									int timeoutForRetrans, rsg_struct* rsg) {
	NodeContainer nodes = NodeContainer::GetGlobal();
	size_t nNodes = path->nodeIds->size();

	for (unsigned int i = 0; i < nNodes - 1; ++i) {
		int curNodeId = (*path->nodeIds)[i];
		int nextNodeId = (*path->nodeIds)[i + 1];

		Ptr<LltcRouter> curRouter = nodes.Get(curNodeId)->GetObject<LltcRouter>();
		Ptr<LltcRouter> nextRouter = nodes.Get(nextNodeId)->GetObject<LltcRouter>();

		ForwardingStateForRSG* curRouterFib = getRouterFS(fsMap, curNodeId);
		ForwardingStateForRSG* nextRouterFib = getRouterFS(fsMap, nextNodeId);

		LltcLink* downLink = lltcGraph->getLinkByNodeIds(curNodeId, nextNodeId);
		LltcLink* upLink = lltcGraph->getLinkByNodeIds(nextNodeId, curNodeId);

		LltcFIBEntry* feCur = new LltcFIBEntry;

		feCur->face_nextHop = curRouter->faceMap[downLink->linkId];
		feCur->metric = lltcGraph->getLinkDelayTotal(downLink, curNodeId, nextNodeId);
		feCur->direction = isPrimPath ? 0 : 1;
		feCur->pathId = pathId;
		//feCur->remoteLltcNodeId = nextNodeId;
		curRouterFib->fibEntries.push_back(feCur);

		LltcFIBEntry* feNext = new LltcFIBEntry;
		feNext->pathId = pathId;
		feNext->face_nextHop = nextRouter->faceMap[upLink->linkId];
		feNext->direction = isPrimPath ? 1 : 0;
		feNext->metric = lltcGraph->getLinkDelayTotal(upLink, nextNodeId, curNodeId);
		//feNext->remoteLltcNodeId = curNodeId;
		nextRouterFib->fibEntries.push_back(feNext);

		if (isPrimPath) {
			feNext->retransTimeout = feCur->metric + feNext->metric;
			feNext->isRetransPoint = true;

			feCur->retransTimeout = -1;
			feCur->isRetransPoint = true;

		} else {
			if (i == 0) {
				feCur->retransTimeout = timeoutForRetrans;
				feCur->isRetransPoint = true;
			} else {
				feCur->retransTimeout = -1;
				feCur->isRetransPoint = false;
			}

			feNext->retransTimeout = -1;
			feNext->isRetransPoint = false;
		}

		list<LltcFaceEntry*> faList_curRouter = LltcFsManager::getLltcFsCollectionByNodeId(curRouter->node->nodeId)->faceEntries;
		list<LltcFaceEntry*> faList_nextRouter = LltcFsManager::getLltcFsCollectionByNodeId(nextRouter->node->nodeId)->faceEntries;

		for (list<LltcFaceEntry*>::const_iterator iterFa = faList_curRouter.begin();
				iterFa != faList_curRouter.end(); ++iterFa) {
			if ((*iterFa)->face_nextHop == feCur->face_nextHop) {
				(*iterFa)->remoteLltcNodeId = nextNodeId;
			}
		}

		for (list<LltcFaceEntry*>::const_iterator iterFa = faList_nextRouter.begin();
				iterFa != faList_nextRouter.end(); ++iterFa) {
			if ((*iterFa)->face_nextHop == feNext->face_nextHop) {
				(*iterFa)->remoteLltcNodeId = curNodeId;
			}
		}

	}

	for (size_t i = 0; i < nNodes; ++i) {
		int nodeId = (*path->nodeIds)[i];
		ForwardingStateForRSG* routerFs = (*fsMap)[nodeId];
		if (routerFs->preferredDownPathId == UINT_MAX) {
			routerFs->preferredDownPathId = pathId;
		}
		if (routerFs->preferredUpPathId == UINT_MAX) {
			routerFs->preferredUpPathId = pathId;
		}

		routerFs->primaryPathId = primPathId;
		routerFs->rsg = *rsg;
	}
}

void LltcRoutingHelper::enforceFs(fs_coll_map* fsCollMap) {
	NodeContainer nodes = NodeContainer::GetGlobal();
	for (fs_coll_map::iterator iter = fsCollMap->begin(); iter != fsCollMap->end(); ++iter) {
		int nodeId = iter->first;
		ForwardingStateCollection* fsColl = iter->second;
		nfd::Forwarder* forwarder = nullptr;

		for (unsigned int i = 0; i < nodes.size(); ++i) {
			Ptr<Node> node = nodes.Get(i);
			Ptr<LltcRouter> router = node->GetObject<LltcRouter>();
			if (router->node->nodeId == nodeId) {
				Ptr<L3Protocol> proto = node->GetObject<L3Protocol>();
				forwarder = proto->getForwarder().get();
				break;
			}
		}

		if (forwarder == nullptr) continue;

		nfd::fw::ForwardingStateCollection* fsCollDP = LltcFsManager::getLltcFsCollectionByForwarder(forwarder);
		fsCollDP->appFace = fsColl->appFace;
		fsCollDP->localNodeId = fsColl->localNodeId;

		for (list<LltcFaceEntry*>::iterator faIter = fsColl->faceEntries.begin(); faIter != fsColl->faceEntries.end(); ++faIter) {
			fsCollDP->faceEntries.push_back(*faIter);
		}

		for (pair<rsg_id, ForwardingStateForRSG> item : fsColl->fssMap) {
			ForwardingStateForRSG& fsDP = fsCollDP->fssMap[item.first];
			ForwardingStateForRSG& fs = fsColl->fssMap[item.first];

			for (list<LltcFIBEntry*>::iterator feIter = fs.fibEntries.begin(); feIter != fs.fibEntries.end(); ++feIter) {
				fsDP.fibEntries.push_back(*feIter);
			}

			fsDP.preferredDownPathId = fs.preferredDownPathId;
			fsDP.preferredUpPathId = fs.preferredUpPathId;
			fsDP.isDownPathInRspState = fs.isDownPathInRspState;
			fsDP.isUpPathInRspState = fs.isUpPathInRspState;
			fsDP.primaryPathId = fs.primaryPathId;
			fsDP.prefixList = fs.prefixList;

			fsDP.rsg = fs.rsg;
			fsDP.waitingTimeoutOnNextPacket = fs.waitingTimeoutOnNextPacket;
			fsDP.waitingTimeoutOnNextPacketForLostReport = fs.waitingTimeoutOnNextPacketForLostReport;
			fsDP.waitingTimeoutOnRetransReport = fs.waitingTimeoutOnRetransReport;
		}

	}

}


ForwardingStateForRSG* LltcRoutingHelper::getRouterFS(fs_map* fsMap, uint32_t nodeId) {
	ForwardingStateForRSG* routerFS = nullptr;
	if (fsMap->find(nodeId) != fsMap->end()) {
		routerFS = (*fsMap)[nodeId];
	} else {
		routerFS = new ForwardingStateForRSG;
		routerFS->preferredDownPathId = UINT_MAX;
		routerFS->preferredUpPathId = UINT_MAX;
		routerFS->primaryPathId = UINT_MAX;
		routerFS->isDownPathInRspState = false;
		routerFS->isUpPathInRspState = false;
		(*fsMap)[nodeId] = routerFS;
	}
	return routerFS;
}

void LltcRoutingHelper::addCommPair(Ptr<Node> producerNode, Ptr<Node> consumerNode) {
	comm_pair* commPair = new comm_pair();
	commPair->nodes.first = producerNode;
	commPair->nodes.second = consumerNode;
	commPair->rr = NULL;
	commPair->id = lastCommPairID++;
	commPairs.push_back(commPair);
}

list<comm_pair*>* LltcRoutingHelper::getCommPairs() {
	return &commPairs;
}

void LltcRoutingHelper::dumpRouterLinkFaceIDs() {
	NodeContainer nodes = NodeContainer::GetGlobal();
	for (unsigned int i = 0; i < nodes.size(); ++i) {
		Ptr<Node> node = nodes.Get(i);
		Ptr<LltcRouter> router = node->GetObject<LltcRouter>();
		if (router == 0) continue;
		cout << *router->toString() << endl;
	}
}

void LltcRoutingHelper::dumpFs(fs_map* fsMap) {
	for (fs_map::iterator iter = fsMap->begin(); iter != fsMap->end(); ++iter) {
		int nodeId = iter->first;
		ForwardingStateForRSG* fib = iter->second;
		cout << "---------------- FIB DUMP FOR NODE ID " << nodeId << " -------------------" << endl;
		cout << "preferredPathId: " << fib->preferredDownPathId << ", primPathId: " << fib->primaryPathId << endl;
		for (list<LltcFIBEntry*>::iterator iter2 = fib->fibEntries.begin(); iter2 != fib->fibEntries.end(); ++iter2) {
			LltcFIBEntry* fe = *iter2;
			if (fe->face_nextHop == NULL) {
				cout << "NULL, " << fe->pathId << ", " << fe->metric << ", " << fe->direction << ", "
								<< fe->retransTimeout << ", " << fe->isRetransPoint << endl;
			} else {
				cout << *fe->face_nextHop << ", " << fe->pathId << ", " << fe->metric << ", " << fe->direction << ", "
								<< fe->retransTimeout << ", " << fe->isRetransPoint << endl;
			}
		}
	}
}

}
