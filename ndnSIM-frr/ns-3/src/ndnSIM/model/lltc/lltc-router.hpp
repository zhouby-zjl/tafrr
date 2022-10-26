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

#ifndef SRC_NDNSIM_MODEL_LLTCROUTER_HPP_
#define SRC_NDNSIM_MODEL_LLTCROUTER_HPP_

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/model/lltc/lltc-graph.hpp"
#include "model/ndn-l3-protocol.hpp"

#include "ns3/object.h"
#include "ns3/ptr.h"
#include <unordered_map>

using namespace std;
using namespace ns3;
using namespace ns3::ndn;
using namespace lltc;

namespace lltc {

typedef list<shared_ptr<Name>> LocalPrefixList;

class LltcRouter : public Object {
public:
	LltcRouter(LltcNode* node);
	unordered_map<int, nfd::face::Face*> faceMap; // map from a linkId to its local face
	void addLocalPrefix(shared_ptr<Name> prefix);
	LocalPrefixList* getLocalPrefixList();

	string* toString();
	static TypeId GetTypeId();
	LltcNode* node;

private:
	LocalPrefixList localPrefixList;
};

}

#endif /* SRC_NDNSIM_MODEL_LLTCROUTER_HPP_ */
