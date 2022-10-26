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

#include "lltc-pdc.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/lltc-fs.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/lltc-messages-helper.hpp"
#include "lltc-utils.hpp"

#include <string>

NS_LOG_COMPONENT_DEFINE("lltc.LltcPDCApp");

using namespace std;

namespace lltc {

NS_OBJECT_ENSURE_REGISTERED(LltcPDCApp);

TypeId LltcPDCApp::GetTypeId() {
	static TypeId tid = TypeId("lltc::LltcPDCApp")
					.SetGroupName("Lltc")
					.SetParent<App>().AddConstructor<LltcPDCApp>();
	return tid;
}

void LltcPDCApp::StartApplication() {
	App::StartApplication();
	LltcFsManager::registerNonPitApp(nodeId, &(*m_face));

	FibHelper::AddRoute(GetNode(), this->pdcName, m_face, 0);
	Simulator::Schedule(Seconds(1.0), &LltcPDCApp::SendInterest, this);

	std::cout << "LLTC PDC started" << std::endl;

	string _pdcName = pdcName;
	string_replace(_pdcName, "/", "-");
	stringstream ss;
	ss << LltcConfig::SIM_LOG_DIR << "log" << _pdcName;
	outLog_pdc.open(ss.str(), ios::trunc);

	ss.str("");
	ss << LltcConfig::SIM_LOG_DIR << "log-res" << _pdcName;
	outLog_pdc_resequenced.open(ss.str(), ios::trunc);

	ss.str("");
	ss << LltcConfig::SIM_LOG_DIR << "log-res-queue-size" << _pdcName;
	outLog_pdc_q_reseq_size.open(ss.str(), ios::trunc);
}

void LltcPDCApp::StopApplication() {
	outLog_pdc.flush();
	outLog_pdc_resequenced.flush();
	outLog_pdc_q_reseq_size.flush();
}

void LltcPDCApp::setPDCName(string pdcName) {
	this->pdcName = pdcName;
}

void LltcPDCApp::addPMU(uint32_t nodeID, std::string pmuName, ns3::Time piat, int nResequenceRangeInPackets) {
	PmuState* ps = new PmuState;
	ps->nodeID = nodeID;
	ps->pmuPrefix = pmuName;
	ps->nResequenceRangeInPackets = nResequenceRangeInPackets;
	ps->piat = piat;
	ns3::Time maxWaitTime = nResequenceRangeInPackets * piat;
	ps->queue = new LltcResequenceQueue(pmuName, 2000, maxWaitTime);
	ps->queue->boundApp(this);

	this->pmuStateMap[pmuName] = ps;
}

void LltcPDCApp::DoInitialize() {
	App::DoInitialize();
}

void LltcPDCApp::DoDispose() {
	App::DoDispose();
}

void LltcPDCApp::OnInterest(std::shared_ptr<const Interest> interest) {
	App::OnInterest(interest);
}

void LltcPDCApp::OnData(std::shared_ptr<const Data> data) {
	std::cout << "--> DATA received at PDC for name " << data->getName().toUri(name::UriFormat::DEFAULT) << std::endl;

	CapsuleUri du;
	bool succ = false;
	PmuState* ps = NULL;
	for (pair<string, PmuState*> pmuState : pmuStateMap) {
		succ = this->parseDataUri(data->getName(), pmuState.first, du);
		ps = pmuState.second;
		if (succ) break;
	}
	if (!succ) return;

	LltcResequenceQueue* q = ps->queue;
	q->receiveData(du, data);
	outLog_pdc_q_reseq_size << Simulator::Now() << "," << q->getQueueSize() << endl;

	outLog_pdc << Simulator::Now() << "," << du.prefix << "," << du.dataId << "," << du.isRetrans << "," << du.requestedDataId << endl;
	cout << "outLog_pdc: " << Simulator::Now() << "," << du.prefix << "," << du.dataId << "," << du.isRetrans << "," << du.requestedDataId << endl;
}

void LltcPDCApp::SendInterest() {
	for (pair<string, PmuState*> pmuState : pmuStateMap) {
		auto interest = std::make_shared<Interest>(pmuState.first);
		Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
		interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
		interest->setInterestLifetime(time::seconds(1));

		m_transmittedInterests(interest, this, m_face);
		m_appLink->onReceiveInterest(*interest);

		cout << "sending Interest packet with name: " << pmuState.first << endl;
	}
}


void LltcPDCApp::setNodeID(uint32_t nodeID) {
	this->nodeId = nodeID;
}

bool LltcPDCApp::parseDataUri(const Name& dataName, const string& prefix, CapsuleUri& du) {
	string dataNameStr = dataName.toUri(name::UriFormat::DEFAULT);
	size_t prefixLen = prefix.size();

	if (dataNameStr.substr(0, prefix.size()).compare(prefix) != 0 ||
			(prefixLen + 9 >= dataNameStr.length()) ||
			dataNameStr.substr(prefixLen, 9).compare("/Capsule/") != 0) {
		return false;
	}

	Name prefixName(prefix);
	int nComponent = prefixName.size();
	du.dataId = stoll(dataName.get(nComponent + 1).toUri(name::UriFormat::DEFAULT));
	if (dataName.size() - nComponent == 4) {
		du.isRetrans = dataName.get(nComponent + 2).toUri(name::UriFormat::DEFAULT).compare("Retrans") == 0;
		if (du.isRetrans) {
			du.requestedDataId = (LltcDataID) stoull(dataName.get(nComponent + 3).toUri(name::UriFormat::DEFAULT));
		} else {
			du.requestedDataId = du.dataId;
		}
	} else {
		du.isRetrans = false;
		du.requestedDataId = du.dataId;
	}

	du.prefix = prefix;
	return true;
}

void LltcPDCApp::onDataArrives(string prefix, CapsuleUri du, shared_ptr<const Data> data) {
	std::cout << "==> DATA ID: " << du.dataId << ", " << du.prefix << "," << du.isRetrans << ", " << du.requestedDataId << endl;
	outLog_pdc_resequenced << Simulator::Now() << "," << du.dataId << "," << du.prefix << ", " << du.isRetrans << ", " << du.requestedDataId << endl;
}



LltcResequenceQueue::LltcResequenceQueue(string prefix, int queueSize, ns3::Time maxWaitTime) {
	this->size = queueSize;
	this->maxWaitTime = maxWaitTime;
	this->lastDataId = -1;
	this->prefix = prefix;
	this->app = NULL;
}

void LltcResequenceQueue::boundApp(LltcPDCApp* app) {
	this->app = app;
}

void LltcResequenceQueue::receiveData(CapsuleUri du, shared_ptr<const Data> data) {
	if (lastDataId == -1) {
		sendData(du, data);
		lastDataId = du.dataId;
		return;
	} else if (du.dataId == lastDataId + 1) {
		sendData(du, data);
		++lastDataId;
		return;
	}

	if (q_seq.size() == size) {
		DataElement _d = q_seq.top();
		q_seq.pop();
		sendData(_d.du, _d.data);
		lastDataId = _d.du.dataId;
	}

	ns3::Time curTime = Simulator::Now();
	DataElement de;
	de.arriveTime = curTime;
	de.data = data;
	de.du = du;
	q_seq.push(de);
	q_time.push(de);

	releaseQueue(curTime);

	if (q_seq.size() > 0) {
		if (curAutoDequeueTaskEvent.IsRunning()) {
			Simulator::Remove(curAutoDequeueTaskEvent);
		}

		DataElement _d = q_time.front();
		Simulator::Schedule(_d.arriveTime + maxWaitTime - curTime, &LltcResequenceQueue::autoDequeueTask, this, this);
	}
}

size_t LltcResequenceQueue::getQueueSize() {
	return q_seq.size();
}

void LltcResequenceQueue::releaseQueue(ns3::Time curTime) {
	while (q_seq.size() > 0) {
		DataElement _d = q_seq.top();
		if (_d.du.dataId == lastDataId + 1) {
			q_seq.pop();
			sendData(_d.du, _d.data);
			++lastDataId;
		} else {
			break;
		}
	}

	if (q_seq.size() == 0) return;

	DataElement d_oldest;
	int64_t dataId_max = -1;
	while (q_time.size() > 0) {
		d_oldest = q_time.front();
		if (d_oldest.arriveTime <= curTime - maxWaitTime) {
			q_time.pop();
			if (dataId_max < d_oldest.du.dataId) {
				dataId_max = d_oldest.du.dataId;
			}
		} else {
			break;
		}
	}
	if (dataId_max == -1) return;

	while (q_seq.size() > 0) {
		DataElement _d = q_seq.top();
		if (_d.du.dataId <= dataId_max) {
			q_seq.pop();
			sendData(_d.du, _d.data);
			lastDataId = _d.du.dataId;
		} else {
			break;
		}
	}
}

void LltcResequenceQueue::sendData(CapsuleUri du, shared_ptr<const Data> data) {
	if (this->app != NULL) {
		this->app->onDataArrives(prefix, du, data);
	}
}

void LltcResequenceQueue::autoDequeueTask(LltcResequenceQueue* rq) {
	rq->releaseQueue(Simulator::Now());
}


}
