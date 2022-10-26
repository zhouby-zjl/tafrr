#include "generic-pdc.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/generic-routes-manager.hpp"
#include "lltc-utils.hpp"

#include "ns3/ndnSIM/model/generic-log.hpp"

#include <string>

NS_LOG_COMPONENT_DEFINE("generic.GenericPDCApp");

using namespace std;

namespace generic {

NS_OBJECT_ENSURE_REGISTERED(GenericPDCApp);

TypeId GenericPDCApp::GetTypeId() {
	static TypeId tid = TypeId("generic::GenericPDCApp")
					.SetGroupName("Generic")
					.SetParent<App>().AddConstructor<GenericPDCApp>();
	return tid;
}

void GenericPDCApp::StartApplication() {
	App::StartApplication();

	GenericRoutesManager::registerAppFace(nodeID, m_face);
	FibHelper::AddRoute(GetNode(), this->pdcName, m_face, 0);
	Simulator::Schedule(Seconds(1.0), &GenericPDCApp::SendInterest, this);

	std::cout << "Generic PDC started" << std::endl;

	string _pdcName = pdcName;
	string_replace(_pdcName, "/", "-");
}

void GenericPDCApp::StopApplication() {
}

void GenericPDCApp::setPDCName(string pdcName) {
	this->pdcName = pdcName;
}

void GenericPDCApp::setPMU(std::string pmuName) {
	this->pmuName = pmuName;
}

void GenericPDCApp::setNodeID(uint32_t nodeID) {
	this->nodeID = nodeID;
}

void GenericPDCApp::DoInitialize() {
	App::DoInitialize();
}

void GenericPDCApp::DoDispose() {
	App::DoDispose();
}

void GenericPDCApp::OnInterest(std::shared_ptr<const Interest> interest) {
	App::OnInterest(interest);
}

void GenericPDCApp::OnData(std::shared_ptr<const Data> data) {
	std::cout << "--> DATA received at PDC for name " << data->getName().toUri(name::UriFormat::DEFAULT) << std::endl;
	Name dataName = data->getName();
	string dataIDStr = dataName.get(dataName.size() - 1).toUri(name::UriFormat::DEFAULT);
	uint32_t dataID = atoi(dataIDStr.c_str());
	GenericLog::TimeReceivedData[dataID] = Simulator::Now();
	GenericLog::dataID_recv.push_back(dataID);
}

void GenericPDCApp::SendInterest() {
	auto interest = std::make_shared<Interest>(pmuName);
	Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
	interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
	interest->setInterestLifetime(::ndn::time::seconds(1));

	m_transmittedInterests(interest, this, m_face);
	m_appLink->onReceiveInterest(*interest);

	cout << "sending Interest packet with name: " << pmuName << endl;
}

}
