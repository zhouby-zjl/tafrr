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

#ifndef SRC_NDNSIM_MODEL_LLTC_LLTC_RESILIENT_ROUTES_GENERATION_HPP_
#define SRC_NDNSIM_MODEL_LLTC_LLTC_RESILIENT_ROUTES_GENERATION_HPP_



#include <string>
#include <vector>
#include <unordered_map>
#include <climits>
#include <unordered_set>
#include <sstream>
#include <iostream>

#include "lltc-graph.hpp"
#include "ns3/ndnSIM/model/lltc/lltc-config.hpp"


namespace lltc {

class LltcConfiguration;
class Path;
class RedundantSubPath;
class LltcResilientRouteVectors;
class ResilientRoutes;


class LltcConfiguration {
public:
	int numRetransRequests;
	int beta;
	double pmuFreq;
	double maxDriftRatioForPmuFreq;
	int    maxConsecutiveDriftPackets;
	double maxPathDelay;

	LltcConfiguration(int numRetransRequests, int beta, double pmuFreq, double maxDriftRatioForPmuFreq, int maxConsecutiveDriftPackets, double maxPathDelay);
};

class Path {
public:
	vector<int>* nodeIds;
	int pathID = 0;

	Path();
	void setLength(int n);
	string* toString();
};

class RedundantSubPath {
public:
	Path* path;
	int retransSourceNodeIdx;
	int retransDstNodeIdx;
	int linkDownDelay = 0;
	int linkUpDelay = 0;
	double dataDeliveryLossRate = 0.0;
	double oneWayDataDeliveryLossRate = 0.0;
	int retransDelayInMultiTimes = 0;
	int retransDelayInSingleTime = 0;

	RedundantSubPath(Path* path, int retransSourceNodeIdx, int retransDstNodeIdx);
	string* toString();
};


class LltcResilientRouteVectors {
public:
	int srcNodeId;
	int n;
	double* expectedLossRates;
	double* maxDelays;
	double* maxPrimPathDelays;
	int* prevNodeIdx;
	vector<RedundantSubPath*>** rspsMap;

	LltcResilientRouteVectors(int srcNodeId, int n);
};


class ResilientRoutes {
public:
	Path* primaryPath = NULL;
	double primPathDelay = 0.0;

	double expectedRrLossRate = 0.0;
	double maxRrDelay = 0.0;
	//vector<int>* maxCulQueueTimesInUs;
	vector<int>* retransTimeouts;
	//int* maxWaitingTimeoutOnRetransReportInUs;
	vector<int>* waitingTimeoutOnNextPacketInUs;
	vector<int>* waitingTimeoutOnNextPacketForLostReportInUs;
	vector<int>* maxPIATInUsUnderFailover;
	unordered_map<int, int>** waitingTimeoutOnRetransReportInUs;

	ResilientRoutes(Path* primaryPath, unordered_map<int, vector<RedundantSubPath*>*>* hrsp);
	vector<RedundantSubPath*>* getRSPsByNodeId(int nodeId);
	void setRSPsToNodeId(int nodeId, vector<RedundantSubPath*>* rsp);
	unordered_map<int, vector<RedundantSubPath*>*>* getHRSP();
	void removeRSPByNodeId(int nodeId);
	string* toString();

private:
	unordered_map<int, vector<RedundantSubPath*>*>* hrsp = NULL;
};


class ResilientRouteGenerationGeneric {
public:
	ResilientRouteGenerationGeneric();
	~ResilientRouteGenerationGeneric();

	virtual LltcResilientRouteVectors* genResilientRoutesVectors(LltcGraph* g, int srcNodeId, LltcConfiguration* config);
	virtual ResilientRoutes* constructResilientRoutes(LltcGraph* g, LltcResilientRouteVectors* rrv, int dstNodeId,
																LltcConfiguration* config);
};


}


#endif /* SRC_NDNSIM_MODEL_LLTC_LLTC_RESILIENT_ROUTES_GENERATION_HPP_ */
