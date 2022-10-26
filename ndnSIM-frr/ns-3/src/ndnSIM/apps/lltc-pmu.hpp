/*
 * This work is licensed under CC BY-NC-SA 4.0
 * (https://creativecommons.org/licenses/by-nc-sa/4.0/).
 * Copyright (c) 2021 Boyang Zhou
 *
 * This file is a part of "Disruption Resilient Transport Protocol"
 * (https://github.com/zhouby-zjl/drtp/).
 * Written by Boyang Zhou (zhouby@zhejianglab.com)
 *
 * This software is protected by the patents numbered with PCT/CN2021/075891,
 * ZL202110344405.7 and ZL202110144836.9, as well as the software copyrights
 * numbered with 2020SR1875227 and 2020SR1875228.
 */

#ifndef SRC_NDNSIM_APPS_LLTC_PMU_HPP_
#define SRC_NDNSIM_APPS_LLTC_PMU_HPP_


#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "ns3/ndnSIM/model/lltc/lltc-config.hpp"
#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/random-variable-stream.h"
#include <string>
#include <fstream>



namespace lltc {

class LltcPMUApp : public ns3::ndn::App {
public:
	static ns3::TypeId GetTypeId();

	void setPmuName(std::string pmuName);
	void setFreq(int freq);
	void setNodeID(uint32_t nodeID);

	virtual void DoInitialize();
	virtual void DoDispose();
	virtual void StartApplication();
	virtual void StopApplication();

	virtual void OnInterest(std::shared_ptr<const ns3::ndn::Interest> interest);
	virtual void OnData(std::shared_ptr<const ns3::ndn::Data> contentObject);

private:
	void sendData(uint32_t rsgId);

	std::string pmuName;
	int freq;
	ns3::Time piat;
	int dataId;
	uint32_t nodeID;
	std::ofstream outLog_pmu;

	ns3::Ptr<ns3::RandomVariableStream> rand;
};

}



#endif /* SRC_NDNSIM_APPS_LLTC_PMU_HPP_ */
