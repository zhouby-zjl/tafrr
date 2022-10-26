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

#include "lltc-pmu.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/double.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ndnSIM/ndn-cxx/lp/tags.hpp"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/lltc-common.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/lltc-fs.hpp"
#include "lltc-utils.hpp"

#include <string>

using namespace ns3;
using namespace ns3::ndn;
using namespace std;
using namespace nfd::fw;

NS_LOG_COMPONENT_DEFINE("lltc.LltcPMUApp");

namespace lltc {

NS_OBJECT_ENSURE_REGISTERED(LltcPMUApp);

TypeId LltcPMUApp::GetTypeId() {
	static TypeId tid = TypeId("lltc::LltcPMUApp")
					.SetGroupName("Lltc")
					.SetParent<App>().AddConstructor<LltcPMUApp>();
	return tid;
}

void LltcPMUApp::setFreq(int freq) {
	this->freq = freq;
	piat = MicroSeconds(1000000.0 / (double) freq);

	if (LltcConfig::ENABLE_PMU_EXP_RAND) {
		rand = CreateObject<ExponentialRandomVariable>();
		rand->SetAttribute("Mean", DoubleValue(1.0 / freq));
		rand->SetAttribute("Bound", DoubleValue(50 * 1.0 / freq));
	} else {
		rand = NULL;
	}
}

void LltcPMUApp::setNodeID(uint32_t nodeID) {
	this->nodeID = nodeID;
}

void LltcPMUApp::setPmuName(string pmuName) {
	this->pmuName = pmuName;
}

void LltcPMUApp::StartApplication() {
	App::StartApplication();
	LltcFsManager::registerNonPitApp(nodeID, &(*m_face));
	FibHelper::AddRoute(GetNode(), pmuName, m_face, 0);

	stringstream ss;
	string _pmuName = pmuName;
	string_replace(_pmuName, "/", "-");
	ss << LltcConfig::SIM_LOG_DIR << "log" << _pmuName;
	outLog_pmu.open(ss.str(), ios::trunc);
	cout << "Log PMU is opened: " << ss.str() << endl;
}

void LltcPMUApp::StopApplication() {
	outLog_pmu.flush();
}

void LltcPMUApp::DoInitialize() {
	App::DoInitialize();
}

void LltcPMUApp::DoDispose() {
	App::DoDispose();
}

void LltcPMUApp::OnInterest(std::shared_ptr<const Interest> interest) {
	App::OnInterest(interest);
	uint32_t rsgId = (uint32_t) interest->getTag<::ndn::lp::LltcRsgIdTag>()->get();
	string prefix = interest->getName().toUri(name::UriFormat::DEFAULT);
	sendData(rsgId);
}

void LltcPMUApp::sendData(uint32_t rsgId) {
	stringstream ss;
	ss << pmuName << "/" << LLTC_MSG_OP_CAPSULE << "/" << dataId;
	auto data = std::make_shared<Data>(string(ss.str()));
	data->setFreshnessPeriod(time::milliseconds(1000));
	data->setContent(std::make_shared< ::ndn::Buffer>(1024));
	data->setTag<::ndn::lp::LltcRsgIdTag>(make_shared<lp::LltcRsgIdTag>(rsgId));
	StackHelper::getKeyChain().sign(*data);
	m_transmittedDatas(data, this, m_face);
	m_appLink->onReceiveData(*data);

	outLog_pmu << Simulator::Now() << "," << pmuName << "," << dataId << endl;
	outLog_pmu.flush();

	cout << "LltcPMUApp send data " << endl;
	++dataId;

	if (LltcConfig::ENABLE_PMU_EXP_RAND) {
		double piat_rand = 0.0;
		do {
			piat_rand = rand->GetValue();
		} while (piat_rand <= 0.0);
		Simulator::Schedule(ns3::Seconds(piat_rand), &LltcPMUApp::sendData, this, rsgId);
	} else {
		Simulator::Schedule(piat, &LltcPMUApp::sendData, this, rsgId);
	}
}


void LltcPMUApp::OnData(std::shared_ptr<const Data> data) {
	std::cout << "--> DATA received at PMU for name " << data->getName().toUri(name::UriFormat::DEFAULT) << std::endl;
}


}
