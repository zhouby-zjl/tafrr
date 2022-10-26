#include "ns3/nstime.h"
#include "ns3/ndnSIM/ndn-cxx/lp/tags.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/ndn-cxx/encoding/block-helpers.hpp"
#include "ns3/ndnSIM/ndn-cxx/name.hpp"

#include "NFD/daemon/fw/forwarder.hpp"
#include "NFD/daemon/face/face-common.hpp"
#include "model/ndn-net-device-transport.hpp"
#include "NFD/daemon/table/pit-entry.hpp"
#include "NFD/daemon/table/pit-in-record.hpp"
#include "NFD/daemon/table/cs-policy-priority-fifo.hpp"

#include "ns3/ndnSIM/model/generic-log.hpp"

#include <algorithm>

#include "sdatp-strategy.hpp"

using namespace nfd::cs;
using namespace nfd::cs::priority_fifo;
using namespace std;
using namespace ns3;
using namespace ns3::ndn;

namespace nfd {
namespace fw {

NFD_LOG_INIT(SdatpStrategy);
NFD_REGISTER_STRATEGY(SdatpStrategy);

SdatpStrategy::SdatpStrategy(Forwarder& forwarder, const Name& name) : Strategy(forwarder) {
	this->setInstanceName(makeInstanceName(name, getStrategyName()));
	routes = GenericRoutesManager::getRoutesByForwarder(&forwarder);
	localCs = &forwarder.getCs();
	recentDataID = -1;
}

SdatpStrategy::~SdatpStrategy() {

}

const Name& SdatpStrategy::getStrategyName() {
	static Name strategyName("ndn:/localhost/nfd/strategy/sdatp/%FD%01");
	return strategyName;
}

void SdatpStrategy::afterReceiveInterest(const FaceEndpoint& ingress, const Interest& interest,
		const shared_ptr<pit::Entry>& pitEntry) {
	std::cout << "received interest with nounce: " << interest.getNonce() << " at node: " << routes->nodeID << std::endl;

	if (hasPendingOutRecords(*pitEntry)) return;

	Face* face = this->lookupForwardingEntry(0);
	if (face == nullptr) {
		const fib::Entry& feLocal = this->lookupFib(*pitEntry);
		if (feLocal.getNextHops().size() > 0) {
			for (const auto& nexthop : feLocal.getNextHops()) {
				Face& outFace = nexthop.getFace();
				this->sendInterest(pitEntry, FaceEndpoint(outFace, 0), interest);
			}
		} else {
			this->rejectPendingInterest(pitEntry);
		}
		return;
	}

	this->sendInterest(pitEntry, FaceEndpoint(*face, 0), interest);
}

uint32_t SdatpStrategy::getUID() {
	Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
	uint32_t uid = rand->GetInteger(0, UINT_MAX);

	return uid;
}

void SdatpStrategy::afterReceiveData(const shared_ptr<pit::Entry>& pitEntry,
		const FaceEndpoint& ingress, const Data& data) {
	Name dataName = data.getName();
	cout << "%%%%%%%>> " << dataName << " @ NodeID: " << routes->nodeID << ", time: " << Simulator::Now().GetSeconds() << endl;

	int nDataNameComponents = pitEntry->getName().size();
	string operationStr = dataName.get(nDataNameComponents).toUri(name::UriFormat::DEFAULT);
	string pitPrefixStr = pitEntry->getName().toUri(name::UriFormat::DEFAULT);
	stringstream ss;

	if (operationStr.compare("Data") == 0) {
		uint32_t dataID = stoull(dataName.get(nDataNameComponents + 1).toUri(name::UriFormat::DEFAULT));

		if (transDataIDs.find(dataID) != transDataIDs.end()) {
			return;
		}

		for (unordered_map<uint32_t, RetranEventState*>::iterator iter = retranStates.begin();
				iter != retranStates.end(); ++iter) {
			if (dataID == iter->second->dataID) {
				Simulator::Remove(iter->second->retranEventID);
				retranStates.erase(iter->first);
				break;
			}
		}

		Face* face = lookupForwardingEntry(1);
		if (face == NULL) {
			this->sendData(pitEntry, data, FaceEndpoint(*routes->appFace, 0));
			transDataIDs.insert(dataID);
			return;
		}

		this->sendData(pitEntry, data, FaceEndpoint(*face, 0));
		transDataIDs.insert(dataID);
		receivedDataIDs.insert(dataID);

		if (dataID > recentDataID + 1) {
			for (uint32_t dataID_s = recentDataID + 1; dataID_s < dataID; ++dataID_s) {
				uint32_t uid = getUID();
				list<uint32_t> lostDataIDs;
				lostDataIDs.push_back(dataID_s);
				shared_ptr<Data> rr = SdatpStrategy::constructRR(pitPrefixStr, &lostDataIDs, uid);

				Face* face_up = lookupForwardingEntry(0);
				if (face_up != NULL) {
					this->sendData(pitEntry, *rr, FaceEndpoint(*face_up, 0));
				}

				RetranEventState* es = new RetranEventState;
				es->dataID = dataID_s;
				es->times = 1;
				es->retranEventID = Simulator::Schedule(routes->roundTripTimeout, &SdatpStrategy::waitingRD, this, es, uid, dataID, pitEntry);
				retranStates[uid] = es;

			}
		}

		recentDataID = dataID;
		uint32_t uid = getUID();
		RetranEventState* es = new RetranEventState;
		es->dataID = recentDataID + 1;
		es->times = 1;
		es->retranEventID = Simulator::Schedule(routes->maxPiat, &SdatpStrategy::waitingRD, this, es, uid, dataID, pitEntry);
		retranStates[uid] = es;

	} else if (operationStr.compare("RD") == 0) {
		uint32_t uid = stoull(dataName.get(nDataNameComponents + 1).toUri(name::UriFormat::DEFAULT));
		list<uint32_t> lostDataIDs;
		extractDataIDs(data, &lostDataIDs);
		uint32_t lostDataID = *lostDataIDs.begin();

		if (retranStates.find(uid) != retranStates.end()) {
			RetranEventState* es = retranStates[uid];
			Simulator::Remove(es->retranEventID);
			retranStates.erase(uid);
		}

		if (transDataIDs.find(lostDataID) == transDataIDs.end()) {
			shared_ptr<Data> retranData = this->constructData(pitPrefixStr, lostDataID);
			transDataIDs.insert(lostDataID);
			Face* face = lookupForwardingEntry(1);
			if (face != NULL) {
				this->sendData(pitEntry, *retranData, FaceEndpoint(*face, 0));
			} else {
				this->sendData(pitEntry, data, FaceEndpoint(*routes->appFace, 0));
			}
			return;
		}

	} else if (operationStr.compare("RR") == 0) {
		uint32_t uid = stoull(dataName.get(nDataNameComponents + 1).toUri(name::UriFormat::DEFAULT));
		list<uint32_t> lostDataIDs;
		extractDataIDs(data, &lostDataIDs);
		uint32_t lostDataID = *lostDataIDs.begin();

		const Data* lostData = lookupInCs(pitPrefixStr, lostDataID);
		if (lostData != NULL) {
			Face* face = lookupForwardingEntry(1);
			if (face == NULL) return;
			list<uint32_t> lostDataIDs;
			lostDataIDs.push_back(lostDataID);
			shared_ptr<Data> rd = this->constructRD(pitPrefixStr, &lostDataIDs, uid);
			this->sendData(pitEntry, *rd, FaceEndpoint(*face, 0));
		} else {
			cout << "missed!" << endl;
		}
	}
}

void SdatpStrategy::waitingRD(RetranEventState* es, uint32_t uid, uint32_t dataID, const shared_ptr<pit::Entry>& pitEntry) {
	Face* face = lookupForwardingEntry(1);
	if (face == NULL) return;
	++es->times;
	string pitPrefixStr = pitEntry->getName().toUri(name::UriFormat::DEFAULT);

	list<uint32_t> lostDataIDs;
	lostDataIDs.push_back(dataID);
	shared_ptr<Data> rr = SdatpStrategy::constructRR(pitPrefixStr, &lostDataIDs, uid);

	Face* face_up = lookupForwardingEntry(0);
	if (face_up != NULL) {
		this->sendData(pitEntry, *rr, FaceEndpoint(*face_up, 0));
	}

	if (es->times < routes->maxCSynTimes) {
		es->retranEventID = Simulator::Schedule(routes->roundTripTimeout, &SdatpStrategy::waitingRD, this, es, uid, dataID, pitEntry);
	}
}

Face* SdatpStrategy::lookupForwardingEntry(int direction) {
	for (list<GenericFIBEntry*>::iterator iter = this->routes->fibEntries.begin(); iter != this->routes->fibEntries.end(); ++iter) {
		GenericFIBEntry* fe = *iter;
		if (fe->direction == direction) {
			return fe->face_nextHop;
		}
	}
	return NULL;
}

void SdatpStrategy::extractDataIDs(const Data& data, list<uint32_t>* dataIds) {
	dataIds->clear();

	const Block& payload = data.getContent();
	const uint8_t* buf = payload.value();

	size_t* nDataIdRegion = (size_t*) buf;
	uint32_t* dataIdRegion = (uint32_t*) (buf + sizeof(size_t));
	size_t n = nDataIdRegion[0];

	for (size_t i = 0; i < n; ++i) {
		uint32_t dataId = dataIdRegion[i];
		dataIds->push_back(dataId);
	}
}

shared_ptr<Data> SdatpStrategy::constructRR(string pitPrefixStr, list<uint32_t>* transDataIDs, uint32_t uid) {
	stringstream ss;
	ss << pitPrefixStr << "/RR/" << uid;
	size_t n = transDataIDs->size();
	size_t nBufBytes = sizeof(uint32_t) * n + sizeof(size_t);
	uint8_t* bufBytes = new uint8_t[nBufBytes];
	size_t* nDataIdRegion = (size_t*) bufBytes;
	nDataIdRegion[0] = n;
	uint32_t* dataIdRegion = (uint32_t*) (bufBytes + sizeof(size_t));
	int i = 0;
	for (list<uint32_t>::iterator iter = transDataIDs->begin(); iter != transDataIDs->end(); ++iter) {
		dataIdRegion[i] = *iter;
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

shared_ptr<Data> SdatpStrategy::constructRD(string pitPrefixStr, list<uint32_t>* transDataIDs, uint32_t uid) {
	stringstream ss;
	ss << pitPrefixStr << "/RD/" << uid;
	size_t n = transDataIDs->size();
	size_t nBufBytes = sizeof(uint32_t) * n + sizeof(size_t);
	uint8_t* bufBytes = new uint8_t[nBufBytes];
	size_t* nDataIdRegion = (size_t*) bufBytes;
	nDataIdRegion[0] = n;
	uint32_t* dataIdRegion = (uint32_t*) (bufBytes + sizeof(size_t));
	int i = 0;
	for (list<uint32_t>::iterator iter = transDataIDs->begin(); iter != transDataIDs->end(); ++iter) {
		dataIdRegion[i] = *iter;
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

shared_ptr<Data> SdatpStrategy::constructData(string pitPrefixStr, uint32_t dataID) {
	stringstream ss;
	ss << pitPrefixStr << "/Data/" << dataID;

	auto data = std::make_shared<Data>(ss.str());
	data->setFreshnessPeriod(time::milliseconds(1000));
	shared_ptr<::ndn::Buffer> buf = std::make_shared<::ndn::Buffer>(100);
	data->setContent(buf);
	StackHelper::getKeyChain().sign(*data);
	return data;
}

const Data* SdatpStrategy::lookupInCs(string pitPrefix, uint32_t dataId) {
	stringstream ss;
	ss << pitPrefix << "/Data/" << dataId;

	Name lostDataName(ss.str());
	Cs::const_iterator iter = localCs->find(lostDataName);

	shared_ptr<Data> newData = nullptr;
	if (iter != localCs->end()) {
		return &iter->getData();
	} else {
		return NULL;
	}
}

}
}
