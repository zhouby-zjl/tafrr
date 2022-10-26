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

#include "lltc-strategy-config.hpp"

namespace nfd {
namespace fw {

string LltcStrategyConfig::lltcPrefix = "/lltc";
ns3::ndn::Name LltcStrategyConfig::lltcPrefixName = "/lltc";
size_t LltcStrategyConfig::queueSizeForTransmittedDataIds = 1000;
size_t LltcStrategyConfig::csLimitInEntries = 1000;
double LltcStrategyConfig::intervalTimeForCheckPathQos = 3;
double LltcStrategyConfig::waitingTimeForCheckLinkConnectivityInPerioid = 10;
double LltcStrategyConfig::delayTimeForRunningCheckLinkConnectivity = 1;
int LltcStrategyConfig::numRetransRequests = 3;
int LltcStrategyConfig::pmuFreq = 50;
int LltcStrategyConfig::dataPiatInUs = 1000000 / 50;
double LltcStrategyConfig::maxDriftRatioForPmuFreq = 0.01;
int LltcStrategyConfig::timesForCheckPathConnectivity = 3;
bool LltcStrategyConfig::enableFailover = true;
int LltcStrategyConfig::numDataRetransReportsToSend = 3;

void LltcStrategyConfig::setNumDataRetransReportsToSend(int n) {
	LltcStrategyConfig::numDataRetransReportsToSend = n;
}

void LltcStrategyConfig::setEnableFailover(bool enable) {
	LltcStrategyConfig::enableFailover = enable;
}

void LltcStrategyConfig::setQueueSizeForTransmittedDataIds(size_t n) {
	LltcStrategyConfig::queueSizeForTransmittedDataIds = n;
}

void LltcStrategyConfig::setCsLimitInEntries(size_t n) {
	LltcStrategyConfig::csLimitInEntries = n;
}

void LltcStrategyConfig::setMaxDriftRatioForPmuFreq(double maxDriftRatioForPmuFreq) {
	LltcStrategyConfig::maxDriftRatioForPmuFreq = maxDriftRatioForPmuFreq;
}

void LltcStrategyConfig::setNumRetransRequests(int numRetransRequests) {
	LltcStrategyConfig::numRetransRequests = numRetransRequests;
}

//void LltcStrategyConfig::setDataPiat(ns3::Time piat) {
//	LltcStrategyConfig::dataPiat = piat;
//}

void LltcStrategyConfig::setPmuFreq(int pmuFreq) {
	LltcStrategyConfig::pmuFreq = pmuFreq;
	LltcStrategyConfig::dataPiatInUs = 1000000 / pmuFreq;
}

void LltcStrategyConfig::setTimesForCheckPathConnectivity(int timesForCheckPathConnectivity) {
	LltcStrategyConfig::timesForCheckPathConnectivity = timesForCheckPathConnectivity;
}

void LltcStrategyConfig::setWaitingTimeForCheckLinkConnectivityInPerioid(double secs) {
	LltcStrategyConfig::waitingTimeForCheckLinkConnectivityInPerioid = secs;
}

void LltcStrategyConfig::setDelayTimeForRunningCheckLinkConnectivity(double secs) {
	LltcStrategyConfig::delayTimeForRunningCheckLinkConnectivity = secs;
}

void LltcStrategyConfig::setIntervalTimeForCheckPathQos(double secs) {
	LltcStrategyConfig::intervalTimeForCheckPathQos = secs;
}

void LltcStrategyConfig::setLltcPrefix(const char* prefix) {
	lltcPrefix = string(prefix);
	lltcPrefixName = Name(prefix);
}

}
}
