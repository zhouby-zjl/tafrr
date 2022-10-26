#ifndef SRC_NDNSIM_NFD_DAEMON_FW_SDATP_STRATEGY_HPP_
#define SRC_NDNSIM_NFD_DAEMON_FW_SDATP_STRATEGY_HPP_



#include "ns3/random-variable-stream.h"

#include "strategy.hpp"
#include "algorithm.hpp"
#include "common/global.hpp"
#include "common/logger.hpp"
#include "ndn-cxx/tag.hpp"
#include <list>
#include <unordered_map>

#include "generic-routes-manager.hpp"

using namespace nfd;
using namespace std;

namespace nfd {
namespace fw {

struct RetranEventState {
	uint32_t dataID;
	ns3::EventId retranEventID;
	uint32_t times;
};

class SdatpStrategy : public Strategy {
public:
	SdatpStrategy(Forwarder& forwarder, const Name& name = getStrategyName());
	virtual ~SdatpStrategy() override;
    static const Name& getStrategyName();

    void afterReceiveInterest(const FaceEndpoint& ingress, const Interest& interest,
	                       const shared_ptr<pit::Entry>& pitEntry) override;

    void afterReceiveData(const shared_ptr<pit::Entry>& pitEntry,
                     const FaceEndpoint& ingress, const Data& data) override;
    void waitingRD(RetranEventState* es, uint32_t uid, uint32_t dataID, const shared_ptr<pit::Entry>& pitEntry);

    Face* lookupForwardingEntry(int direction);
    const Data* lookupInCs(string pitPrefix, uint32_t dataId);
    shared_ptr<Data> constructRR(string pitPrefixStr, list<uint32_t>* transDataIDs, uint32_t uid);
    shared_ptr<Data> constructRD(string pitPrefixStr, list<uint32_t>* transDataIDs, uint32_t uid);
    shared_ptr<Data> constructData(string pitPrefixStr, uint32_t dataID);
    void extractDataIDs(const Data& data, list<uint32_t>* dataIds);
    uint32_t getUID();

    Cs* localCs;
    GenericRoutes* routes;
    unordered_set<uint32_t> transDataIDs;
    unordered_set<uint32_t> receivedDataIDs;
    int64_t recentDataID;
    unordered_map<uint32_t, RetranEventState*> retranStates;
};

}
}



#endif /* SRC_NDNSIM_NFD_DAEMON_FW_SDATP_STRATEGY_HPP_ */
