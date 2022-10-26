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

#include "lltc-router.hpp"

#include "model/ndn-l3-protocol.hpp"
#include <sstream>

using namespace std;
using namespace ns3;
using namespace ns3::ndn;
using namespace lltc;

namespace lltc {

LltcRouter::LltcRouter(LltcNode* node) {
	this->node = node;
}

string* LltcRouter::toString() {
	stringstream ss;
	ss << "Router with node ID: " << node->nodeId << endl;
	for (unordered_map<int, nfd::face::Face*>::iterator iter = faceMap.begin(); iter != faceMap.end(); ++iter) {
		ss << "--> link ID (LltcGraph): " << iter->first << ", face ID (ns3::Node): " << iter->second->getId() << endl;
	}
	string* s = new string(ss.str());
	return s;
}

TypeId LltcRouter::GetTypeId() {
	static TypeId tid = TypeId("lltc::LltcRouter").SetGroupName("lltc").SetParent<Object>();
	return tid;
}

void LltcRouter::addLocalPrefix(shared_ptr<Name> prefix) {
	this->localPrefixList.push_back(prefix);
}

LocalPrefixList* LltcRouter::getLocalPrefixList() {
	return &localPrefixList;
}

}
