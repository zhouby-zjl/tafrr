#ifndef SRC_NDNSIM_NFD_DAEMON_FW_R2T_STRATEGY_HPP_
#define SRC_NDNSIM_NFD_DAEMON_FW_R2T_STRATEGY_HPP_

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

struct CsynEventState {
	uint32_t dataID;
	ns3::EventId retranEventID;
	uint32_t times;
};

class R2tStrategy : public Strategy {
public:
	R2tStrategy(Forwarder& forwarder, const Name& name = getStrategyName());
	virtual ~R2tStrategy() override;
    static const Name& getStrategyName();

    void afterReceiveInterest(const FaceEndpoint& ingress, const Interest& interest,
	                       const shared_ptr<pit::Entry>& pitEntry) override;

    void afterReceiveData(const shared_ptr<pit::Entry>& pitEntry,
                     const FaceEndpoint& ingress, const Data& data) override;
    void waitingCACK(CsynEventState* es, const shared_ptr<pit::Entry>& pitEntry);

    Face* lookupForwardingEntry(int direction);
    const Data* lookupInCs(string pitPrefix, uint32_t dataId);
    shared_ptr<Data> constructCACK(string pitPrefixStr, uint32_t dataID);

    Cs* localCs;
    GenericRoutes* routes;
    unordered_set<uint32_t> transDataIDs;
    unordered_map<uint32_t, CsynEventState*> csynStates;
};

}
}


#endif /* SRC_NDNSIM_NFD_DAEMON_FW_R2T_STRATEGY_HPP_ */
