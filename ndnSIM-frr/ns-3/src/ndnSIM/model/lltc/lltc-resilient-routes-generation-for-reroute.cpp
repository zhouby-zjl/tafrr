/* *********************************************************************************
This work is licensed under CC BY-NC-SA 4.0
(https://creativecommons.org/licenses/by-nc-sa/4.0/).

Copyright (c) 2022 zhouby-zjl @ github

This file is a part of "Traffic Aware Fast Reroute Mechanism (TA-FRR)"
(https://github.com/zhouby-zjl/drtp/).

Written by zhouby-zjl @ github

This software is protected by the patents as well as the software copyrights.
 ***********************************************************************************/

#include "lltc-resilient-routes-generation-for-reroute.hpp"

#include <iostream>
#include <algorithm>
#include <unordered_set>
#include "lltc-fibonacci-heap-key-funcs.hpp"

using namespace std;
using namespace lltc;

int ResilientRouteGenerationForReroute::_nextPathID = 0;

LltcResilientRouteVectors* ResilientRouteGenerationForReroute::genResilientRoutesVectors(LltcGraph* g,
																						int srcNodeId,
																						LltcConfiguration* config) {
	int n = g->getNNodes();
	LltcResilientRouteVectors* rrv = new LltcResilientRouteVectors(srcNodeId, n);

	double alpha = LltcConfig::LLTC_REROUTE_PATH_GEN_ALPHA;

	bool isNodeIdInQ[n];
	double primPathRelias[n];
	double rerouteLossRates[n];

	for (int i = 0; i < n; ++i) {
		isNodeIdInQ[i] = true;

		rrv->prevNodeIdx[i] = -1;
		rrv->rspsMap[i] = NULL;
		rrv->maxPrimPathDelays[i] = 0.0;
		rrv->maxDelays[i] = 0.0;
		rrv->expectedLossRates[i] = 1.0;
		primPathRelias[i] = 0.0;
		rerouteLossRates[i] = 0.0;
	}

	rrv->expectedLossRates[srcNodeId] = 0.0;
	rrv->maxDelays[srcNodeId] = 0;
	primPathRelias[srcNodeId] = 1.0;
	rerouteLossRates[srcNodeId] = 0.0;

	FHKeyFuncLossRate keyFunc(rrv->expectedLossRates);
	FibonacciHeap q(&keyFunc);

	for (int i = 0; i < n; ++i) {
		q.insert((FHObjectPtr) g->nodes[i]);
	}

	while (q.size() > 0) {
		int uNodeId = *((int*) (q.removeMax()->getNodeObject()));
		LltcNode* uNode = g->getNodeById(uNodeId);
		isNodeIdInQ[uNode->nodeId] = false;

		if (rrv->expectedLossRates[uNode->nodeId] == 1.0) break;

		for (unsigned int i = 0; i < uNode->eLinks->size(); ++i) {
			LltcLink* e = (*(uNode->eLinks))[i];
			LltcNode* vNode = LltcGraph::getLinkOpSide(e, uNode);
			int vNodeId = vNode->nodeId;

			if (!isNodeIdInQ[vNodeId]) continue;

			int primPathLinkDelay = LltcGraph::getLinkDelayTotal(e, uNode, vNode);
			double primPathDelay = rrv->maxPrimPathDelays[uNodeId] + primPathLinkDelay;
			if (primPathDelay > config->maxPathDelay) continue;

			double primPathLinkRelia = LltcGraph::getLinkRelia(e, uNode, vNode, false);
			double primPathRelia = primPathRelias[uNode->nodeId] * primPathLinkRelia;
			vector<int>* retransDstNodeIds = convertPrevArrayToPathNodeIdsArray(rrv->prevNodeIdx,
																				srcNodeId,
																				uNode->nodeId);

			unordered_set<int>* invalidLinkIds = convertPathNodeIdsArrayToLinkIdsSet(g, retransDstNodeIds);
			invalidLinkIds->insert(e->linkId);

			double curRrMaxDelay = rrv->maxDelays[uNodeId] + primPathLinkDelay;
			double maxAllowedDelay = config->maxPathDelay - curRrMaxDelay;
			vector<RedundantSubPath*>* rsps = genRedundantSubPaths(g, vNode->nodeId, retransDstNodeIds,
										invalidLinkIds, config, maxAllowedDelay);

			double reroute_lossRate_overV = 1.0 - (1.0 - rerouteLossRates[uNode->nodeId]) * primPathLinkRelia;

			//double total_lossRate_overV = primPathRelia;
			for (vector<RedundantSubPath*>::iterator iter = rsps->begin(); iter != rsps->end(); ++iter) {
				double rsp_lossRate = (*iter)->oneWayDataDeliveryLossRate;
				int rsp_dstNodeID = (*(*iter)->path->nodeIds)[(*iter)->path->nodeIds->size() - 1];
				reroute_lossRate_overV *= 1.0 - (1.0 - rsp_lossRate) * (1.0 - rerouteLossRates[rsp_dstNodeID]);
				vector<int>* rsp_nodeIds = (*iter)->path->nodeIds;
				int rsp_nodeIds_last = (*rsp_nodeIds) [rsp_nodeIds->size() - 1];
				double reroute_delay = (*iter)->linkDownDelay +  rrv->maxDelays[rsp_nodeIds_last];
				if (reroute_delay > curRrMaxDelay)
					curRrMaxDelay = reroute_delay;
			}

			double overall_lossRate_overV = alpha * (1.0 - primPathRelia) + (1.0 - alpha) * reroute_lossRate_overV;

			if (overall_lossRate_overV < rrv->expectedLossRates[vNode->nodeId]) {
				rrv->maxPrimPathDelays[vNodeId] = primPathDelay;

				//rrv->expectedLossRates[vNodeId] = total_lossRate_overV;
				primPathRelias[vNodeId] = primPathRelia;
				rerouteLossRates[vNodeId] = reroute_lossRate_overV;
				rrv->expectedLossRates[vNodeId] = overall_lossRate_overV;

				q.updateKey((FHObjectPtr) &(vNode->nodeId), (FHObjectPtr) &(vNode->nodeId));

				if (rsps->size() > 0)
					rrv->rspsMap[vNodeId] = rsps;

				rrv->prevNodeIdx[vNodeId] = uNode->nodeId;
				rrv->maxDelays[vNodeId] = curRrMaxDelay;
			}
		}
	}



	return rrv;
}



