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

#include "lltc-graph.hpp"

#include <iostream>
#include <fstream>
#include <regex>
#include <stdlib.h>
#include <unordered_map>
#include <cmath>

using namespace std;
using namespace __gnu_cxx;

using namespace lltc;

LltcGraph::LltcGraph(int meanDelay_process, int maxDelay_queue, int delay_trans) {
	links = new vector<LltcLink*>();
	nodes = NULL;
	virDstNode = NULL;
	nNodes = 0;

	this->maxDelay_process = meanDelay_process;
	this->maxDelay_queue = maxDelay_queue;
	this->delay_trans = delay_trans;
}

string trimstr(string s){
  size_t n = s.find_last_not_of(" \r\n\t");
    if (n != string::npos){
        s.erase(n + 1, s.size() - n);
    }
    n = s.find_first_not_of(" \r\n\t");
    if (n != string::npos){
        s.erase(0, n);
    }
    return s;
}

size_t LltcGraph::getNNodes() {
	return nNodes;
}

LltcNode* LltcGraph::createNode(int nodeId) {
	LltcNode* node = new LltcNode;
	node->nodeId = nodeId;
	node->eLinks = new vector<LltcLink*>();
	node->iLinks = new vector<LltcLink*>();
	return node;
}


bool LltcGraph::loadFromIEEEDataFile(string* filePath, bool isRandom, double reliaHigh, double reliaLow) {
	ifstream ifs;
	ifs.open(*filePath);
	if (!ifs.is_open()) {
		cerr << "open file err" << endl;
		return false;
	}

	string line;
	int state = 0;
	regex busPattern("(\\d+)\\s+.+");
	regex branchPattern("(\\d+)\\s+(\\d+)\\s+\\d+\\s+\\d+\\s+\\d+\\s+\\d+\\s+([\\d\\.])+\\s+.+");
	smatch m;
	unordered_map<int, int> nodeIdToIdx;
	nodeIdx = 0;
	srand((int) time(0));
	vector<LltcNode*> _nodes;

	while (getline(ifs, line)) {
		line = trimstr(line);
		if (line.size() == 0) {
			continue;
		}

		if (state < 2) {
			state++;
			continue;
		}

		if (state == 2) {
			if (regex_match(line, m, busPattern)) {
				LltcNode* node = createNode(nodeIdx);
				_nodes.push_back(node);

				nodeIdToIdx[atoi(m[1].str().c_str())] = nodeIdx;
				nodeIdx++;
			} else {
				state = 3;
				continue;
			}
		} else if (state == 3) {
			nNodes = _nodes.size();
			nodes = new LltcNode*[nNodes + 1];
			for (int i = 0; i < nNodes; ++i) {
				nodes[i] = _nodes[i];
			}

			state++;
		} else if (state == 4) {
			if (regex_match(line, m, branchPattern)) {
				int x = nodeIdToIdx[atoi(m[1].str().c_str())];
				int y = nodeIdToIdx[atoi(m[2].str().c_str())];
				//double resistance = atof(m[3].str().c_str());
				//int delay = (int)(alpha * resistance);

				double reliaA, reliaB;
				if (isRandom) {
					reliaA = ((double) rand() / double(RAND_MAX)) * (reliaHigh - reliaLow) + reliaLow;
					reliaB = ((double) rand() / double(RAND_MAX)) * (reliaHigh - reliaLow) + reliaLow;
				} else {
					reliaA = reliaHigh;
					reliaB = reliaHigh;
				}

				int delay_total = delay_trans + maxDelay_process + maxDelay_queue;

				if (getLinkByNodeIds(x, y) == NULL) {
					LltcLink* l = addLinkDual(_nodes[y], _nodes[x], reliaA, reliaB, delay_total, delay_total,
							delay_trans, delay_trans, maxDelay_process, maxDelay_process,
							maxDelay_queue, maxDelay_queue);

					this->minLinkRelia = min(min(reliaA, reliaB), minLinkRelia);
					this->minLinkReliaInLog = min(min(l->abReliaInLog, l->baReliaInLog), minLinkReliaInLog);
				}
			} else {
				state = 5;
				continue;
			}
		}
	}

	ifs.close();

	maxVirtLinkId = maxLinkId;

	virDstNode = createNode(nodeIdx);
	nodes[nNodes] = virDstNode;

	return true;
}

