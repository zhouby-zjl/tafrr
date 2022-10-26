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

#ifndef SRC_NDNSIM_NFD_DAEMON_FW_LLTC_MESSAGES_HELPER_HPP_
#define SRC_NDNSIM_NFD_DAEMON_FW_LLTC_MESSAGES_HELPER_HPP_

#include "algorithm.hpp"
#include "common/global.hpp"
#include "common/logger.hpp"
#include "ns3/ndnSIM/model/ndn-common.hpp"

#include <list>
#include "lltc-common.hpp"

using namespace std;


namespace nfd {
namespace fw {

class LltcMessagesHelper {
public:
	static void extractLsa(const Data& data, list<LinkState>* lsList);

	static CapsuleUri parseCapsuleUri(const Name& prefixName, const Name& dataName);
	static ChangePathUri parseChangePathUri(const Name& prefixName, const Name& dataName);
	static CapsuleOnRSPUri parseCapsuleOnRSPUri(const Name& prefixName, const Name& dataName);
	static EchoUri parseEchoUri(const Name& lltcPrefixName, const Name& dataName);
	static BackUri parseBackUri(const Name& lltcPrefixName, const Name& dataName);
	static LsaUri parseLsaUri(const Name& lltcPrefixName, const Name& dataName);

	static shared_ptr<Data> constructCapsule(string pitPrefixStr, LltcDataID dataID, const Data& origData,
													bool isRetrans, LltcDataID origDataID);
	static shared_ptr<Data> constructCapsuleOnRSP(string pitPrefixStr, LltcDataID dataID, const Data& origData,
														bool isRetrans, LltcDataID origDataID);

	static shared_ptr<Data> constructEcho(string lltcPrefixStr, uint32_t echoId, uint32_t sourceNodeId);
	static shared_ptr<Data> constructBack(string lltcPrefixStr, uint32_t echoId, uint32_t sourceNodeId, uint32_t destNodeId);
	static shared_ptr<Data> constructLsa(string lltcPrefixStr, uint32_t lsaId, uint32_t sourceNodeId,
									uint32_t updateSeqNo, list<LinkState>* lsList, string type);

	static string getCapsuleUri(string pitPrefixStr, uint32_t dataId);

};

}
}

#endif /* SRC_NDNSIM_NFD_DAEMON_FW_LLTC_MESSAGES_HELPER_HPP_ */
