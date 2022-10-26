/*
 * ndn-pdc.cpp
 *
 *  Created on: 2020年6月15日
 *      Author: zby
 */

#include "ndn-pdc.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include <string>

NS_LOG_COMPONENT_DEFINE("ndn.PDCApp");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(PDCApp);

TypeId PDCApp::GetTypeId() {
	static TypeId tid = TypeId("ns3::ndn::PDCApp")
			.SetGroupName("Ndn")
			.SetParent<App>().AddConstructor<PDCApp>();
	return tid;
}

void PDCApp::StartApplication() {
	App::StartApplication();
	ndn::FibHelper::AddRoute(GetNode(), "/pdc", m_face, 0);
	Simulator::Schedule(Seconds(1.0), &PDCApp::SendInterest, this);
	std::cout << "PDC started" << std::endl;
}

void PDCApp::StopApplication() {

}

void PDCApp::SetNPmu(int nPMU) {
	this->m_nPMU = nPMU;
}

void PDCApp::DoInitialize() {
	App::DoInitialize();
}

void PDCApp::DoDispose() {
	App::DoDispose();
}

void PDCApp::OnInterest(std::shared_ptr<const Interest> interest) {
	App::OnInterest(interest);
}


void PDCApp::OnData(std::shared_ptr<const ndn::Data> data) {
//	NS_LOG_DEBUG("Receiving Data packet for " << data->getName());
	std::cout << "DATA received for name " << data->getName() << std::endl;
}

void PDCApp::SendInterest() {
	for (int i = 0; i < m_nPMU; ++i) {
		std::ostringstream oss;
		oss << "/pmu/" << i;
		auto interest = std::make_shared<ndn::Interest>(oss.str());
		Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
		interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
		interest->setInterestLifetime(ndn::time::seconds(1));

		m_transmittedInterests(interest, this, m_face);
		m_appLink->onReceiveInterest(*interest);

		std::cout << "sending Interest packet with name: " << oss.str() << std::endl;
	}

	Simulator::Schedule(Seconds(1.0), &PDCApp::SendInterest, this);
}

}
}