bool LltcGraph::loadFromBriteDataFile(string* filePath, bool isRandom, double reliaHigh, double reliaLow) {
	ifstream ifs;
	ifs.open(*filePath);
	if (!ifs.is_open()) {
		cerr << "open file err" << endl;
		return false;
	}

	string line;
	int state = 0;
	regex busPattern("(\\d+)\\s+.+");
	regex branchPattern("(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+.+");

	smatch m;
	unordered_map<int, int> nodeIdToIdx;
	nodeIdx = 0;
	srand((int) time(0));
	vector<LltcNode*> _nodes;

	while (getline(ifs, line)) {
		line = trimstr(line);
		if (line.size() == 0) {
			continue;
		}

		if (state < 3) {
			state++;
			continue;
		}

		if (state == 3) {
			if (regex_match(line, m, busPattern)) {
				LltcNode* node = createNode(nodeIdx);
				_nodes.push_back(node);

				nodeIdToIdx[atoi(m[1].str().c_str())] = nodeIdx;
				nodeIdx++;
			} else {
				nNodes = _nodes.size();
				nodes = new LltcNode*[nNodes + 1];
				for (int i = 0; i < nNodes; ++i) {
					nodes[i] = _nodes[i];
				}

				state = 4;
				continue;
			}
		} else if (state == 4) {
			if (regex_match(line, m, branchPattern)) {
				int x = nodeIdToIdx[atoi(m[2].str().c_str())];
				int y = nodeIdToIdx[atoi(m[3].str().c_str())];
				//double resistance = atof(m[3].str().c_str());
				//int delay = (int)(alpha * resistance);

				double reliaA, reliaB;
				if (isRandom) {
					reliaA = ((double) rand() / double(RAND_MAX)) * (reliaHigh - reliaLow) + reliaLow;
					reliaB = ((double) rand() / double(RAND_MAX)) * (reliaHigh - reliaLow) + reliaLow;
				} else {
					reliaA = reliaHigh;
					reliaB = reliaHigh;
				}

				int delay_total = delay_trans + maxDelay_process + maxDelay_queue;

				if (getLinkByNodeIds(x, y) == NULL) {
					LltcLink* l = addLinkDual(_nodes[y], _nodes[x], reliaA, reliaB, delay_total, delay_total,
							delay_trans, delay_trans, maxDelay_process, maxDelay_process,
							maxDelay_queue, maxDelay_queue);

					this->minLinkRelia = min(min(reliaA, reliaB), minLinkRelia);
					this->minLinkReliaInLog = min(min(l->abReliaInLog, l->baReliaInLog), minLinkReliaInLog);
				}
			} else {
				state = 5;
				continue;
			}
		}
	}

	ifs.close();

	maxVirtLinkId = maxLinkId;

	virDstNode = createNode(nodeIdx);
	nodes[nNodes] = virDstNode;

	return true;
}

