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

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "model/lltc/lltc-graph.hpp"
#include "model/lltc/lltc-resilient-routes-subgraph.hpp"
#include "NFD/daemon/face/face-common.hpp"
#include "model/lltc/lltc-router.hpp"

#include <utility>
#include <unordered_map>

#include "../../model/lltc/lltc-resilient-routes-generation.hpp"
#include "../../model/lltc/lltc-resilient-routes-generation-for-reroute.hpp"
#include "NFD/daemon/fw/lltc-fs.hpp"
#include "NFD/daemon/fw/lltc-common.hpp"

#ifndef SRC_NDNSIM_HELPER_LLTC_LLTCROUTINGHELPER_H_
#define SRC_NDNSIM_HELPER_LLTC_LLTCROUTINGHELPER_H_

using namespace ns3;
using namespace ns3::ndn;
using namespace nfd::fw;
using namespace std;

#define ROUTING_TYPE_REROUTE 	1

namespace lltc {
class LltcResilientRouteVectors;

struct comm_pair {
	int id;
	pair<Ptr<Node>, Ptr<Node>> nodes;
	ResilientRoutes* rr;
};

class LltcRoutingHelper {
public:
	LltcRoutingHelper();
	LltcRoutingHelper(LltcGraph* lltcGraph, LltcConfiguration* config, int routingType);
	void Install(Ptr<Node> node, LltcNode* lltcNode);
	void InstallAll();
	void AddOrigin(const std::string& prefix, Ptr<Node> node);
	void dumpRouterLinkFaceIDs();
	void computeRoutesVectors();
	void computeRoutesForCommPairs();
	void addCommPair(Ptr<Node> producerNode, Ptr<Node> consumerNode);
	list<comm_pair*>* getCommPairs();

	~LltcRoutingHelper();

	ResilientRouteGenerationGeneric* getRRG();
	LltcGraph* getGraph();
	LltcConfiguration* getConfig();

private:
	//typedef pair<Ptr<Node>, Ptr<Node>> comm_pair;
	typedef unordered_map<uint32_t, ForwardingStateForRSG*> fs_map;
	typedef unordered_map<uint32_t, ForwardingStateCollection*> fs_coll_map;

	fs_map* convertToFSes(ResilientRoutes* rr, LocalPrefixList* producerNodePrefixList, rsg_struct* rsg);
	void addFibsFromPath(Path* path, fs_map* fsMap, uint32_t pathId, bool isPrimPath,
							uint32_t primPathId, int timeoutForRetrans, rsg_struct* rsg);

	void enforceFs(fs_coll_map* fsCollMap);
	ForwardingStateForRSG* getRouterFS(fs_map* fs, uint32_t nodeId);
	void dumpFs(fs_map* fs);

	ResilientRouteGenerationGeneric* rrg;
	LltcGraph* lltcGraph;
	LltcConfiguration* config;
	list<comm_pair*> commPairs;
	int lastCommPairID;
	//uint32_t nextPathId;
	unordered_map<int, LltcResilientRouteVectors*> rrvMap;
};

}

#endif /* SRC_NDNSIM_HELPER_LLTC_LLTCROUTINGHELPER_H_ */
