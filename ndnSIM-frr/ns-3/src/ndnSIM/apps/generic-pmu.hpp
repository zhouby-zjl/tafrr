#ifndef SRC_NDNSIM_APPS_GENERIC_PMU_HPP_
#define SRC_NDNSIM_APPS_GENERIC_PMU_HPP_

#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include <string>
#include <fstream>

namespace generic {

class GenericPMUApp : public ns3::ndn::App {
public:
	static ns3::TypeId GetTypeId();

	virtual void DoInitialize();
	virtual void DoDispose();
	virtual void StartApplication();
	virtual void StopApplication();

	virtual void OnInterest(std::shared_ptr<const ns3::ndn::Interest> interest);
	virtual void OnData(std::shared_ptr<const ns3::ndn::Data> contentObject);

	void setFreq(int freq);
	void setPmuName(std::string pmuName);

private:
	void sendData();

	int dataId;
	std::ofstream outLog_pmu;
	ns3::Time piat;
	std::string pmuName;
};

}

#endif /* SRC_NDNSIM_APPS_GENERIC_PMU_HPP_ */