bool LltcGraph::loadFromCSVDataFile(string* filePath, bool isRandom, double reliaHigh, double reliaLow) {
	ifstream ifs;
	ifs.open(*filePath);
	if (!ifs.is_open()) {
		cerr << "open file err" << endl;
		return false;
	}

	string line;
	int state = 0;
	regex branchPattern("(\\d+),(\\d+)");
	smatch m;
	unordered_map<int, int> nodeIdToIdx;
	nodeIdx = 0;
	srand((int) time(0));

	while (getline(ifs, line)) {
		line = trimstr(line);
		if (line.size() == 0) {
			continue;
		}

		if (state == 0) {
			nNodes = atoi(line.c_str());
			state = 1;

			nodes = new LltcNode*[nNodes + 1];
			for (int i = 0; i < nNodes; ++i) {
				LltcNode* node = createNode(nodeIdx);
				nodes[i] = node;

				nodeIdToIdx[i] = nodeIdx;
				nodeIdx++;
			}
		} else if (state == 1) {
			if (regex_match(line, m, branchPattern)) {
				int x = nodeIdToIdx[atoi(m[1].str().c_str())];
				int y = nodeIdToIdx[atoi(m[2].str().c_str())];
				//double resistance = atof(m[3].str().c_str());
				//int delay = (int)(alpha * resistance);

				double reliaA, reliaB;
				if (isRandom) {
					reliaA = ((double) rand() / double(RAND_MAX)) * (reliaHigh - reliaLow) + reliaLow;
					reliaB = ((double) rand() / double(RAND_MAX)) * (reliaHigh - reliaLow) + reliaLow;
				} else {
					reliaA = reliaHigh;
					reliaB = reliaHigh;
				}

				int delay_total = delay_trans + maxDelay_process + maxDelay_queue;

				if (getLinkByNodeIds(x, y) == NULL) {
					LltcLink* l = addLinkDual(nodes[y], nodes[x], reliaA, reliaB, delay_total, delay_total,
							delay_trans, delay_trans, maxDelay_process, maxDelay_process,
							maxDelay_queue, maxDelay_queue);

					this->minLinkRelia = min(min(reliaA, reliaB), minLinkRelia);
					this->minLinkReliaInLog = min(min(l->abReliaInLog, l->baReliaInLog), minLinkReliaInLog);
				}
			}
		}
	}

	ifs.close();

	maxVirtLinkId = maxLinkId;

	virDstNode = createNode(nodeIdx);
	nodes[nNodes] = virDstNode;

	return true;
}

LltcLink* LltcGraph::addLinkDual(LltcNode* a, LltcNode* b, double abRelia, double baRelia,
												 int abDelay_total, int baDelay_total,
												 int abDelay_trans, int baDelay_trans,
												 int abDelay_process, int baDelay_process,
												 int abDelay_queue, int baDelay_queue) {
	LltcLink* link = createLink(maxLinkId, abRelia, abRelia, abDelay_total, baDelay_total, abDelay_trans, baDelay_trans,
											abDelay_process, baDelay_process, abDelay_queue, baDelay_queue);

	link->nodeA = a;
	link->nodeB = b;

	a->eLinks->push_back(link);
	b->iLinks->push_back(link);

	b->eLinks->push_back(link);
	a->iLinks->push_back(link);

	links->push_back(link);

	maxLinkId++;
	return link;
}

LltcLink* LltcGraph::addVirtLinkDual(LltcNode* a, LltcNode* b) {
	LltcLink* link = createLink(maxVirtLinkId, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 0, 0);
	link->nodeA = a;
	link->nodeB = b;

	a->eLinks->push_back(link);
	b->iLinks->push_back(link);

	b->eLinks->push_back(link);
	a->iLinks->push_back(link);

	maxVirtLinkId++;
	return link;
}

void LltcGraph::removeLinkDual(int linkId) {
	LltcLink* l = (*links)[linkId];
	LltcNode* nodeA = l->nodeA;
	LltcNode* nodeB = l->nodeB;

	nodeA->eLinks->erase(findIdx(nodeA->eLinks, l));
	nodeA->iLinks->erase(findIdx(nodeA->iLinks, l));
	nodeB->eLinks->erase(findIdx(nodeB->eLinks, l));
	nodeB->iLinks->erase(findIdx(nodeB->iLinks, l));
}

void LltcGraph::removeVirtNode() {
	for (LltcLink* l : (*(virDstNode->eLinks))) {
		LltcNode* nodeB = getLinkOpSide(l, virDstNode);
		nodeB->iLinks->erase(findIdx(nodeB->iLinks, l));
	}

	for (LltcLink* l : (*(virDstNode->iLinks))) {
		LltcNode* nodeB = getLinkOpSide(l, virDstNode);
		nodeB->eLinks->erase(findIdx(nodeB->eLinks, l));
	}

	virDstNode->iLinks->clear();
	virDstNode->eLinks->clear();

	maxVirtLinkId = maxLinkId;
}

