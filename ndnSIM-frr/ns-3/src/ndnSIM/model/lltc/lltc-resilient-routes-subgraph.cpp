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

#include "lltc-resilient-routes-subgraph.hpp"

#include <cmath>

using namespace std;

namespace lltc {

uint32_t LltcRrSubgraph::nextRsgId = 0;

rsg_struct* LltcRrSubgraph::genLltcRrSubgraph(ResilientRoutes* rr, LltcGraph* g) {
	rsg_struct* rsg = new rsg_struct;

	rsg_node* node = new rsg_node;
	node->nodeID = (*rr->primaryPath->nodeIds)[0];
	rsg->nodes.push_back(node);
	rsg_node* node_prev = node;
	size_t nNodesOnPrimPath = rr->primaryPath->nodeIds->size();

	for (size_t i = 1; i < nNodesOnPrimPath; ++i) {
		uint32_t nodeId = (*rr->primaryPath->nodeIds)[i];
		node = new rsg_node;
		node->nodeID = nodeId;
		rsg->nodes.push_back(node);

		rsg_vir_link* virLink = new rsg_vir_link;
		virLink->isOnPrimPath = true;
		virLink->linkID = LltcRrSubgraph::rsgGetLinkID(node_prev->nodeID, nodeId);
		virLink->pathID = rr->primaryPath->pathID;
		virLink->node_ingress = node_prev->nodeID;
		virLink->node_egress = nodeId;
		LltcLink* link = g->getLinkByNodeIds(node_prev->nodeID, nodeId);
		virLink->delay = g->getLinkDelayTotal(link, node_prev->nodeID, nodeId);
		virLink->succRatioInLog = g->getLinkSuccRatio(link, node_prev->nodeID, nodeId, true);

		node_prev->egressVirLinks.push_back(virLink);
		//node->ingressVirLinks.push_back(virLink);
		rsg->links.push_back(virLink);

		rsg->linkStatusMap[virLink->linkID] = RSG_VIR_LINK_STATUS_NORMAL;
		node_prev = node;
	}

	for (size_t i = 1; i < nNodesOnPrimPath; ++i) {
		uint32_t nodeId = (*rr->primaryPath->nodeIds)[i];
		vector<RedundantSubPath*>* rsps = rr->getRSPsByNodeId(nodeId);
		if (rsps == NULL) {
			continue;
		}

		for (RedundantSubPath* rsp : *rsps) {
			rsg_vir_link* virLink = new rsg_vir_link;
			virLink->isOnPrimPath = false;
			virLink->linkID = 0;
			virLink->pathID = rsp->path->pathID;
			virLink->node_ingress = (*rsp->path->nodeIds)[rsp->retransDstNodeIdx];
			virLink->node_egress = (*rsp->path->nodeIds)[rsp->retransSourceNodeIdx];
			virLink->delay = rsp->linkDownDelay;
			virLink->succRatioInLog = log(rsp->oneWayDataDeliveryLossRate);

			rsg->nodes[i]->ingressVirLinks.push_back(virLink);
			rsg_node* node_egress = NULL;
			for (size_t j = 0; j < i; ++j) {
				if (rsg->nodes[j]->nodeID == virLink->node_ingress) {
					node_egress = rsg->nodes[j];
					break;
				}
			}
			node_egress->egressVirLinks.push_back(virLink);
			rsg->links.push_back(virLink);

			rsg_node_id rspNodeID_prev = (*rsp->path->nodeIds)[0];
			for (size_t i = 0; i < rsp->path->nodeIds->size(); ++i) {
				rsg_node_id rspNodeID = (*rsp->path->nodeIds)[i];
				rsg_link_id rspLinkID = LltcRrSubgraph::rsgGetLinkID(rspNodeID_prev, rspNodeID);
				rsg->linkStatusMap[rspLinkID] = RSG_VIR_LINK_STATUS_NORMAL;
			}

			rsg->linkStatusMap[rsp->path->pathID] = RSG_VIR_LINK_STATUS_NORMAL;
		}
	}

	rsg->srcNodeId = (*rr->primaryPath->nodeIds)[0];
	rsg->dstNodeId = (*rr->primaryPath->nodeIds)[rr->primaryPath->nodeIds->size() - 1];
	rsg->primPathID = rr->primaryPath->pathID;

	for (int nodeId : *rr->primaryPath->nodeIds) {
		rsg->rsg_compacted_rr.primPath.nodeIds.push_back((rsg_node_id) nodeId);
	}
	rsg->rsg_compacted_rr.primPath.pathID = rr->primaryPath->pathID;
	rsg_path_compact path_compact;

	for (int nodeId : *rr->primaryPath->nodeIds) {
		vector<RedundantSubPath*>* rsps = rr->getRSPsByNodeId(nodeId);
		if (rsps == NULL) continue;
		vector<rsg_path_compact>& rsps_compact = rsg->rsg_compacted_rr.hrsp[nodeId];

		for (RedundantSubPath* rsp : *rsps) {
			path_compact.pathID = rsp->path->pathID;
			path_compact.nodeIds.clear();
			for (int nodeId : *rsp->path->nodeIds) {
				path_compact.nodeIds.push_back(nodeId);
			}
			rsps_compact.push_back(path_compact);
		}
	}

	rsg->rsgId = nextRsgId++;

	return rsg;
}

rsg_link_id LltcRrSubgraph::rsgGetLinkID(rsg_node_id node_a, rsg_node_id node_b) {
	if (node_a > node_b) {
		rsg_node_id tmp = node_a;
		node_a = node_b;
		node_b = tmp;
	}
	return ((uint64_t) node_a) << 32 | (uint64_t) node_b | RSG_PRIM_PATH_LINK_ID_MARK;
}

void LltcRrSubgraph::rsgUpdateLinkStatus(rsg_struct* rsg, rsg_node_id nodeID_a, rsg_node_id nodeID_b, rsg_phy_link_status status) {
	rsg_link_id vir_link_id = rsgSearchVirLink(&rsg->rsg_compacted_rr.primPath, nodeID_a, nodeID_b, true);
	if (vir_link_id != RSG_VIR_LINK_ID_NULL) {
		rsg->linkStatusMap[vir_link_id] = status;
	}

	for (size_t i = 1; i < rsg->rsg_compacted_rr.primPath.nodeIds.size(); ++i) {
		uint32_t nodeId = rsg->rsg_compacted_rr.primPath.nodeIds[i];
		vector<rsg_path_compact>& rsps = rsg->rsg_compacted_rr.hrsp[nodeId];
		for (rsg_path_compact& rsp : rsps) {
			vir_link_id = rsgSearchVirLink(&rsp, nodeID_a, nodeID_b, false);
			if (vir_link_id == RSG_VIR_LINK_ID_NULL) continue;
			rsg->linkStatusMap[vir_link_id] = status;

			int rspStatus = RSG_VIR_LINK_STATUS_NORMAL;
			rsg_node_id nodeID_prev = rsp.nodeIds[0];
			for (size_t j = 1; j < rsp.nodeIds.size(); ++j) {
				rsg_node_id nodeID = rsp.nodeIds[j];
				rsg_link_id rspLinkID = rsgGetLinkID(nodeID_prev, nodeID);
				if (rsg->linkStatusMap[rspLinkID] == RSG_VIR_LINK_STATUS_FAILURE) {
					rspStatus = RSG_VIR_LINK_STATUS_FAILURE;
					break;
				}
				nodeID_prev = nodeID;
			}
			rsg->linkStatusMap[rsp.pathID] = rspStatus;
		}
	}

}

rsg_link_id LltcRrSubgraph::rsgSearchVirLink(rsg_path_compact* path, rsg_node_id nodeID_phyLinkIngress, rsg_node_id nodeID_phyLinkEgress, bool isPrimPath) {
	for (size_t i = 1; i < path->nodeIds.size(); ++i) {
		rsg_node_id nodeID =  path->nodeIds[i];
		rsg_node_id nodeID_prev = path->nodeIds[i - 1];
		if ((nodeID_prev == nodeID_phyLinkIngress && nodeID == nodeID_phyLinkEgress) ||
				(nodeID == nodeID_phyLinkIngress && nodeID_prev == nodeID_phyLinkEgress)) {
			return rsgGetLinkID(nodeID_prev, nodeID);
		}
		nodeID_prev = nodeID;
	}
	return RSG_VIR_LINK_ID_NULL;
}

preferred_link LltcRrSubgraph::rsgComputePreferredPathID(rsg_struct* rsg, uint32_t localNodeID) {
	rsg_path_node_ids& nodeIds_pp = rsg->rsg_compacted_rr.primPath.nodeIds;
	unordered_map<rsg_node_id, int> nodesToComponentIDs;
	int componentID = 0;
	nodesToComponentIDs[nodeIds_pp[0]] = componentID;
	for (size_t i = 1; i < nodeIds_pp.size(); ++i) {
		rsg_node_id nodeID = nodeIds_pp[i];
		rsg_link_id linkID = LltcRrSubgraph::rsgGetLinkID(nodeIds_pp[i - 1], nodeID);
		if (rsg->linkStatusMap[linkID] == RSG_VIR_LINK_STATUS_FAILURE) {
			componentID++;
		}
		nodesToComponentIDs[nodeID] = componentID;
	}

	int n = rsg->nodes.size();
	RsgFHKeyFunc func;
	FibonacciHeap fh(&func);
	rsg_fibo_node nodes[n];
	unordered_map<rsg_node_id, rsg_prev_node_info> rrNodeInfo;
	unordered_set<rsg_node_id> nodesInFH;
	size_t i = 0;

	for (rsg_node* node : rsg->nodes) {
		nodes[i].nodeId = node->nodeID;
		if (node->nodeID == rsg->srcNodeId) {
			nodes[i].succRatioInLog = 0.0;
		} else {
			nodes[i].succRatioInLog = (-1.0 / 0.0);
		}
		rrNodeInfo[node->nodeID].nodeId = UINT_MAX;
		rrNodeInfo[node->nodeID].virLinkID = 0;
		fh.insert((FHObjectPtr) &(nodes[i]));
		nodesInFH.insert(node->nodeID);
		++i;
	}

	while (fh.size() > 0) {
		rsg_fibo_node* u = (rsg_fibo_node*) fh.removeMax()->getNodeObject();
		nodesInFH.erase(u->nodeId);
		rsg_node* node = rsgGetSubGraphNodeByID(rsg, u->nodeId);
		int u_compID = nodesToComponentIDs[u->nodeId];
		for (rsg_vir_link* egressVirLink : node->egressVirLinks) {
			if (nodesInFH.find(egressVirLink->node_egress) == nodesInFH.end()) {
				continue;
			}
			if (rsg->linkStatusMap[egressVirLink->linkID] == RSG_VIR_LINK_STATUS_FAILURE) {
				continue;
			}
			rsg_fibo_node* v = rsgGetFiboRRNodeByID(egressVirLink->node_egress, nodes, n);
			if (nodesToComponentIDs[v->nodeId] == u_compID && !egressVirLink->isOnPrimPath) {
				continue;  // jump over the unnecessary RSP between the two nodes with the same component IDs.
			}
			double succRate_alt = u->succRatioInLog + egressVirLink->succRatioInLog;
			if (succRate_alt > v->succRatioInLog) {
				v->succRatioInLog = succRate_alt;
				rrNodeInfo[v->nodeId].nodeId = u->nodeId;
				if (egressVirLink->isOnPrimPath) {
					rrNodeInfo[v->nodeId].virLinkID = egressVirLink->linkID;
				} else {
					rrNodeInfo[v->nodeId].virLinkID = egressVirLink->pathID;
				}
				fh.updateKey(v, v);
			}
		}
	}

	rsg_node_id nodeId_prev = rsg->dstNodeId;
	rsg_link_id preferredDownLinkID = UINT64_MAX;
	rsg_link_id preferredUpLinkID = UINT64_MAX;
	while (true) {
		rsg_prev_node_info nodeInfo = rrNodeInfo[nodeId_prev];
		if (nodeInfo.nodeId == localNodeID) {
			preferredDownLinkID = nodeInfo.virLinkID;
			if (rrNodeInfo[nodeInfo.nodeId].nodeId != UINT_MAX) {
				preferredUpLinkID = rrNodeInfo[nodeInfo.nodeId].virLinkID;
			}
			break;
		}
		if (nodeInfo.nodeId == UINT_MAX) break;
		nodeId_prev = nodeInfo.nodeId;
	}

	preferred_link pl;

	if (preferredDownLinkID != UINT64_MAX) {
		if ((RSG_PRIM_PATH_LINK_ID_MARK & preferredDownLinkID) == RSG_PRIM_PATH_LINK_ID_MARK) {
			pl.downLinkID = rsg->primPathID;
		} else {
			pl.downLinkID = preferredDownLinkID;
		}
	} else {
		pl.downLinkID = UINT_MAX;
	}

	if (preferredUpLinkID != UINT64_MAX) {
		if ((RSG_PRIM_PATH_LINK_ID_MARK & preferredUpLinkID) == RSG_PRIM_PATH_LINK_ID_MARK) {
			pl.upLinkID = rsg->primPathID;
		} else {
			pl.upLinkID = preferredUpLinkID;
		}
	} else {
		pl.upLinkID = UINT_MAX;
	}

	return pl;
}

rsg_node* LltcRrSubgraph::rsgGetSubGraphNodeByID(rsg_struct* rsg, rsg_node_id nodeId) {
	for (rsg_node* node : rsg->nodes) {
		if (node->nodeID == nodeId) {
			return node;
		}
	}
	return NULL;
}

rsg_vir_link* LltcRrSubgraph::rsgGetSubGraphVirLinkByNodeIDs(rsg_struct* rsg, rsg_node_id nodeId_a, rsg_node_id nodeId_b) {
	for (rsg_vir_link* link : rsg->links) {
		if (link->node_ingress == nodeId_a && link->node_egress == nodeId_b) {
			return link;
		}
	}
	return NULL;
}

rsg_fibo_node* LltcRrSubgraph::rsgGetFiboRRNodeByID(rsg_node_id nodeId, rsg_fibo_node* nodes, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		rsg_fibo_node* node = &(nodes[i]);
		if (node->nodeId == nodeId) {
			return node;
		}
	}
	return NULL;
}

rsg_vir_link* LltcRrSubgraph::rsgGetSubGraphVirLinkByLinkID(rsg_struct* rsg, rsg_link_id virLinkID) {
	for (rsg_vir_link* link : rsg->links) {
		if (link->linkID == virLinkID) {
			return link;
		}
	}
	return NULL;
}

double RsgFHKeyFunc::PRECISION = 1E10;

long RsgFHKeyFunc::getKey(FHObjectPtr nodeObject) {
	rsg_fibo_node* node = (rsg_fibo_node*) nodeObject;
	return (long) (node->succRatioInLog * PRECISION);
}

void RsgFHKeyFunc::setKey(FHObjectPtr nodeObject, long newKey) {
	rsg_fibo_node* node = (rsg_fibo_node*) nodeObject;
	node->succRatioInLog = (double) newKey / PRECISION;
}

}
