#include "generic-routes-manager.hpp"
#include "ns3/ptr.h"

using namespace ns3::ndn;
using namespace std;


namespace nfd {
namespace fw {

unordered_map<Forwarder*, GenericRoutes*> GenericRoutesManager::routesMap;
unordered_map<uint32_t, GenericRoutes*> GenericRoutesManager::nodeIDRoutesMap;

GenericRoutesManager::GenericRoutesManager() {

}

void GenericRoutesManager::boundForwarderWithRoutes(Forwarder* forwarder,  uint32_t nodeID, GenericRoutes* routes) {
	routesMap[forwarder] = routes;
	nodeIDRoutesMap[nodeID] = routes;
}

GenericRoutes* GenericRoutesManager::getRoutesByForwarder(Forwarder* forwarder) {
	if (routesMap.find(forwarder) == routesMap.end()) return nullptr;
	GenericRoutes* routes = routesMap[forwarder];
	return routes;
}

void GenericRoutesManager::registerAppFace(uint32_t nodeID, shared_ptr<Face> appFace) {
	if (nodeIDRoutesMap.find(nodeID) == nodeIDRoutesMap.end()) return;
	GenericRoutes* routes = nodeIDRoutesMap[nodeID];
	routes->appFace = appFace;
}

}
}