LltcNode* LltcGraph::addVirtNode(vector<int>* neighborNodeIds) {
	for (unsigned int i = 0; i < neighborNodeIds->size(); ++i) {
		int neighborNodeId = (*(neighborNodeIds))[i];
		LltcNode* nodeA = nodes[neighborNodeId];
		addVirtLinkDual(nodeA, virDstNode);
	}

	return virDstNode;
}

LltcLink* LltcGraph::getLink(LltcNode* a, LltcNode* b) {
	int nodeBId = b->nodeId;
	for (LltcLink* e : *a->eLinks) {
		if (e->nodeB->nodeId == nodeBId || e->nodeA->nodeId == nodeBId) {
			return e;
		}
	}
	return NULL;
}

LltcLink* LltcGraph::getLinkByNodeIds(int nodeIdA, int nodeIdB) {
	LltcNode* nodeA = nodes[nodeIdA];
	for (LltcLink* e : *nodeA->eLinks) {
		if (e->nodeB->nodeId == nodeIdB || e->nodeA->nodeId == nodeIdB) {
			return e;
		}
	}
	return NULL;
}

LltcNode* LltcGraph::getNodeById(int id) {
	return nodes[id];
}

double LltcGraph::getMinLinkRelia(bool inLog) {
	double minRelia = 1.0;
	double relia;
	for (LltcLink* l : *links) {
		if (inLog) {
			relia = min(l->abReliaInLog, l->baReliaInLog);
		} else {
			relia = min(l->abRelia, l->baRelia);
		}
		minRelia = min(relia, minRelia);
	}
	return minRelia;
}

LltcLink* LltcGraph::findLinkById(int linkId) {
	for (LltcLink* l : *links) {
		if (l->linkId == linkId) return l;
	}
	return NULL;
}


inline vector<LltcLink*>::iterator LltcGraph::findIdx(vector<LltcLink*>* vec, LltcLink* l) {
	vector<LltcLink*>::iterator iter = find(vec->begin(), vec->end(), l);
	return iter;
}

inline vector<LltcNode*>::iterator LltcGraph::findIdx(vector<LltcNode*>* vec, LltcNode* l) {
	vector<LltcNode*>::iterator iter = find(vec->begin(), vec->end(), l);
	return iter;
}

LltcLink* LltcGraph::createLink(int id, double abRelia, double baRelia,
										int abDelay_total, int baDelay_total,
										int abDelay_trans, int baDelay_trans,
										int abDelay_process, int baDelay_process,
										int abDelay_queue, int baDelay_queue) {
	LltcLink* link = new LltcLink;
	link->linkId = id;
	link->abRelia = abRelia;
	link->baRelia = baRelia;
	link->abReliaInLog = log(abRelia);
	link->baReliaInLog = log(baRelia);

	link->abDelay_total = abDelay_total;
	link->baDelay_total = baDelay_total;
	link->abDelay_trans = abDelay_trans;
	link->baDelay_trans = baDelay_trans;
	link->abDelay_process = abDelay_process;
	link->baDelay_process = baDelay_process;
	link->abDelay_queue = abDelay_queue;
	link->baDelay_queue = baDelay_queue;

	link->nodeA = NULL;
	link->nodeB = NULL;

	return link;
}

LltcNode* LltcGraph::getLinkOpSide(LltcLink* link, LltcNode* c) {
	if (c->nodeId == link->nodeA->nodeId) return link->nodeB;
	else if (c->nodeId == link->nodeB->nodeId) return link->nodeA;
	else return NULL;
}

int LltcGraph::getLinkDelayTotal(LltcLink* link, LltcNode *nodeA, LltcNode *nodeB) {
	if (link->nodeA->nodeId == nodeA->nodeId && link->nodeB->nodeId == nodeB->nodeId) {
		return link->abDelay_total;
	} else if (link->nodeB->nodeId == nodeA->nodeId && link->nodeA->nodeId == nodeB->nodeId) {
		return link->baDelay_total;
	} else {
		return -1;
	}
}

