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

#ifndef SRC_NDNSIM_MODEL_LLTC_LLTC_CONFIG_HPP_
#define SRC_NDNSIM_MODEL_LLTC_LLTC_CONFIG_HPP_

#include <string>

using namespace std;

#define FAULT_SIM_TYPE_ERR_NONE				  	0
#define FAULT_SIM_TYPE_ERR_FOR_FULL_RR        	1
#define FAULT_SIM_TYPE_ERR_FOR_FULL_PP        	2
#define FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK   3
#define FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS   4


class LltcConfig {
public:
	static bool loadConfigFile(string filePath);

	static string NETWORK_TOPO_FILE;
	static string NETWORK_TOPO_FILE_TYPE;
	static int NETWORK_NUM_SUBSTATIONS;
	static int NETWORK_NODE_ID_PDC;
	static int NETWORK_NODE_ID_PMU_STATIC;
	static int NETWORK_NUM_PMUS;
	static int NETWORK_PMU_DATA_FREQ;
	static string NETWORK_LINK_BAND;
	static string NETWORK_LINK_QUEUE_SIZE;
	static int NETWORK_NODE_PROCESS_DELAY_US;
	static int NETWORK_LINK_QUEUE_DELAY_US;
	static int NETWORK_LINK_TRANS_DELAY_US;

	static int FAULT_SIM_TYPE;
	static double FAULT_SIM_TYPE_ERR_FOR_FULL_RR_ERR_RATE;
	static double FAULT_SIM_TYPE_ERR_FOR_FULL_RR_START_TIME;
	static double FAULT_SIM_TYPE_ERR_FOR_FULL_PP_ERR_RATE;
	static double FAULT_SIM_TYPE_ERR_FOR_FULL_PP_START_TIME;
	static double FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK_START_TIME;
	static double FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK_ERR_RATE;
	static int FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK_ROUTER_1;
	static int FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK_ROUTER_2;
	static double FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_ERR_RATE;
	static double FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_START_TIME;
	static int FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_N_RAND_DISRUPTED_PP_LINKS;
	static int FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_N_RAND_DISRUPTED_NON_PP_LINKS;
	static double FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_FLAP_PERIOD;
	static int FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_FLAP_TIMES;

	static string SIM_LOG_DIR;
	static string SIM_LOG_DIR_CP_TESTS;
	static bool ENABLE_LOG_IN_OUT_MSGS;
	static double SIM_TIME_SECS;
	static double SIM_PDC_START_TIME_SECS;
	static double SIM_PMU_START_TIMES_SECS;

	static string LLTC_PREFIX;
	static double LLTC_ASSUMED_LINK_RELIA_RATE;
	static int LLTC_BETA;
	static double LLTC_REROUTE_PATH_GEN_ALPHA;
	static int LLTC_MAX_PATH_DELAY_US;
	static double LLTC_MAX_DRIFT_RANGE_RATIO_FOR_PMU_FREQ;
	static int LLTC_MAX_CONSECUTIVE_DRIFT_PACKETS;
	static int LLTC_NUM_DATA_RETRANS_REPORTS_TO_SEND;
	static int LLTC_NUM_RETRANS_REQUESTS;
	static int LLTC_QUEUE_SIZE_FOR_TRANSMITTED_DATA_IDS;
	static int LLTC_CS_LIMITS_IN_ENTRIES;
	static int LLTC_TIMES_FOR_CHECK_PATH_CONNECTIVITY;
	static double LLTC_PERIOD_FOR_CHECK_LINK_CONNECTIVITY_SECS;
	static double LLTC_INITIAL_DELAY_FOR_CHECK_LINK_CONNECTIVITY_SECS;
	static double LLTC_INTERVAL_TIME_FOR_CHECK_PATH_QOS_SECS;
	static bool LLTC_ENABLE_FAILOVER;
	static bool LLTC_ENABLE_FAILOVER_ACTIVE_CHECK;
	static double LLTC_ENABLE_FAILOVER_ACTIVE_CHECK_THROUGHPUT_PERIOD_SECS;
	static double LLTC_ENABLE_FAILOVER_ACTIVE_CHECK_INTER_ARRIVAL_MAX_CONFIDENT_RATIO;
	static int LLTC_PDC_RESEQNEUCE_RANGE;
	static double LLTC_PIT_DURATION_MS;
	static bool LLTC_DISABLE_RETRAN;

	static bool ENABLE_PMU_EXP_RAND;

};

#endif /* SRC_NDNSIM_MODEL_LLTC_LLTC_CONFIG_HPP_ */
