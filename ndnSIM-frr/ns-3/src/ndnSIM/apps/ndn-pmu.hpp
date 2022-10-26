/*
 * ndn-pmu.hpp
 *
 *  Created on: 2020年6月15日
 *      Author: zby
 */

#ifndef SRC_NDNSIM_APPS_NDN_PMU_HPP_
#define SRC_NDNSIM_APPS_NDN_PMU_HPP_

#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include <string>

namespace ns3 {
namespace ndn {
class PMUApp : public App {
public:
	static TypeId GetTypeId();

	void setPmuID(int pmuID);
	std::string getPrefix();

	virtual void DoInitialize();
	virtual void DoDispose();
	virtual void StartApplication();
	virtual void StopApplication();

	virtual void OnInterest(std::shared_ptr<const Interest> interest);
	virtual void OnData(std::shared_ptr<const Data> contentObject);

private:
	void SendInterest();

	int m_pmuID;
	std::string m_prefix;
};
}
}

#endif /* SRC_NDNSIM_APPS_NDN_PMU_HPP_ */
