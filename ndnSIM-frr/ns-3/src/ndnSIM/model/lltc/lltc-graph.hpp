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

#ifndef LLTC_GRAPH_HPP_
#define LLTC_GRAPH_HPP_

#include <vector>
#include <climits>
#include <cmath>
#include <algorithm>
#include <string>

using namespace std;

namespace lltc {

class LltcGraph;
class LltcNode;
class LltcLink;

class LltcGraph {
public:
	LltcNode** nodes;
	vector<LltcLink*> *links;
	int maxLinkId = 0;
	int maxVirtLinkId = 0;

	//below is for testing
	double firstPathRelia = 0;
	int minDelay = INT_MAX;
	int maxDelay = -1;
	int nodeIdx = 0;
	double minLinkRelia = 0.0;
	double minLinkReliaInLog = 0.0;

	LltcGraph(int meanDelay_process, int maxDelay_queue, int delay_trans);
	bool loadFromIEEEDataFile(string* filePath, bool isRandom, double reliaHigh, double reliaLow);
	bool loadFromCSVDataFile(string* filePath, bool isRandom, double reliaHigh, double reliaLow);
	bool loadFromBriteDataFile(string* filePath, bool isRandom, double reliaHigh, double reliaLow);
	LltcLink* addLinkDual(LltcNode* a, LltcNode* b, double abRelia, double baRelia,
										int abDelay_total, int baDelay_total,
										int abDelay_trans, int baDelay_trans,
										int abDelay_process, int baDelay_process,
										int abDelay_queue, int baDelay_queue);
	LltcLink* addVirtLinkDual(LltcNode* a, LltcNode* b);
	void removeLinkDual(int linkId);
	void removeVirtNode();
	LltcNode* addVirtNode(vector<int>* neighborNodeIds);
	LltcLink* getLink(LltcNode* a, LltcNode* b);
	LltcLink* getLinkByNodeIds(int nodeIdA, int nodeIdB);
	LltcNode* getNodeById(int id);
	double getMinLinkRelia(bool inLog);
	LltcLink* findLinkById(int linkId);
	size_t getNNodes();

	static LltcNode* getLinkOpSide(LltcLink* link, LltcNode* c);
	static int getLinkDelayTotal(LltcLink* link, LltcNode *nodeA, LltcNode *nodeB);
	static int getLinkDelayTotal(LltcLink* link, int nodeAId, int nodeBId);
	static int getLinkDelayTrans(LltcLink* link, LltcNode *nodeA, LltcNode *nodeB);
	static int getLinkDelayTrans(LltcLink* link, int nodeAId, int nodeBId);
	static int getLinkDelayQueue(LltcLink* link, LltcNode *nodeA, LltcNode *nodeB);
	static int getLinkDelayProcess(LltcLink* link, LltcNode *nodeA, LltcNode *nodeB);
	static int getLinkDelayQueue(LltcLink* link, int nodeAId, int nodeBId);
	static int getLinkDelayProcess(LltcLink* link, int nodeAId, int nodeBId);
	static double getLinkRelia(LltcLink* link, LltcNode *nodeA, LltcNode *nodeB, bool inLog);
	static double getLinkSuccRatio(LltcLink* link, int nodeAId, int nodeBId, bool inLog);

private:
	inline vector<LltcLink*>::iterator findIdx(vector<LltcLink*>* vec, LltcLink* l);
	inline vector<LltcNode*>::iterator findIdx(vector<LltcNode*>* vec, LltcNode* l);

	LltcNode* createNode(int nodeId);
	LltcLink* createLink(int id, double abRelia, double baRelia,
								int abDelay_total, int baDelay_total,
								int abDelay_trans, int baDelay_trans,
								int abDelay_process, int baDelay_process,
								int abDelay_queue, int baDelay_queue);

	int nNodes;
	LltcNode* virDstNode;

	int maxDelay_process;
	int maxDelay_queue;
	int delay_trans;
};


struct LltcNode {
	int nodeId;
	vector<LltcLink*> *eLinks, *iLinks;
};


struct LltcLink {
	int linkId;
	LltcNode *nodeA, *nodeB;

	int abDelay_total;      // in microseconds
	int baDelay_total;      // in microseconds
	int abDelay_trans;      // in microseconds
	int baDelay_trans;      // in microseconds
	int abDelay_queue;      // in microseconds
	int baDelay_queue;      // in microseconds
	int abDelay_process;      // in microseconds
	int baDelay_process;      // in microseconds

	double abRelia;   // in percentage
	double abReliaInLog;
	double baRelia;   // in percentage
	double baReliaInLog;
};

}

#endif /* LLTC_GRAPH_HPP_ */
