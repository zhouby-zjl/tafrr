/* *********************************************************************************
This work is licensed under CC BY-NC-SA 4.0
(https://creativecommons.org/licenses/by-nc-sa/4.0/).

Copyright (c) 2022 zhouby-zjl @ github

This file is a part of "Traffic Aware Fast Reroute Mechanism (TA-FRR)"
(https://github.com/zhouby-zjl/drtp/).

Written by zhouby-zjl @ github

This software is protected by the patents as well as the software copyrights.
 ***********************************************************************************/

#include "lltc-strategy-common.hpp"

namespace nfd {
namespace fw {

// ================= Link Layer: Measurement & Control (Also Contained in Service Layer) ==================
void LltcStrategy::onReceiveEcho(const FaceEndpoint& ingress, const Data& data) {
	EchoUri eru = LltcMessagesHelper::parseEchoUri(LltcStrategyConfig::lltcPrefix, data.getName());

	shared_ptr<Data> newData = LltcMessagesHelper::constructBack(LltcStrategyConfig::lltcPrefix, eru.echoId,
									forwardingStateColl->localNodeId, eru.sourceNodeId);
	this->sendLltcNonPitData(*newData, ingress);

	ofstream* logLinkTestMsgs = LltcLog::getLinkTestMsgs();
	*logLinkTestMsgs <<  Simulator::Now()  << "," << forwardingStateColl->localNodeId << ",Back," << eru.echoId << "," << eru.sourceNodeId << endl;
}

void LltcStrategy::onReceiveBack(const FaceEndpoint& ingress, const Data& data) {
	BackUri ebu = LltcMessagesHelper::parseBackUri(LltcStrategyConfig::lltcPrefix, data.getName());

	if (ebu.echoId == 3711032277) {
		cout << "here" << endl;
	}

	if (linkMeasurementState.echoMsgStateMap.find(ebu.echoId) != linkMeasurementState.echoMsgStateMap.end()) {
		EchoMessageState& ems = linkMeasurementState.echoMsgStateMap[ebu.echoId];
		ems.rtt = Simulator::Now() - linkMeasurementState.echoMsgStateMap[ebu.echoId].requestedTime;
		ems.status = ECHO_MSG_STATUS_NORMAL;
	}

}

void LltcStrategy::onReceiveLsa(const FaceEndpoint& ingress, const Data& data) {
	LsaUri flu = LltcMessagesHelper::parseLsaUri(LltcStrategyConfig::lltcPrefix, data.getName());

	if (linkMeasurementState.recentLsaIds.find(flu.lsaId) != linkMeasurementState.recentLsaIds.end()) {
		return;
	} else {
		linkMeasurementState.recentLsaIds.insert(flu.lsaId);
	}

	for (list<LltcFaceEntry*>::const_iterator iter = forwardingStateColl->faceEntries.begin(); iter != forwardingStateColl->faceEntries.end(); ++iter) {
		LltcFaceEntry* fa = *iter;
		this->sendNonPitData(data, FaceEndpoint(*fa->face_nextHop, 0));
	}

	list<LinkState> lsList;
	LltcMessagesHelper::extractLsa(data, &lsList);

	cout << "LSA UPATE received for ID: " << flu.lsaId << ", at node ID: " << forwardingStateColl->localNodeId << endl;

	updatePreferredPathWithLSA(flu.sourceNodeId, lsList, flu.type.compare("active") ==  0 ? true : false);
}


void LltcStrategy::checkLinkConnectivityInPeriodic() {
	link_test_id nextLinkTestID = linkMeasurementState.nextLinkTestID++;
	linkMeasurementState.EchoIDsInLinkTest[nextLinkTestID];
	runLinkTestsAndDoUpdate(LltcStrategyConfig::timesForCheckPathConnectivity, nextLinkTestID, true, 0, false);

	ns3::Simulator::Schedule(Seconds(LltcStrategyConfig::waitingTimeForCheckLinkConnectivityInPerioid),
						&LltcStrategy::checkLinkConnectivityInPeriodic, this);
}

void LltcStrategy::doActiveLinkConnectivityCheck(const FaceEndpoint& ingress) {
	int period = (int) LltcConfig::LLTC_ENABLE_FAILOVER_ACTIVE_CHECK_THROUGHPUT_PERIOD_SECS;

	for (list<LltcFaceEntry*>::iterator iter = this->forwardingStateColl->faceEntries.begin();
			iter != this->forwardingStateColl->faceEntries.end(); ++iter) {
		LltcFaceEntry* fe = *iter;
		FaceId faceId = fe->face_nextHop->getId();
		if (faceId == ingress.face.getId()) {
			int cur_secs = floor(Simulator::Now().GetSeconds());
			if (fe->last_count_secs == -1) {
				fe->last_count_secs = cur_secs;
				fe->init_secs = floor(Simulator::Now().GetSeconds());
				fe->num_received_Capsules_in_recent_secs.push_back(1);
			} else if (fe->last_count_secs == cur_secs) {
				++fe->num_received_Capsules_in_recent_secs[cur_secs - fe->init_secs];
			} else if (fe->last_count_secs < cur_secs) { // other conditions
				for (int skip_secs = fe->last_count_secs + 1; skip_secs <= cur_secs; ++skip_secs) {
					fe->num_received_Capsules_in_recent_secs.push_back(0);
				}
				++fe->num_received_Capsules_in_recent_secs[cur_secs - fe->init_secs];
			}
			fe->last_count_secs = cur_secs;

			int n = fe->num_received_Capsules_in_recent_secs.size();
			if (n >= period + 1) {
				int total = 0;
				for (int m = n - 2; m >= n - period - 1; --m) {
					total += fe->num_received_Capsules_in_recent_secs[m];
				}
				double throughput = (double) total / (double) period;
				double confRatio = LltcConfig::LLTC_ENABLE_FAILOVER_ACTIVE_CHECK_INTER_ARRIVAL_MAX_CONFIDENT_RATIO;
				double timeout = -log(1.0 - confRatio) /  throughput;

				if (this->linkMeasurementState.waitCapForActiveFailoverEvents.find(faceId) !=
						this->linkMeasurementState.waitCapForActiveFailoverEvents.end()) {
					Simulator::Remove(this->linkMeasurementState.waitCapForActiveFailoverEvents[faceId]->e);
					this->linkMeasurementState.waitCapForActiveFailoverEvents.erase(faceId);
				}

				WaitCapsuleForActiveFailoverEvent* event = new WaitCapsuleForActiveFailoverEvent;
				event->e = Simulator::Schedule(ns3::Seconds(timeout),
						&LltcStrategy::checkLinkConnectivityInActiveManner, this, faceId);
				event->waitTime = Simulator::Now();
				this->linkMeasurementState.waitCapForActiveFailoverEvents[faceId] = event;
			}
		}
	}
}


void LltcStrategy::checkLinkConnectivityInActiveManner(FaceId faceID) {
	cout << "DO ACTIVE LINK CONNECTIVITY CHECK at time: " << Simulator::Now() << ", @ Node ID: " <<
			this->forwardingStateColl->localNodeId << endl;

	ofstream* logLinkFailoversActiveChecks = LltcLog::getLogFailoversActiveChecks();
	*logLinkFailoversActiveChecks << Simulator::Now() << "," << this->forwardingStateColl->localNodeId << endl;

	link_test_id linkTestID = linkMeasurementState.nextLinkTestID++;
	linkMeasurementState.EchoIDsInLinkTest[linkTestID];
	runLinkTestsAndDoUpdate(LltcStrategyConfig::timesForCheckPathConnectivity, linkTestID, false, faceID, true);
}


void LltcStrategy::runLinkTestsAndDoUpdate(int _timesForCheckPathQoS, link_test_id linkTestID,
											bool isFull, FaceId faceID, bool isActive) {
	if (_timesForCheckPathQoS == 0) {
		updateFaceStatsWithLinkTestStates(linkTestID);
		broadcastingLsaTask(isActive);
		return;
	}
	this->broadcastEchos(linkTestID, isFull, faceID);
	ns3::Simulator::Schedule(Seconds(LltcStrategyConfig::intervalTimeForCheckPathQos),
					&LltcStrategy::runLinkTestsAndDoUpdate, this, _timesForCheckPathQoS - 1, linkTestID,
					isFull, faceID, isActive);
}

void LltcStrategy::updateFaceStatsWithLinkTestStates(link_test_id linkTestID) {
	unordered_map<LltcFaceEntry*, FaceStatusInternal> faceStatusMap;
	vector<echo_id>& echoIDs = linkMeasurementState.EchoIDsInLinkTest[linkTestID];

	for (echo_id echoID : echoIDs) {
		EchoMessageState& ems = linkMeasurementState.echoMsgStateMap[echoID];
		if (faceStatusMap.find(ems.fe) == faceStatusMap.end()) {
			FaceStatusInternal& fsi = faceStatusMap[ems.fe];
			fsi.nBacks = 0;
			if (ems.status == ECHO_MSG_STATUS_NORMAL) {
				fsi.faceStatus = FACE_STATUS_NORMAL;
				++fsi.nBacks;
			} else {
				fsi.faceStatus = FACE_STATUS_FAILURE;
			}
		} else if (ems.status == ECHO_MSG_STATUS_NORMAL) {
			FaceStatusInternal& fsi = faceStatusMap[ems.fe];
			fsi.faceStatus = FACE_STATUS_NORMAL;
			++fsi.nBacks;
		}

		linkMeasurementState.echoMsgStateMap.erase(echoID);
	}

	linkMeasurementState.EchoIDsInLinkTest.erase(linkTestID);

	for (LltcFaceEntry* fe : forwardingStateColl->faceEntries) {
		if (fe->remoteLltcNodeId == -1) continue;
		if (faceStatusMap.find(fe) != faceStatusMap.end()) {
			FaceStatusInternal& fsi = faceStatusMap[fe];
			fe->status = fsi.faceStatus;
			fe->lastCheckTime = Simulator::Now();
		}
	}
}

void LltcStrategy::broadcastEchos(link_test_id linkTestID, bool isFull, FaceId faceID) {
	Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();

	vector<echo_id>& echoIDs = linkMeasurementState.EchoIDsInLinkTest[linkTestID];
	EchoMessageState es;
	for (list<LltcFaceEntry*>::const_iterator iterFa = forwardingStateColl->faceEntries.begin(); iterFa != forwardingStateColl->faceEntries.end(); ++iterFa) {
		LltcFaceEntry* fe = *iterFa;
		if (fe->remoteLltcNodeId == -1) continue;

		if (!isFull && fe->face_nextHop->getId() != faceID) {
			continue;
		}

		echo_id echoId = rand->GetInteger(0, UINT_MAX);
		shared_ptr<Data> newData = LltcMessagesHelper::constructEcho(LltcStrategyConfig::lltcPrefix, echoId, forwardingStateColl->localNodeId);
		this->sendLltcNonPitData(*newData, FaceEndpoint(*fe->face_nextHop, 0));

		es.requestedTime = Simulator::Now();
		es.fe = fe;
		es.status = ECHO_MSG_STATUS_WAITING;
		linkMeasurementState.echoMsgStateMap[echoId] = es;
		echoIDs.push_back(echoId);

		ofstream* logLinkTestMsgs = LltcLog::getLinkTestMsgs();
		*logLinkTestMsgs <<  Simulator::Now()  << "," << forwardingStateColl->localNodeId << ",Echo," << echoId << "," <<  forwardingStateColl->localNodeId << endl;
	}
}


void LltcStrategy::broadcastingLsaTask(bool isActive) {
	shared_ptr<list<LinkState>> lsList = make_shared<list<LinkState>>();
	LinkState ls;
	bool diff = false;
	for (list<LltcFaceEntry*>::const_iterator iter = forwardingStateColl->faceEntries.begin(); iter != forwardingStateColl->faceEntries.end(); ++iter) {
		LltcFaceEntry* fa = *iter;
		if (fa->remoteLltcNodeId == -1) continue;

		ls.neighboredNodeId = fa->remoteLltcNodeId;
		ls.status = fa->status;

		if (!diff && linkMeasurementState.prevLsa != nullptr) {
			diff = false; bool found = false;
			for (list<LinkState>::const_iterator iterPrev = linkMeasurementState.prevLsa->begin(); iterPrev != linkMeasurementState.prevLsa->end(); ++iterPrev) {
				if ((*iterPrev).neighboredNodeId == ls.neighboredNodeId) {
					found = true;
					if ((*iterPrev).status != ls.status) {
						diff = true;
					}
					break;
				}
			}
			if (!found) diff = true;
		} else if (!diff && linkMeasurementState.prevLsa == nullptr && fa->status == FACE_STATUS_FAILURE) {
			diff = true;
		}
		lsList->push_back(ls);
	}

	if (!diff) return;

	updatePreferredPathWithLSA(forwardingStateColl->localNodeId, *lsList, isActive);

	linkMeasurementState.prevLsa = lsList;

	Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
	uint32_t lsaId = rand->GetInteger(0, UINT_MAX);

	shared_ptr<Data> data = LltcMessagesHelper::constructLsa(LltcStrategyConfig::lltcPrefix, lsaId,
												forwardingStateColl->localNodeId,
												linkMeasurementState.lsaUpdateSeqNo, &(*lsList),
												(isActive ? "active" : "period"));
	++linkMeasurementState.lsaUpdateSeqNo;

	/*ofstream* log_failovers = LltcLog::getLogFailovers();
	*log_failovers << Simulator::Now() << ",broadcastingLsaTask,@" << forwardingStateColl->localNodeId << ","
			<< lsaId << "," << linkMeasurementState.lsaUpdateSeqNo << ",lsList: ";
	for (LinkState& ls : *lsList) {
		*log_failovers << "(" << ls.neighboredNodeId << "," << ls.status << ") ";
	}
	*log_failovers << endl;*/

	for (list<LltcFaceEntry*>::const_iterator iter = forwardingStateColl->faceEntries.begin(); iter != forwardingStateColl->faceEntries.end(); ++iter) {
		LltcFaceEntry* fa = *iter;
		this->sendNonPitData(*data, FaceEndpoint(*fa->face_nextHop, 0));

		ofstream* logLinkTestMsgs = LltcLog::getLinkTestMsgs();
		*logLinkTestMsgs <<  Simulator::Now()  << "," << forwardingStateColl->localNodeId << ",Lsa," << lsaId << "," <<
				forwardingStateColl->localNodeId << "," << linkMeasurementState.lsaUpdateSeqNo << ",(";

		for (LinkState& ls : *lsList) {
			*logLinkTestMsgs << ls.neighboredNodeId << "|" << ls.status << "#";
		}

		*logLinkTestMsgs << ")" << endl;
	}
}


void LltcStrategy::updatePreferredPathWithLSA(int32_t sourceNodeId, list<LinkState>& lsList, bool isActive) {
	if (isActive) {
		cout << "here" << endl;
	}

	for (unordered_map<rsg_id, ForwardingStateForRSG>::iterator iter = forwardingStateColl->fssMap.begin(); iter != forwardingStateColl->fssMap.end(); ++iter) {
		ForwardingStateForRSG& linkLayerForwardingState = iter->second;

		bool hasRetransPoint = false;
		for (auto fe : linkLayerForwardingState.fibEntries) {
			if (fe->isRetransPoint) {
				hasRetransPoint = true;
				break;
			}
		}
		if (!hasRetransPoint) return;

		for (LinkState ls: lsList) {
			rsg_vir_link_status virLinkStatus = ls.status == FACE_STATUS_FAILURE ? RSG_VIR_LINK_STATUS_FAILURE : RSG_VIR_LINK_STATUS_NORMAL;
			if (virLinkStatus == RSG_VIR_LINK_STATUS_FAILURE) {
				cout << "here" << endl;
			}
			LltcRrSubgraph::rsgUpdateLinkStatus(&linkLayerForwardingState.rsg, sourceNodeId, ls.neighboredNodeId, virLinkStatus);
		}

		preferred_link preferredPaths = LltcRrSubgraph::rsgComputePreferredPathID(&linkLayerForwardingState.rsg, forwardingStateColl->localNodeId);
		cout << "rsgComputePreferredPathID, downLinkID: " << preferredPaths.downLinkID << ", upLinkID: " << preferredPaths.upLinkID << " @ Node ID: " << forwardingStateColl->localNodeId << endl;

		ofstream* log_failovers = LltcLog::getLogFailovers();
		string isActiveStr = isActive ? "ACTIVE" : "PERIOD";
		if (preferredPaths.downLinkID != UINT_MAX && preferredPaths.downLinkID != linkLayerForwardingState.preferredDownPathId) {
			linkLayerForwardingState.preferredDownPathId = preferredPaths.downLinkID;
			linkLayerForwardingState.isDownPathInRspState = (preferredPaths.downLinkID != linkLayerForwardingState.primaryPathId);
			cout << isActiveStr << ", PERFORM CHANGE ROUTE AT NODE ID: " << forwardingStateColl->localNodeId << " WITH PREFERRED DOWNSTREAM PATH ID: " << preferredPaths.downLinkID << " FOR RSG ID: " << iter->first << endl;
			*log_failovers << Simulator::Now() << "," << isActiveStr << "," << forwardingStateColl->localNodeId <<
					", PERFORM CHANGE ROUTE AT NODE ID: " << forwardingStateColl->localNodeId
					<< " WITH PREFERRED DOWNSTREAM PATH ID: " << preferredPaths.downLinkID << " FOR RSG ID: "
					<< iter->first << endl;
		}

		if (preferredPaths.upLinkID != UINT_MAX && preferredPaths.upLinkID != linkLayerForwardingState.preferredUpPathId) {
			linkLayerForwardingState.preferredUpPathId = preferredPaths.upLinkID;
			linkLayerForwardingState.isUpPathInRspState = (preferredPaths.upLinkID != linkLayerForwardingState.primaryPathId);
			cout << isActiveStr << ", PERFORM CHANGE ROUTE AT NODE ID: " << forwardingStateColl->localNodeId << " WITH PREFERRED UPSTREAM PATH ID: " << preferredPaths.upLinkID << " FOR RSG ID: " << iter->first << endl;
			*log_failovers << Simulator::Now() << "," << isActiveStr << "," << forwardingStateColl->localNodeId <<
					", PERFORM CHANGE ROUTE AT NODE ID: " << forwardingStateColl->localNodeId <<
					" WITH PREFERRED UPSTREAM PATH ID: " << preferredPaths.upLinkID << " FOR RSG ID: " <<
					iter->first << endl;
		}
	}

}

// =======================================================================================================
}
}
