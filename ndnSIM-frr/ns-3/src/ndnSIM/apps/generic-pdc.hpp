#ifndef SRC_NDNSIM_APPS_GENERIC_PDC_HPP_
#define SRC_NDNSIM_APPS_GENERIC_PDC_HPP_

#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/forwarder.hpp"
#include <list>
#include <unordered_map>
#include <queue>
#include <fstream>

using namespace nfd::fw;
using namespace ns3;
using namespace ns3::ndn;

namespace generic {

class GenericPDCApp : public ns3::ndn::App {
public:
	static ns3::TypeId GetTypeId();

	virtual void DoInitialize();
	virtual void DoDispose();
	virtual void StartApplication();
	virtual void StopApplication();

	virtual void OnInterest(std::shared_ptr<const ns3::ndn::Interest> interest);
	virtual void OnData(std::shared_ptr<const ns3::ndn::Data> contentObject);

	void setPDCName(std::string pdcName);
	void setPMU(std::string pmuName);
	void setNodeID(uint32_t nodeID);

private:
	void SendInterest();

	std::string pdcName;
	std::string pmuName;
	uint32_t nodeID;
};

}


#endif /* SRC_NDNSIM_APPS_GENERIC_PDC_HPP_ */
