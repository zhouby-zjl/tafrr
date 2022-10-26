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

#include "lltc-utils.hpp"

#include <string>

unordered_map<uint32_t, ofstream*> LltcLog::logsInMsgs;
unordered_map<uint32_t, ofstream*> LltcLog::logsOutMsgs;
string* LltcLog::logDirPath = NULL;
ofstream* LltcLog::logControlPlane = NULL;
ofstream* LltcLog::logFailovers = NULL;
ofstream* LltcLog::logFailureEvents = NULL;
ofstream* LltcLog::logFailoversActiveChecks = NULL;
ofstream* LltcLog::logLinkTestMsgs = NULL;
ofstream* LltcLog::logRoutesDump = NULL;
ofstream* LltcLog::logRrLossRatesEvaluation = NULL;
ofstream* LltcLog::logRrDelaysEvaluation = NULL;
ofstream* LltcLog::logRetranEvents = NULL;
ofstream* LltcLog::logNumRetranPaths = NULL;

list<retran_event>* LltcLog::retranEvents = NULL;
list<num_retran_paths>* LltcLog::numRetranPaths = NULL;

using namespace nfd::fw;
using namespace ns3;

void LltcLog::setLogDirPath(const char* logDirPath) {
	LltcLog::logDirPath = new string(logDirPath);
}

void LltcLog::openLogs(int nRouters) {
	stringstream ss;
	for (size_t i = 0; i < (size_t) nRouters; ++i) {
		uint32_t routerID = i;
		logsInMsgs[routerID] = new ofstream;
		logsOutMsgs[routerID] = new ofstream;
		if (LltcConfig::ENABLE_LOG_IN_OUT_MSGS) {
			ss.str("");
			ss << *logDirPath << "log-router-in-msgs-" << routerID;
			logsInMsgs[routerID]->open(ss.str(), ios::trunc);
			ss.str("");
			ss << *logDirPath << "log-router-out-msgs-" << routerID;
			logsOutMsgs[routerID]->open(ss.str(), ios::trunc);
		}
	}

	ss.str("");
	ss << *logDirPath << "log-control-plane";
	logControlPlane = new ofstream;
	logControlPlane->open(ss.str(), ios::trunc);

	ss.str("");
	ss << *logDirPath << "log-failovers";
	logFailovers = new ofstream;
	logFailovers->open(ss.str(), ios::trunc);

	ss.str("");
	ss << *logDirPath << "log-failure-events";
	logFailureEvents = new ofstream;
	logFailureEvents->open(ss.str(), ios::trunc);

	ss.str("");
	ss << *logDirPath << "log-failovers-active-checks";
	logFailoversActiveChecks = new ofstream;
	logFailoversActiveChecks->open(ss.str(), ios::trunc);

	ss.str("");
	ss << *logDirPath << "log-link-tests-msgs";
	logLinkTestMsgs = new ofstream;
	logLinkTestMsgs->open(ss.str(), ios::trunc);

	ss.str("");
	ss << *logDirPath << "log-routes-dump";
	logRoutesDump = new ofstream;
	logRoutesDump->open(ss.str(), ios::trunc);

	ss.str("");
	ss << *logDirPath << "log-rr-lossrate-evaluation";
	logRrLossRatesEvaluation = new ofstream;
	logRrLossRatesEvaluation->open(ss.str(), ios::trunc);

	ss.str("");
	ss << *logDirPath << "log-rr-delay-evaluation";
	logRrDelaysEvaluation = new ofstream;
	logRrDelaysEvaluation->open(ss.str(), ios::trunc);

	ss.str("");
	ss << *logDirPath << "log-retran-events";
	logRetranEvents = new ofstream;
	logRetranEvents->open(ss.str(), ios::trunc);

	ss.str("");
	ss << *logDirPath << "log-num-retran-paths";
	logNumRetranPaths = new ofstream;
	logNumRetranPaths->open(ss.str(), ios::trunc);
}

ofstream* LltcLog::getLogOutput(uint32_t routerID, int logType) {
	switch (logType) {
	case LOG_IN_MSGS:
		if (logsInMsgs.find(routerID) == logsInMsgs.end()) return NULL;
		return logsInMsgs[routerID];
		break;

	case LOG_OUT_MSGS:
		if (logsOutMsgs.find(routerID) == logsOutMsgs.end()) return NULL;
		return logsOutMsgs[routerID];
		break;
	}

	return NULL;
}

list<retran_event>* LltcLog::getRetranEvents() {
	if (retranEvents == NULL) {
		retranEvents = new list<retran_event>();
	}
	return retranEvents;
}