ResilientRoutes* ResilientRouteGenerationForReroute::constructResilientRoutes(LltcGraph* g,
		LltcResilientRouteVectors* rrv, int dstNodeId,
		LltcConfiguration* config) {
	vector<int>* nodeIds = convertPrevArrayToPathNodeIdsArray(rrv->prevNodeIdx, rrv->srcNodeId, dstNodeId);
	if (nodeIds->size() <= 1) {
		return NULL;
	}
	Path* primaryPath = new Path();
	primaryPath->nodeIds = nodeIds;
	primaryPath->pathID = _nextPathID++;

	vector<int> ppLinksIDs;

	for (uint32_t i = 1; i < nodeIds->size(); ++i) {
		LltcLink* l = g->getLinkByNodeIds((*nodeIds)[i - 1], (*nodeIds)[i]);
		ppLinksIDs.push_back(l->linkId);

	}

	unordered_map<int, vector<RedundantSubPath*>*>* hrsp = new unordered_map<int, vector<RedundantSubPath*>*>();
	for (unsigned int i = 1; i < nodeIds->size(); ++i) {
		int nodeId = (*nodeIds)[i];
		vector<RedundantSubPath*>* rsps = rrv->rspsMap[nodeId];
		if (rsps == NULL) continue;

		vector<int> jointRSPs_indexes;
		int j = 0;
		for (vector<RedundantSubPath*>::iterator iter = rsps->begin(); iter != rsps->end(); ++iter) {
			for (uint32_t i = 1; i < (*iter)->path->nodeIds->size(); ++i) {
				LltcLink* l = g->getLinkByNodeIds((*(*iter)->path->nodeIds)[i - 1], (*(*iter)->path->nodeIds)[i]);
				for (uint32_t k = i; k < nodeIds->size(); ++k) {
					LltcLink* l_prim = g->getLinkByNodeIds((*nodeIds)[i - 1], (*nodeIds)[i]);
					if (l_prim->linkId == l->linkId) {
						jointRSPs_indexes.push_back(j);
						break;
					}
				}
			}
			++j;
		}

		for (int k = jointRSPs_indexes.size() - 1; k >= 0; --k) {
			rsps->erase(rsps->begin() + jointRSPs_indexes[k]);
		}

		for (RedundantSubPath* rsp : *rsps) {
			rsp->path->pathID = _nextPathID++;
		}

		(*hrsp)[nodeId] = rsps;
	}

	ResilientRoutes* rr = new ResilientRoutes(primaryPath, hrsp);
	rr->expectedRrLossRate = rrv->expectedLossRates[dstNodeId];
	rr->maxRrDelay = rrv->maxDelays[dstNodeId];
	rr->primPathDelay = rrv->maxPrimPathDelays[dstNodeId];

	return rr;
}

