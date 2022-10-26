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

#ifndef SRC_NDNSIM_MODEL_LLTC_LLTC_RESILIENT_ROUTES_SUBGRAPH_HPP_
#define SRC_NDNSIM_MODEL_LLTC_LLTC_RESILIENT_ROUTES_SUBGRAPH_HPP_

#include <vector>
#include "lltc-fibonacci-heap.hpp"
#include "lltc-resilient-routes-generation.hpp"

using namespace std;

namespace lltc {

typedef uint32_t rsg_node_id;
typedef uint64_t rsg_link_id;
typedef uint8_t rsg_vir_link_status;
typedef uint8_t rsg_phy_link_status;
typedef vector<rsg_node_id> rsg_path_node_ids;

#define RSG_PRIM_PATH_LINK_ID_MARK (0x1UL << 63)

struct rsg_vir_link {
	bool isOnPrimPath;
	rsg_link_id linkID;   // if isOnPrimPath is true
	uint32_t pathID;
	rsg_node_id node_ingress;
	rsg_node_id node_egress;
	double succRatioInLog;
	double delay;
};

struct rsg_node {
	rsg_node_id nodeID;
	vector<rsg_vir_link*> egressVirLinks;
	vector<rsg_vir_link*> ingressVirLinks;
};

struct rsg_path_compact {
	rsg_path_node_ids nodeIds;
	uint32_t pathID;
};

struct rsg_compacted_rr_struct {
	rsg_path_compact primPath;
	unordered_map<rsg_node_id, vector<rsg_path_compact>> hrsp;
};


struct rsg_struct {
	uint32_t rsgId;
	vector<rsg_node*> nodes;
	vector<rsg_vir_link*> links;
	unordered_map<rsg_link_id, rsg_vir_link_status> linkStatusMap;
	rsg_node_id srcNodeId;
	rsg_node_id dstNodeId;
	uint32_t primPathID;

	rsg_compacted_rr_struct rsg_compacted_rr;
};

struct rsg_fibo_node {
	rsg_node_id nodeId;
	double succRatioInLog;
};

struct rsg_prev_node_info {
	rsg_node_id nodeId;
	rsg_link_id virLinkID;
};

struct preferred_link {
	rsg_link_id downLinkID;
	rsg_link_id upLinkID;
};

#define RSG_VIR_LINK_STATUS_NORMAL 0
#define RSG_VIR_LINK_STATUS_FAILURE 1
#define RSG_PHY_LINK_STATUS_NORMAL 0
#define RSG_PHY_LINK_STATUS_FAILURE 1

#define RSG_VIR_LINK_ID_NULL UINT64_MAX

class LltcRrSubgraph {
public:
	static rsg_struct* genLltcRrSubgraph(ResilientRoutes* rr, LltcGraph* g);

	static void rsgUpdateLinkStatus(rsg_struct* rsg, rsg_node_id nodeID_ingress, rsg_node_id nodeID_egress, rsg_phy_link_status status);
	static preferred_link rsgComputePreferredPathID(rsg_struct* rsg, uint32_t localNodeID);

private:
	static rsg_link_id rsgGetLinkID(rsg_node_id node_ingress, rsg_node_id node_egress);
	static rsg_link_id rsgSearchVirLink(rsg_path_compact* path, rsg_node_id nodeID_ingress, rsg_node_id nodeID_egress, bool isPrimPath);
	static rsg_node* rsgGetSubGraphNodeByID(rsg_struct* rsg, rsg_node_id nodeId);
	static rsg_vir_link* rsgGetSubGraphVirLinkByNodeIDs(rsg_struct* rsg, rsg_node_id nodeId_a, rsg_node_id nodeId_b);
	static rsg_fibo_node* rsgGetFiboRRNodeByID(rsg_node_id nodeId, rsg_fibo_node* nodes, size_t n);
	static rsg_vir_link* rsgGetSubGraphVirLinkByLinkID(rsg_struct* rsg, rsg_link_id virLinkID);

	static uint32_t nextRsgId;
};

class RsgFHKeyFunc : public FibonacciHeapKeyFunc {
public:
	static double PRECISION;

	long getKey(FHObjectPtr nodeObject);
	void setKey(FHObjectPtr nodeObject, long newKey);
};


}

#endif /* SRC_NDNSIM_MODEL_LLTC_LLTC_RESILIENT_ROUTES_SUBGRAPH_HPP_ */
