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
#include <string>

namespace nfd {
namespace fw {

NFD_LOG_INIT(LltcStrategy);
NFD_REGISTER_STRATEGY(LltcStrategy);

LltcStrategy::LltcStrategy(Forwarder& forwarder, const Name& name) : Strategy(forwarder) {
	this->setInstanceName(makeInstanceName(name, getStrategyName()));
	forwardingStateColl = LltcFsManager::getLltcFsCollectionByForwarder(&forwarder);
	localCs = &forwarder.getCs();
	forwarder.lltcStrategy = this;
	localCs->setPolicy(make_unique<PriorityFifoPolicy>());
	localCs->setLimit(LltcStrategyConfig::csLimitInEntries);

	if (forwardingStateColl->localNodeId == 100) {
		cout << "here" << endl;
	}

	for (pair<rsg_id, ForwardingStateForRSG> item : forwardingStateColl->fssMap) {
		ForwardingStateForRSG& linkLayerForwardingState = item.second;
		linkLayerForwardingState.preferredDownPathId = linkLayerForwardingState.primaryPathId;
		linkLayerForwardingState.isDownPathInRspState = false;
		linkLayerForwardingState.preferredUpPathId = linkLayerForwardingState.primaryPathId;
		linkLayerForwardingState.isUpPathInRspState = false;
	}

	for (LltcFaceEntry* fa : forwardingStateColl->faceEntries) {
		fa->status = FACE_STATUS_NORMAL;
	}

	linkMeasurementState.prevLsa = nullptr;
	linkMeasurementState.nextLinkTestID = 0;
	linkMeasurementState.isCheckingLinkStatesInPeriodInRunning = false;

	if (LltcStrategyConfig::enableFailover && forwardingStateColl->fssMap.size() > 0) {
		bool isRSGNode = isPPNode() || isRSPNode();
		if (isRSGNode) {
			Simulator::Schedule(Seconds(LltcStrategyConfig::delayTimeForRunningCheckLinkConnectivity),
								&LltcStrategy::checkLinkConnectivityInPeriodic, this);
		}
	}

	outLog_in_msgs = LltcLog::getLogOutput(this->forwardingStateColl->localNodeId, LOG_IN_MSGS);
	outLog_out_msgs = LltcLog::getLogOutput(this->forwardingStateColl->localNodeId, LOG_OUT_MSGS);

	ofstream* logNumRetranPaths = LltcLog::getLogNumRetranPaths();
	list<num_retran_paths>* numRetranPathsList = LltcLog::getNumRetranPaths();
	for (unordered_map<rsg_id, ForwardingStateForRSG>::iterator iter =
				forwardingStateColl->fssMap.begin();
			iter != forwardingStateColl->fssMap.end();
			++iter) {
		rsg_id rsgId = iter->first;
		ForwardingStateForRSG& forwardingSublayerForwardingState = iter->second;

		int numRetranPaths = 0;
		for (list<LltcFIBEntry*>::iterator iter = forwardingSublayerForwardingState.fibEntries.begin();
											iter != forwardingSublayerForwardingState.fibEntries.end(); ++iter) {
			LltcFIBEntry* fe = *iter;
			if (fe->direction != 1 || !fe->isRetransPoint) continue;
			numRetranPaths++;
		}

		*logNumRetranPaths << forwardingStateColl->localNodeId << "," <<
				rsgId << "," << numRetranPaths << endl;

		num_retran_paths nrp;
		nrp.nodeId = forwardingStateColl->localNodeId;
		nrp.rsgId = rsgId;
		nrp.numRetranPaths = numRetranPaths;
		numRetranPathsList->push_back(nrp);
	}

}

LltcStrategy::~LltcStrategy() {
	outLog_in_msgs->flush();
	outLog_out_msgs->flush();
}

const Name& LltcStrategy::getStrategyName() {
	static Name strategyName("ndn:/localhost/nfd/strategy/lltc/%FD%01");
	return strategyName;
}

bool LltcStrategy::isPPNode() {
	bool isPPNode = false;
	for (unordered_map<rsg_id, ForwardingStateForRSG>::iterator iter = forwardingStateColl->fssMap.begin();
			iter != forwardingStateColl->fssMap.end(); ++iter) {
		for (vector<rsg_node*>::iterator iter2 = iter->second.rsg.nodes.begin(); iter2 != iter->second.rsg.nodes.end(); ++iter2) {
			if ((*iter2)->nodeID == forwardingStateColl->localNodeId) {
				isPPNode = true;
				break;
			}
		}
	}
	return isPPNode;
}

bool LltcStrategy::isRSPNode() {
	bool isRSPNode = false;
	for (unordered_map<rsg_id, ForwardingStateForRSG>::iterator iter = forwardingStateColl->fssMap.begin();
						iter != forwardingStateColl->fssMap.end(); ++iter) {
		for (unordered_map<rsg_node_id, vector<rsg_path_compact>>::iterator iter2 = iter->second.rsg.rsg_compacted_rr.hrsp.begin();
				iter2 != iter->second.rsg.rsg_compacted_rr.hrsp.end(); ++iter2) {
			for (vector<rsg_path_compact>::iterator iter3 = iter2->second.begin(); iter3 != iter2->second.end(); ++iter3) {
				if (std::find(iter3->nodeIds.begin(), iter3->nodeIds.end(), forwardingStateColl->localNodeId) != iter3->nodeIds.end()) {
					isRSPNode = true;
					break;
				}
			}

		}

		if (isRSPNode) break;
	}
	return isRSPNode;
}


}
}