vector<RedundantSubPath*>* ResilientRouteGenerationForReroute::genRedundantSubPaths(LltcGraph* g, int srcNodeId, vector<int>* dstNodeIds,
				unordered_set<int>* invalidLinkIds, LltcConfiguration* config, double maxAllowedDelay) {
	vector<RedundantSubPath*>* r = new vector<RedundantSubPath*>();
	unordered_set<int> expelledLinkIds;

	for (int i = 0; i < config->beta; ++i) {
		RedundantSubPath* sp = genPath(g, srcNodeId, dstNodeIds, &expelledLinkIds,
					invalidLinkIds, config, maxAllowedDelay);
		if (sp == NULL || isInArray(r, sp)) break;
		r->push_back(sp);

		if (i < config->beta - 1) {
			for (unsigned int j = 1; j < sp->path->nodeIds->size(); ++j) {
				LltcLink* l = g->getLinkByNodeIds((*(sp->path->nodeIds))[j - 1], (*(sp->path->nodeIds))[j]);
				expelledLinkIds.insert(l->linkId);
			}
		}
	}
	return r;
}

RedundantSubPath* ResilientRouteGenerationForReroute::genPath(LltcGraph* g, int srcNodeId, vector<int>* dstNodeIds,
		unordered_set<int>* expelledLinkIds, unordered_set<int>* invalidLinkIds,
		LltcConfiguration* config, double maxAllowedDelay) {
	size_t nNodes = g->getNNodes();
	LltcNode* virDstNode = g->addVirtNode(dstNodeIds);
	int n = nNodes + 1;
	double logNodeRelias[n];
	double _logNodeRelias[n];
	int downDelays[n];
	bool isNodeIdInQ[n];
	int prevNodeIdx[n];

	for (int i = 0; i < n; ++i) {
		logNodeRelias[i] = DOUBLE_NEGATIVE_INFINITY;
		_logNodeRelias[i] = DOUBLE_NEGATIVE_INFINITY;
		isNodeIdInQ[i] = true;
		downDelays[i] = 0;
		prevNodeIdx[i] = -1;
	}

	logNodeRelias[srcNodeId] = 0.0;
	_logNodeRelias[srcNodeId] = 0.0;
	downDelays[srcNodeId] = 0;

	FHKeyFuncNodeRelias keyFunc(logNodeRelias);
	FibonacciHeap q(&keyFunc);

	for (int i = 0; i < n; ++i) {
		q.insert((FHObjectPtr) g->nodes[i]);
	}

	while (q.size() > 0) {
		int uNodeId = *((int*) q.removeMaxObject());

		LltcNode* uNode = g->getNodeById(uNodeId);
		isNodeIdInQ[uNodeId] = false;

		for (LltcLink* e : *uNode->eLinks) {
			if (invalidLinkIds->find(e->linkId) != invalidLinkIds->end() ||
					expelledLinkIds->find(e->linkId) != expelledLinkIds->end()) continue;

			LltcNode* vNode = LltcGraph::getLinkOpSide(e, uNode);
			int vNodeId = vNode->nodeId;
			int* vNodeIdPtr = &(vNode->nodeId);
			if (!isNodeIdInQ[vNodeId]) continue;

			int downDelay = LltcGraph::getLinkDelayTotal(e, vNode, uNode) + downDelays[uNodeId];

			if (downDelays[uNodeId] + downDelay > maxAllowedDelay) 	continue;

			double logLinkRelia = LltcGraph::getLinkRelia(e, uNode, vNode, true) + LltcGraph::getLinkRelia(e, vNode, uNode, true);

			double logReliaOverV = logLinkRelia + logNodeRelias[uNodeId];
			double prevLogReliaOverV = logNodeRelias[vNodeId];
			if (logReliaOverV > prevLogReliaOverV) {
				prevNodeIdx[vNodeId] = uNode->nodeId;

				logNodeRelias[vNodeId] = logReliaOverV;
				q.updateKey((FHObjectPtr) vNodeIdPtr, (FHObjectPtr) vNodeIdPtr);

				_logNodeRelias[vNodeId] = logLinkRelia + _logNodeRelias[uNodeId];
				downDelays[vNodeId] = downDelay;
			}
		}
	}

	if (prevNodeIdx[virDstNode->nodeId] == -1) {
		g->removeVirtNode();
		return NULL;
	}

	int pathNodes = 1;
	int v = virDstNode->nodeId;
	while (v != srcNodeId) {
		v = prevNodeIdx[v];
		++pathNodes;
	}

	Path* path = new Path();
	path->setLength(pathNodes - 1);
	(*path->nodeIds)[0] = srcNodeId;
	v = virDstNode->nodeId;
	int p = pathNodes - 1;
	do {
		if (p < pathNodes - 1) {
			(*path->nodeIds)[p] = v;
		}
		v = prevNodeIdx[v];
		--p;
	} while (v != srcNodeId);

	(*path->nodeIds)[0] = srcNodeId;

	RedundantSubPath* rsp = new RedundantSubPath(path, 0, pathNodes - 2);
	rsp->oneWayDataDeliveryLossRate = 1 - pow(exp(1), _logNodeRelias[virDstNode->nodeId]);
	rsp->linkDownDelay = downDelays[virDstNode->nodeId];

	g->removeVirtNode();

	return rsp;
}