list<num_retran_paths>* LltcLog::getNumRetranPaths() {
	if (numRetranPaths == NULL) {
		numRetranPaths = new list<num_retran_paths>();
	}
	return numRetranPaths;
}

ofstream* LltcLog::getLogRetranEvents() {
	return logRetranEvents;
}

ofstream* LltcLog::getLogNumRetranPaths() {
	return logNumRetranPaths;
}

ofstream* LltcLog::getLogControlPlane() {
	return logControlPlane;
}

ofstream* LltcLog::getLogFailovers() {
	return logFailovers;
}

ofstream* LltcLog::getLogFailureEvents() {
	return logFailureEvents;
}

ofstream* LltcLog::getLogFailoversActiveChecks() {
	return logFailoversActiveChecks;
}

ofstream* LltcLog::getLinkTestMsgs() {
	return logLinkTestMsgs;
}

ofstream* LltcLog::getLogRoutesDump() {
	return logRoutesDump;
}

ofstream* LltcLog::getLogRrLossRatesEvaluation() {
	return logRrLossRatesEvaluation;
}

ofstream* LltcLog::getLogRrDelaysEvaluation() {
	return logRrDelaysEvaluation;
}

void LltcLog::flushRouterLogs(uint32_t routerID) {
	if (logsInMsgs.find(routerID) != logsInMsgs.end()) {
		logsInMsgs[routerID]->flush();
	}

	if (logsOutMsgs.find(routerID) != logsOutMsgs.end()) {
		logsOutMsgs[routerID]->flush();
	}
}

void LltcLog::closeLogs() {
	for (unordered_map<uint32_t, ofstream*>::iterator iter = logsInMsgs.begin(); iter != logsInMsgs.end(); ++iter) {
		logsInMsgs[iter->first]->close();
	}
	for (unordered_map<uint32_t, ofstream*>::iterator iter = logsOutMsgs.begin(); iter != logsOutMsgs.end(); ++iter) {
		logsOutMsgs[iter->first]->close();
	}

	logControlPlane->close();
	logLinkTestMsgs->close();
	logFailovers->close();
	logFailureEvents->close();
	logFailoversActiveChecks->close();
}

void LltcLog::cleanLogDir() {
	stringstream ss;
	ss << "rm -rf " << *logDirPath << "log-*";
	system(ss.str().c_str());
}


string LltcRoutesDumper::dumpPath(Path* path) {
	stringstream ss;
	size_t nNodes = path->nodeIds->size();
	for (size_t i = 0; i < nNodes; ++i) {
		ss << (*path->nodeIds) [i];
		if (i <= nNodes - 2) {
			ss << ".";
		}
	}
	return ss.str();
}

string LltcRoutesDumper::dumpRR(ResilientRoutes* rr) {
	stringstream ss;
	int m = 0;
	for (pair<int, vector<RedundantSubPath*>*> rspItem : *rr->getHRSP()) {
		if (rspItem.second->size() > 0) ++m;
	}

	int u = 0;
	for (pair<int, vector<RedundantSubPath*>*> rspItem : *rr->getHRSP()) {
		vector<RedundantSubPath*>* rsps = rspItem.second;
		if (rsps->size() == 0) continue;
		ss << rspItem.first << ":";
		for (size_t i = 0; i < rsps->size(); ++i) {
			RedundantSubPath* rsp = (*rsps) [i];
			ss << rsp->dataDeliveryLossRate << "|" << rsp->linkDownDelay << "|" << rsp->linkUpDelay <<
					"|" << rsp->retransDelayInMultiTimes << "|" << rsp->oneWayDataDeliveryLossRate << "#";
			for (size_t p = 0; p < rsp->path->nodeIds->size(); ++p) {
				ss << (*rsp->path->nodeIds)[p];
				if (p <= rsp->path->nodeIds->size() - 2) {
					ss << ".";
				}
			}
			if (i <= rsps->size() - 2) {
				ss << ";";
			}
		}
		if (u < m - 1) {
			ss << "&";
		}
		++u;
	}
	return ss.str();
}


string LltcRoutesDumper::dumpMsg(bool isPIT, bool isRSG, uint32_t rsgId, uint32_t pathId, const FaceEndpoint& port, const Data& data) {
	stringstream ss;
	ss << Simulator::Now();
	ss << (isPIT ? ",PIT" : ",NonPIT");
	ss << (isRSG ? ",RSG," : ",NonRSG,");
	string dataNameStr = data.getName().toUri(name::UriFormat::DEFAULT);

	ss << rsgId << "," << pathId << "," << port.face.getRemoteUri().toString()
					<< "," << data.getName().toUri(name::UriFormat::DEFAULT);


	return ss.str();
}
