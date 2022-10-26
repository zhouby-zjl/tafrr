/*
 * ndn-pmu.cpp
 *
 *  Created on: 2020年6月15日
 *      Author: zby
 */


#include "ndn-pmu.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include <string>


NS_LOG_COMPONENT_DEFINE("ndn.PMUApp");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(PMUApp);

TypeId PMUApp::GetTypeId() {
	static TypeId tid = TypeId("ns3::ndn::PMUApp")
					.SetGroupName("Ndn")
					.SetParent<App>().AddConstructor<PMUApp>();
	return tid;
}

std::string PMUApp::getPrefix() {
	return m_prefix;
}

void PMUApp::setPmuID(int pmuID) {
	this->m_pmuID = pmuID;
	std::ostringstream ss;
	ss << "/pmu/" << m_pmuID;
	m_prefix = ss.str();
}

void PMUApp::StartApplication() {
	App::StartApplication();
	ndn::FibHelper::AddRoute(GetNode(), m_prefix.c_str(), m_face, 0);
	Simulator::Schedule(Seconds(1.0), &PMUApp::SendInterest, this);
}

void PMUApp::StopApplication() {

}

void PMUApp::DoInitialize() {
	App::DoInitialize();
}

void PMUApp::DoDispose() {
	App::DoDispose();
}

void PMUApp::OnInterest(std::shared_ptr<const Interest> interest) {
	App::OnInterest(interest);

	auto data = std::make_shared<ndn::Data>(interest->getName());
	data->setFreshnessPeriod(ndn::time::milliseconds(1000));
	data->setContent(std::make_shared< ::ndn::Buffer>(1024));
	ndn::StackHelper::getKeyChain().sign(*data);

	NS_LOG_DEBUG("Sending Data packet for " << data->getName());

	// Call trace (for logging purposes)
	m_transmittedDatas(data, this, m_face);

	m_appLink->onReceiveData(*data);
}


void PMUApp::OnData(std::shared_ptr<const ndn::Data> data) {
	//	NS_LOG_DEBUG("Receiving Data packet for " << data->getName());
	std::cout << "DATA received for name " << data->getName() << std::endl;
}

void PMUApp::SendInterest() {
}

}
}