vector<int>* ResilientRouteGenerationForReroute::convertPrevArrayToPathNodeIdsArray(int* prevNodeIdx, int srcNodeId, int uNodeId) {
	int pathNodes = 1;
	int v = uNodeId;
	while (v != srcNodeId) {
		v = prevNodeIdx[v];
		if (v == -1) break;
		++pathNodes;
	}

	vector<int>* path = new vector<int>();
	for (int i = 0; i < pathNodes; ++i) {
		path->push_back(0);
	}

	v = uNodeId;
	int p = pathNodes - 1;
	do {
		(*path)[p] = v;
		v = prevNodeIdx[v];
		--p;
	} while (p >= 0 && v != srcNodeId);

	if (pathNodes >= 2)
		(*path)[0] = srcNodeId;
	return path;
}

unordered_set<int>* ResilientRouteGenerationForReroute::convertPathNodeIdsArrayToLinkIdsSet(LltcGraph* g, vector<int>* retransDstNodeIds) {
	unordered_set<int>* linkIdSet = new unordered_set<int>();
	for (unsigned int i = 1; i < retransDstNodeIds->size(); ++i) {
		int linkId = g->getLinkByNodeIds((*retransDstNodeIds)[i], (*retransDstNodeIds)[i - 1])->linkId;
		linkIdSet->insert(linkId);
	}
	return linkIdSet;
}


void ResilientRouteGenerationForReroute::convertPathNodeIdsArrayToLinkIdsSet(LltcGraph* g,
		vector<int>* retransDstNodeIds, unordered_set<int>* output) {

	for (unsigned int i = 1; i < retransDstNodeIds->size(); ++i) {
		int linkId = g->getLinkByNodeIds((*retransDstNodeIds)[i], (*retransDstNodeIds)[i - 1])->linkId;
		output->insert(linkId);
	}
}

bool ResilientRouteGenerationForReroute::isInArray(vector<RedundantSubPath*>* rsps, RedundantSubPath* rsp) {
	for (RedundantSubPath* _rsp : *rsps) {
		if (_rsp->path->nodeIds->size() != rsp->path->nodeIds->size()) continue;
		bool isTheSame = true;
		for (unsigned int i = 0; i < _rsp->path->nodeIds->size(); ++i) {
			if ((*(_rsp->path->nodeIds))[i] != (*(rsp->path->nodeIds))[i]) {
				isTheSame = false;
				break;
			}
		}
		if (isTheSame) return true;
	}
	return false;
}

Path::Path() {
	nodeIds = new vector<int>();
	pathID = 0;
}

void Path::setLength(int n) {
	nodeIds = new vector<int>();

	for (int i = 0; i < n; ++i) {
		nodeIds->push_back(0);
	}
}

string* Path::toString() {
	stringstream ss;
	ss << pathID << ": " << nodeIds->size() << ", (";
	for (int nodeId : *nodeIds) {
		ss << nodeId << " ";
	}
	ss << ")";
	string* r = new string(ss.str());
	return r;
}

