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

#include "r2t-strategy.hpp"

using namespace nfd::cs;
using namespace nfd::cs::priority_fifo;
using namespace std;
using namespace ns3;
using namespace ns3::ndn;

namespace nfd {
namespace fw {

NFD_LOG_INIT(R2tStrategy);
NFD_REGISTER_STRATEGY(R2tStrategy);

R2tStrategy::R2tStrategy(Forwarder& forwarder, const Name& name) : Strategy(forwarder) {
	this->setInstanceName(makeInstanceName(name, getStrategyName()));
	routes = GenericRoutesManager::getRoutesByForwarder(&forwarder);
	localCs = &forwarder.getCs();
}

R2tStrategy::~R2tStrategy() {

}

const Name& R2tStrategy::getStrategyName() {
	static Name strategyName("ndn:/localhost/nfd/strategy/r2t/%FD%01");
	return strategyName;
}

void R2tStrategy::afterReceiveInterest(const FaceEndpoint& ingress, const Interest& interest,
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

void R2tStrategy::afterReceiveData(const shared_ptr<pit::Entry>& pitEntry,
		const FaceEndpoint& ingress, const Data& data) {
	Name dataName = data.getName();
	//cout << "%%%%%%%>> " << dataName << " @ NodeID: " << routes->nodeID << ", time: " << Simulator::Now().GetSeconds() << endl;

	int nDataNameComponents = pitEntry->getName().size();
	string operationStr = dataName.get(nDataNameComponents).toUri(name::UriFormat::DEFAULT);
	string pitPrefixStr = pitEntry->getName().toUri(name::UriFormat::DEFAULT);
	stringstream ss;

	if (operationStr.compare("Data") == 0) {
		uint32_t dataID = stoull(dataName.get(nDataNameComponents + 1).toUri(name::UriFormat::DEFAULT));

		if (transDataIDs.find(dataID) != transDataIDs.end()) {
			return;
		}

		cout << "Data with dataID: " <<  dataID << " @ NodeID: " << routes->nodeID << ", time: " << Simulator::Now().GetSeconds() << endl;

		Face* face = lookupForwardingEntry(1);
		if (face == NULL) {
			this->sendData(pitEntry, data, FaceEndpoint(*routes->appFace, 0));

		} else {
			this->sendData(pitEntry, data, FaceEndpoint(*face, 0));

			CsynEventState* es = new CsynEventState;
			es->dataID = dataID;
			csynStates[dataID] = es;

			if (routes->maxCSynTimes >= 1) {
				es->times = 1;
				es->retranEventID = Simulator::Schedule(routes->roundTripTimeout, &R2tStrategy::waitingCACK, this, es, pitEntry);
			} else {
				es->times = -1;
			}
		}

		transDataIDs.insert(dataID);

		shared_ptr<Data> cack_up = R2tStrategy::constructCACK(pitPrefixStr, dataID);

		Face* face_up = lookupForwardingEntry(0);
		if (face_up != NULL) {
			this->sendData(pitEntry, *cack_up, FaceEndpoint(*face_up, 0));
		}


	} else if (operationStr.compare("CACK") == 0) {
		uint32_t dataID = stoull(dataName.get(nDataNameComponents + 1).toUri(name::UriFormat::DEFAULT));

		cout << "CACK with dataID: " << dataID << " @ NodeID: " << routes->nodeID << ", time: " << Simulator::Now().GetSeconds() << endl;
		CsynEventState* es = csynStates[dataID];
		if (routes->maxCSynTimes >= 1) {
			Simulator::Remove(es->retranEventID);
		}
		csynStates.erase(dataID);
	}
}

void R2tStrategy::waitingCACK(CsynEventState* es, const shared_ptr<pit::Entry>& pitEntry) {
	Face* face = lookupForwardingEntry(1);
	if (face == NULL) return;

	string pitPrefixStr = pitEntry->getName().toUri(name::UriFormat::DEFAULT);
	const Data* lostData = lookupInCs(pitPrefixStr, es->dataID);
	this->sendData(pitEntry, *lostData, FaceEndpoint(*face, 0));

	++es->times;
	if (es->times <= routes->maxCSynTimes) {
		es->retranEventID = Simulator::Schedule(routes->roundTripTimeout, &R2tStrategy::waitingCACK, this, es, pitEntry);
	}
}

Face* R2tStrategy::lookupForwardingEntry(int direction) {
	for (list<GenericFIBEntry*>::iterator iter = this->routes->fibEntries.begin(); iter != this->routes->fibEntries.end(); ++iter) {
		GenericFIBEntry* fe = *iter;
		if (fe->direction == direction) {
			return fe->face_nextHop;
		}
	}
	return NULL;
}



shared_ptr<Data> R2tStrategy::constructCACK(string pitPrefixStr, uint32_t dataID) {
	stringstream ss;
	ss << pitPrefixStr << "/CACK/" << dataID;

	auto data = std::make_shared<Data>(ss.str());
	data->setFreshnessPeriod(time::milliseconds(1000));
	StackHelper::getKeyChain().sign(*data);
	return data;
}

const Data* R2tStrategy::lookupInCs(string pitPrefix, uint32_t dataId) {
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
