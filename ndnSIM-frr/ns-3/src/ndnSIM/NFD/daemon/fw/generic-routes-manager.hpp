#ifndef SRC_NDNSIM_NFD_DAEMON_FW_GENERIC_ROUTES_MANAGER_HPP_
#define SRC_NDNSIM_NFD_DAEMON_FW_GENERIC_ROUTES_MANAGER_HPP_

#include "NFD/daemon/face/face-common.hpp"
#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ptr.h"
#include "NFD/daemon/fw/forwarder.hpp"
#include "NFD/daemon/face/face-common.hpp"
#include "model/lltc/lltc-resilient-routes-subgraph.hpp"

#include <list>
#include <unordered_map>

using namespace ns3::ndn;
using namespace std;
using namespace nfd::face;


namespace nfd {
namespace fw {

struct GenericFIBEntry {
	Face* face_nextHop;
	int direction;  // 0 stands for the downstream, and 1 for the upstream
};

struct GenericRoutes {
	list<GenericFIBEntry*> fibEntries;
	uint32_t nodeID;
	ns3::Time roundTripTimeout;
	ns3::Time maxPiat;
	uint32_t maxCSynTimes;
	shared_ptr<Face> appFace;
};

class GenericRoutesManager {
public:
	GenericRoutesManager();
	static void boundForwarderWithRoutes(Forwarder* forwarder, uint32_t nodeID, GenericRoutes* routes);
	static GenericRoutes* getRoutesByForwarder(Forwarder* forwarder);
	static void registerAppFace(uint32_t nodeId, shared_ptr<Face> appFace);

private:
	static unordered_map<Forwarder*, GenericRoutes*> routesMap;
	static unordered_map<uint32_t, GenericRoutes*> nodeIDRoutesMap;
};

}
}



#endif /* SRC_NDNSIM_NFD_DAEMON_FW_GENERIC_ROUTES_MANAGER_HPP_ */
