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

#include "lltc-messages-helper.hpp"
#include "ns3/nstime.h"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/ndn-cxx/encoding/block-helpers.hpp"
#include <iostream>
#include <cmath>

using namespace nfd;
using namespace std;
using namespace ns3::ndn;

namespace nfd {
namespace fw {


shared_ptr<Data> LltcMessagesHelper::constructEcho(string lltcPrefixStr, uint32_t echoId, uint32_t sourceNodeId) {
	stringstream ss;
	ss << lltcPrefixStr << "/" << LLTC_MSG_OP_ECHO << "/" << echoId << "/" << sourceNodeId << "/Unused";
	string prefix = ss.str();
	auto data = std::make_shared<Data>();
	data->setName(prefix);
	data->setFreshnessPeriod(time::milliseconds(1000));
	StackHelper::getKeyChain().sign(*data);
	return data;
}

shared_ptr<Data> LltcMessagesHelper::constructBack(string lltcPrefixStr, uint32_t echoId, uint32_t sourceNodeId, uint32_t destNodeId) {
	stringstream ss;
	ss << lltcPrefixStr << "/" << LLTC_MSG_OP_BACK << "/" << echoId << "/" << sourceNodeId << "/" << destNodeId;
	string prefix = ss.str();
	auto data = std::make_shared<Data>();
	data->setName(prefix);
	data->setFreshnessPeriod(time::milliseconds(1000));
	StackHelper::getKeyChain().sign(*data);
	return data;
}


string LltcMessagesHelper::getCapsuleUri(string pitPrefixStr, uint32_t dataId) {
	stringstream ss;
	ss.str("");
	ss << pitPrefixStr << "/" << LLTC_MSG_OP_CAPSULE << "/" << dataId;
	return ss.str();
}

shared_ptr<Data> LltcMessagesHelper::constructCapsule(string pitPrefixStr, LltcDataID dataID, const Data& origData,
												bool isRetrans, LltcDataID origDataID) {
	stringstream ss;
	ss.str("");
	if (isRetrans) {
		ss << pitPrefixStr << "/" << LLTC_MSG_OP_CAPSULE << "/" << dataID;// << "/Retrans/" << origDataID;
	} else {
		ss << pitPrefixStr << "/" << LLTC_MSG_OP_CAPSULE << "/" << dataID;// << "/Normal/Unused";
	}

	shared_ptr<Data> newData = make_shared<Data>(string(ss.str()));
	newData->setFreshnessPeriod(origData.getFreshnessPeriod());
	newData->setContent(origData.getContent());
	StackHelper::getKeyChain().sign(*newData);

	return newData;
}

shared_ptr<Data> LltcMessagesHelper::constructCapsuleOnRSP(string pitPrefixStr, LltcDataID dataID, const Data& origData,
												bool isRetrans, LltcDataID origDataID) {
	stringstream ss;
	ss.str("");

	if (isRetrans) {
		ss << pitPrefixStr << "/" << LLTC_MSG_OP_CAPSULE_ON_RSP << "/" << dataID << "/Retrans/" << origDataID;
	} else {
		ss << pitPrefixStr << "/" << LLTC_MSG_OP_CAPSULE_ON_RSP << "/" << dataID << "/Normal/Unused";
	}

	shared_ptr<Data> newData = make_shared<Data>(string(ss.str()));
	newData->setFreshnessPeriod(origData.getFreshnessPeriod());
	newData->setContent(origData.getContent());
	StackHelper::getKeyChain().sign(*newData);

	return newData;
}

CapsuleUri LltcMessagesHelper::parseCapsuleUri(const Name& prefixName, const Name& dataName) {
	CapsuleUri du;
	int nDataNameComponents = prefixName.size();
	du.dataId = (LltcDataID) stoull(dataName.get(nDataNameComponents + 1).toUri(name::UriFormat::DEFAULT));
	if (dataName.size() - nDataNameComponents == 4) {
		string type = dataName.get(nDataNameComponents + 2).toUri(name::UriFormat::DEFAULT);
		if (type.compare("Retrans") == 0) {
			du.isRetrans = true;
			du.requestedDataId = stoll(dataName.get(nDataNameComponents + 3).toUri(name::UriFormat::DEFAULT));
		}
	} else {
		du.isRetrans = false;
		du.requestedDataId = du.dataId;
	}
	du.prefix = prefixName.toUri(name::UriFormat::DEFAULT);
	return du;
}

ChangePathUri LltcMessagesHelper::parseChangePathUri(const Name& prefixName, const Name& dataName) {
	ChangePathUri cpu;

	int nDataNameComponents = prefixName.size();
	cpu.prefix = prefixName.toUri(name::UriFormat::DEFAULT);
	cpu.nodeIdToChangePath = stoll(dataName.get(nDataNameComponents + 1).toUri(name::UriFormat::DEFAULT));
	cpu.newPathId = stoll(dataName.get(nDataNameComponents + 2).toUri(name::UriFormat::DEFAULT));
	cpu.ununused = dataName.get(nDataNameComponents + 3).toUri(name::UriFormat::DEFAULT);

	return cpu;
}

EchoUri LltcMessagesHelper::parseEchoUri(const Name& lltcPrefixName, const Name& dataName) {
	EchoUri eru;

	int nDataNameComponents = lltcPrefixName.size();
	eru.echoId = stoll(dataName.get(nDataNameComponents + 1).toUri(name::UriFormat::DEFAULT));
	eru.sourceNodeId = stoll(dataName.get(nDataNameComponents + 2).toUri(name::UriFormat::DEFAULT));

	return eru;
}

BackUri LltcMessagesHelper::parseBackUri(const Name& lltcPrefixName, const Name& dataName) {
	BackUri ebu;

	int nDataNameComponents = lltcPrefixName.size();
	ebu.echoId = stoll(dataName.get(nDataNameComponents + 1).toUri(name::UriFormat::DEFAULT));
	ebu.sourceNodeId = stoll(dataName.get(nDataNameComponents + 2).toUri(name::UriFormat::DEFAULT));
	ebu.destNodeId = stoll(dataName.get(nDataNameComponents + 3).toUri(name::UriFormat::DEFAULT));
	return ebu;
}


CapsuleOnRSPUri LltcMessagesHelper::parseCapsuleOnRSPUri(const Name& prefixName, const Name& dataName) {
	CapsuleOnRSPUri dbu;
	int nDataNameComponents = prefixName.size();
	dbu.dataId = (LltcDataID) stoull(dataName.get(nDataNameComponents + 1).toUri(name::UriFormat::DEFAULT));
	if (dataName.size() - nDataNameComponents == 4) {
		string type = dataName.get(nDataNameComponents + 2).toUri(name::UriFormat::DEFAULT);
		if (type.compare("Retrans") == 0) {
			dbu.isRetrans = true;
			dbu.requestedDataId = stoll(dataName.get(nDataNameComponents + 4).toUri(name::UriFormat::DEFAULT));
		} else if (type.compare("Normal") == 0) {
			dbu.isRetrans = false;
			dbu.requestedDataId = dbu.dataId;
		}
	} else {
		dbu.isRetrans = false;
		dbu.requestedDataId = dbu.dataId;
	}
	dbu.prefix = prefixName.toUri(name::UriFormat::DEFAULT);
	return dbu;
}



shared_ptr<Data> LltcMessagesHelper::constructLsa(string lltcPrefixStr, uint32_t lsaId, uint32_t sourceNodeId,
				uint32_t updateSeqNo, list<LinkState>* lsList, string type) {
	stringstream ss;
	ss.str("");
	ss << lltcPrefixStr << "/" << LLTC_MSG_OP_LSA << "/" << lsaId << "/" << sourceNodeId << "/" << updateSeqNo
			<< "-" << type;

	size_t n = lsList->size();
	size_t nBufBytes = sizeof(LinkState) * n + sizeof(size_t);
	uint8_t* bufBytes = new uint8_t[nBufBytes];
	size_t* nLsRegion = (size_t*) bufBytes;
	nLsRegion[0] = n;
	LinkState* dataIdRegion = (LinkState*) (bufBytes + sizeof(size_t));
	int i = 0;
	for (list<LinkState>::iterator iter = lsList->begin(); iter != lsList->end(); ++iter) {
		dataIdRegion[i].neighboredNodeId = iter->neighboredNodeId;
		dataIdRegion[i].status = iter->status;
		++i;
	}

	auto data = std::make_shared<Data>(ss.str());
	data->setFreshnessPeriod(time::milliseconds(1000));
	shared_ptr<::ndn::Buffer> buf = std::make_shared<::ndn::Buffer>(nBufBytes);
	data->setContent(buf);
	for (size_t i = 0; i < nBufBytes; ++i) {
		(*buf)[i] = bufBytes[i];
	}
	StackHelper::getKeyChain().sign(*data);

	return data;
}

LsaUri LltcMessagesHelper::parseLsaUri(const Name& lltcPrefixName, const Name& dataName) {
	LsaUri flu;

	int nDataNameComponents = lltcPrefixName.size();
	flu.lsaId = stoll(dataName.get(nDataNameComponents + 1).toUri(name::UriFormat::DEFAULT));
	flu.sourceNodeId = stoll(dataName.get(nDataNameComponents + 2).toUri(name::UriFormat::DEFAULT));
	string seqNoAndType = dataName.get(nDataNameComponents + 3).toUri(name::UriFormat::DEFAULT);
	size_t i = seqNoAndType.find("-");
	flu.updateSeqNo = stoll(seqNoAndType.substr(0, i));
	flu.type = seqNoAndType.substr(i + 1);

	return flu;
}

void LltcMessagesHelper::extractLsa(const Data& data, list<LinkState>* lsList) {
	lsList->clear();

	const Block& payload = data.getContent();
	const uint8_t* buf = payload.value();

	size_t* nDataIdRegion = (size_t*) buf;
	LinkState* lsRegion = (LinkState*) (buf + sizeof(size_t));
	size_t n = nDataIdRegion[0];

	for (size_t i = 0; i < n; ++i) {
		LinkState ls = lsRegion[i];
		lsList->push_back(ls);
	}
}

}
}
