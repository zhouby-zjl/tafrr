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

#ifndef SRC_NDNSIM_NFD_DAEMON_FW_LLTC_STRATEGY_CONFIG_HPP_
#define SRC_NDNSIM_NFD_DAEMON_FW_LLTC_STRATEGY_CONFIG_HPP_

#include "ns3/ndnSIM/model/ndn-common.hpp"

using namespace nfd;
using namespace std;

namespace nfd {
namespace fw {


class LltcStrategyConfig {
public:
	// ================================== Strategy Configurations ===========================================
    static const Name& getStrategyName();
	static void setIntervalTimeForCheckPathQos(double secs);
	static void setWaitingTimeForCheckLinkConnectivityInPerioid(double secs);
	static void setDelayTimeForRunningCheckLinkConnectivity(double secs);
	static void setQueueSizeForTransmittedDataIds(size_t n);
	static void setCsLimitInEntries(size_t n);
	static void setNumRetransRequests(int numRetransRequests);
	//static void setDataPiat(ns3::Time piat);
	static void setPmuFreq(int pmuFreq);
	static void setTimesForCheckPathConnectivity(int timesForCheckPathConnectivity);
	static void setLltcPrefix(const char* prefix);
	static void setMaxDriftRatioForPmuFreq(double maxDriftRatioForPmuFreq);
	static void setEnableFailover(bool enable);
	static void setNumDataRetransReportsToSend(int n);
	// ======================================================================================================

	// ============================== Strategy Configuration Variables =======================================
	static size_t 			queueSizeForTransmittedDataIds;
	static size_t 			csLimitInEntries;
	static int 				numRetransRequests;
	static int				pmuFreq;
	static int		 		dataPiatInUs;
	static double 			maxDriftRatioForPmuFreq;
	static double	 		intervalTimeForCheckPathQos;
	static double 			waitingTimeForCheckLinkConnectivityInPerioid;
	static double 			delayTimeForRunningCheckLinkConnectivity;
	static int 				timesForCheckPathConnectivity;
	static string 			lltcPrefix;
	static ns3::ndn::Name 	lltcPrefixName;
	static bool 			enableFailover;
	static int 				numDataRetransReportsToSend;
	// =======================================================================================================
};

}
}

#endif /* SRC_NDNSIM_NFD_DAEMON_FW_LLTC_STRATEGY_CONFIG_HPP_ */