RedundantSubPath::RedundantSubPath(Path* path, int retransSourceNodeIdx, int retransDstNodeIdx) {
	this->path = path;
	this->retransSourceNodeIdx = retransSourceNodeIdx;
	this->retransDstNodeIdx = retransDstNodeIdx;
}

string* RedundantSubPath::toString() {
	stringstream ss;

	ss << path->pathID << ": " << path->nodeIds->size() << ", (";
	for (int i = retransSourceNodeIdx; i <= retransDstNodeIdx; ++i) {
		ss << (*path->nodeIds)[i] << " ";
	}
	ss << "), ";

	ss << "linkDownDelay: " << linkDownDelay << ", ";
	ss << "linkUpDelay: " << linkUpDelay << ", ";
	ss << "dataDeliveryLossRate: " << dataDeliveryLossRate << ", ";
	ss << "timeoutForRetrans: " << retransDelayInMultiTimes;

	string* s = new string(ss.str());
	return s;
}

ResilientRoutes::ResilientRoutes(Path* primaryPath, unordered_map<int, vector<RedundantSubPath*>*>* hrsp) {
	this->primaryPath = primaryPath;
	this->hrsp = hrsp;
	size_t n = primaryPath->nodeIds->size();

	this->waitingTimeoutOnNextPacketInUs = new vector<int>();
	this->waitingTimeoutOnNextPacketForLostReportInUs = new vector<int>();
	this->maxPIATInUsUnderFailover = new vector<int>();
	retransTimeouts = new vector<int>();
	waitingTimeoutOnRetransReportInUs = new unordered_map<int, int>*[n];

	for (size_t i = 0; i < n; ++i) {
		waitingTimeoutOnNextPacketInUs->push_back(0);
		waitingTimeoutOnNextPacketForLostReportInUs->push_back(0);
		maxPIATInUsUnderFailover->push_back(0);
		waitingTimeoutOnRetransReportInUs[i] = new unordered_map<int, int>();
		retransTimeouts->push_back(0);
	}
}

string* ResilientRoutes::toString() {
	stringstream ss;
	ss << "Resilient Routes Dump: \n" << *(primaryPath->toString()) + "\n";

	for (unordered_map<int, vector<RedundantSubPath*>*>::iterator iter = hrsp->begin(); iter != hrsp->end(); ++iter) {
		ss << "--> " << iter->first << ": \n";
		vector<RedundantSubPath*>* rsps = iter->second;
		if (rsps == NULL) continue;
		for (RedundantSubPath* rsp : *rsps) {
			ss << "\t" << *(rsp->toString()) << "\n";
		}
	}

	ss << "RR loss rate: " << expectedRrLossRate << ", maxPathDelay: " << maxRrDelay << "\n";
	string *s = new string(ss.str());
	return s;
}



LltcConfiguration::LltcConfiguration(int numRetransRequests, int beta, double pmuFreq, double maxDriftRatioForPmuFreq,
		int maxConsecutiveDriftPackets, double maxPathDelay) {
	this->numRetransRequests = numRetransRequests;
	this->beta = beta;
	this->pmuFreq = pmuFreq;
	this->maxDriftRatioForPmuFreq = maxDriftRatioForPmuFreq;
	this->maxConsecutiveDriftPackets = maxConsecutiveDriftPackets;
	this->maxPathDelay = maxPathDelay;
}

LltcResilientRouteVectors::LltcResilientRouteVectors(int srcNodeId, int n) {
	this->srcNodeId = srcNodeId;
	this->n = n;
	this->maxDelays = new double[n];
	this->expectedLossRates = new double[n];
	this->prevNodeIdx = new int[n];
	this->rspsMap = new vector<RedundantSubPath*>*[n];
	this->maxPrimPathDelays = new double[n];
}

vector<RedundantSubPath*>* ResilientRoutes::getRSPsByNodeId(int nodeId) {
	if (hrsp->find(nodeId) == hrsp->end()) return NULL;
	return (*hrsp)[nodeId];
}

void ResilientRoutes::setRSPsToNodeId(int nodeId, vector<RedundantSubPath*>* rsp) {
	(*hrsp)[nodeId] = rsp;
}

unordered_map<int, vector<RedundantSubPath*>*>* ResilientRoutes::getHRSP() {
	return hrsp;
}

void ResilientRoutes::removeRSPByNodeId(int nodeId) {
	hrsp->erase(nodeId);
}