int LltcGraph::getLinkDelayTrans(LltcLink* link, LltcNode *nodeA, LltcNode *nodeB) {
	if (link->nodeA->nodeId == nodeA->nodeId && link->nodeB->nodeId == nodeB->nodeId) {
		return link->abDelay_trans;
	} else if (link->nodeB->nodeId == nodeA->nodeId && link->nodeA->nodeId == nodeB->nodeId) {
		return link->baDelay_trans;
	} else {
		return -1;
	}
}

int LltcGraph::getLinkDelayQueue(LltcLink* link, LltcNode *nodeA, LltcNode *nodeB) {
	if (link->nodeA->nodeId == nodeA->nodeId && link->nodeB->nodeId == nodeB->nodeId) {
		return link->abDelay_queue;
	} else if (link->nodeB->nodeId == nodeA->nodeId && link->nodeA->nodeId == nodeB->nodeId) {
		return link->baDelay_queue;
	} else {
		return -1;
	}
}

int LltcGraph::getLinkDelayProcess(LltcLink* link, LltcNode *nodeA, LltcNode *nodeB) {
	if (link->nodeA->nodeId == nodeA->nodeId && link->nodeB->nodeId == nodeB->nodeId) {
		return link->abDelay_process;
	} else if (link->nodeB->nodeId == nodeA->nodeId && link->nodeA->nodeId == nodeB->nodeId) {
		return link->baDelay_process;
	} else {
		return -1;
	}
}

int LltcGraph::getLinkDelayTotal(LltcLink* link, int nodeAId, int nodeBId) {
	if (link->nodeA->nodeId == nodeAId && link->nodeB->nodeId == nodeBId) {
		return link->abDelay_total;
	} else if (link->nodeB->nodeId == nodeAId && link->nodeA->nodeId == nodeBId) {
		return link->baDelay_total;
	} else {
		return -1;
	}
}

int LltcGraph::getLinkDelayTrans(LltcLink* link, int nodeAId, int nodeBId) {
	if (link->nodeA->nodeId == nodeAId && link->nodeB->nodeId == nodeBId) {
		return link->abDelay_trans;
	} else if (link->nodeB->nodeId == nodeAId && link->nodeA->nodeId == nodeBId) {
		return link->baDelay_trans;
	} else {
		return -1;
	}
}

int LltcGraph::getLinkDelayQueue(LltcLink* link, int nodeAId, int nodeBId) {
	if (link->nodeA->nodeId == nodeAId && link->nodeB->nodeId == nodeBId) {
		return link->abDelay_queue;
	} else if (link->nodeB->nodeId == nodeAId && link->nodeA->nodeId == nodeBId) {
		return link->baDelay_queue;
	} else {
		return -1;
	}
}

int LltcGraph::getLinkDelayProcess(LltcLink* link, int nodeAId, int nodeBId) {
	if (link->nodeA->nodeId == nodeAId && link->nodeB->nodeId == nodeBId) {
		return link->abDelay_process;
	} else if (link->nodeB->nodeId == nodeAId && link->nodeA->nodeId == nodeBId) {
		return link->baDelay_process;
	} else {
		return -1;
	}
}

double LltcGraph::getLinkRelia(LltcLink* link, LltcNode *nodeA, LltcNode *nodeB, bool inLog) {
	if (link->nodeA->nodeId == nodeA->nodeId && link->nodeB->nodeId == nodeB->nodeId) {
		return inLog ? link->abReliaInLog : link->abRelia;
	} else if (link->nodeB->nodeId == nodeA->nodeId && link->nodeA->nodeId == nodeB->nodeId) {
		return inLog ? link->baReliaInLog : link->baRelia;
	} else {
		return -1.0;
	}
}

double LltcGraph::getLinkSuccRatio(LltcLink* link, int nodeAId, int nodeBId, bool inLog) {
	if (link->nodeA->nodeId == nodeAId && link->nodeB->nodeId == nodeBId) {
		return inLog ? link->abReliaInLog : link->abRelia;
	} else if (link->nodeB->nodeId == nodeAId && link->nodeA->nodeId == nodeBId) {
		return inLog ? link->baReliaInLog : link->baRelia;
	} else {
		return -1.0;
	}
}

