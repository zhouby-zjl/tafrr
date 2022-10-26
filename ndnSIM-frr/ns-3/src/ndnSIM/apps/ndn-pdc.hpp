/*
 * ndn-pdc.hpp
 *
 *  Created on: 2020年6月15日
 *      Author: zby
 */

#ifndef SRC_NDNSIM_APPS_NDN_PDC_HPP_
#define SRC_NDNSIM_APPS_NDN_PDC_HPP_

#include "ns3/ndnSIM/apps/ndn-app.hpp"

namespace ns3 {
namespace ndn {
class PDCApp : public App {
public:
	static TypeId GetTypeId();

	virtual void DoInitialize();
	virtual void DoDispose();
	virtual void StartApplication();
	virtual void StopApplication();

	virtual void OnInterest(std::shared_ptr<const Interest> interest);
	virtual void OnData(std::shared_ptr<const Data> contentObject);

	void SetNPmu(int nPMU);

private:
	void SendInterest();
	int m_nPMU;
};
}
}

#endif /* SRC_NDNSIM_APPS_NDN_PDC_HPP_ */
