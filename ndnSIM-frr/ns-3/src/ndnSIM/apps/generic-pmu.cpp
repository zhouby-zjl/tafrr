#include "generic-pmu.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ndnSIM/ndn-cxx/lp/tags.hpp"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/lltc-common.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/lltc-fs.hpp"
#include "lltc-utils.hpp"

#include "ns3/ndnSIM/model/generic-log.hpp"

#include <string>

using namespace ns3;
using namespace ns3::ndn;
using namespace std;
using namespace nfd::fw;

namespace generic {

NS_LOG_COMPONENT_DEFINE("generic.GenericPMUApp");

NS_OBJECT_ENSURE_REGISTERED(GenericPMUApp);

TypeId GenericPMUApp::GetTypeId() {
	static TypeId tid = TypeId("generic::GenericPMUApp")
					.SetGroupName("Generic")
					.SetParent<App>().AddConstructor<GenericPMUApp>();
	return tid;
}


void GenericPMUApp::StartApplication() {
	App::StartApplication();
	FibHelper::AddRoute(GetNode(), pmuName, m_face, 0);
}

void GenericPMUApp::StopApplication() {
}

void GenericPMUApp::DoInitialize() {
	App::DoInitialize();
}

void GenericPMUApp::DoDispose() {
	App::DoDispose();
}

void GenericPMUApp::setPmuName(string pmuName) {
	this->pmuName = pmuName;
}

void GenericPMUApp::setFreq(int freq) {
	piat = MicroSeconds(1000000.0 / (double) freq);
}

void GenericPMUApp::OnInterest(std::shared_ptr<const Interest> interest) {
	cout << "GenericPMUApp: receive Interest packet with the nonce of " << interest->getNonce() << endl;
	App::OnInterest(interest);
	string prefix = interest->getName().toUri(name::UriFormat::DEFAULT);
	sendData();
}

void GenericPMUApp::sendData() {
	stringstream ss;
	ss << pmuName << "/Data/" << dataId;
	auto data = std::make_shared<Data>(string(ss.str()));
	data->setFreshnessPeriod(::ndn::time::milliseconds(1000));
	data->setContent(std::make_shared< ::ndn::Buffer>(1024));
	StackHelper::getKeyChain().sign(*data);
	m_transmittedDatas(data, this, m_face);
	m_appLink->onReceiveData(*data);

	GenericLog::TimeSendingData[dataId] = Simulator::Now();
	GenericLog::dataID_sent.push_back(dataId);

	cout << "GenericPMUApp: send data " << ss.str() << endl;
	++dataId;

	Simulator::Schedule(piat, &GenericPMUApp::sendData, this);
}


void GenericPMUApp::OnData(std::shared_ptr<const Data> data) {
	std::cout << "--> DATA received at PMU with the name of " << data->getName().toUri(name::UriFormat::DEFAULT) << std::endl;
}

}

