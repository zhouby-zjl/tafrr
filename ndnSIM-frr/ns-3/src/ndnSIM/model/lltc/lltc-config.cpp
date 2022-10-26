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

#include "lltc-config.hpp"
#include <fstream>
#include <regex>
#include <stdlib.h>

using namespace std;

string LltcConfig::NETWORK_TOPO_FILE = "/home/zby/link-loss-comm/topo/ieee300cdf.txt";
string LltcConfig::NETWORK_TOPO_FILE_TYPE = "IEEE";     // IEEE or CSV
int LltcConfig::NETWORK_NUM_SUBSTATIONS = 300;
int LltcConfig::NETWORK_NODE_ID_PDC = 100;
int LltcConfig::NETWORK_NODE_ID_PMU_STATIC = 138;
int LltcConfig::NETWORK_NUM_PMUS = 1;
int LltcConfig::NETWORK_PMU_DATA_FREQ = 50;
string LltcConfig::NETWORK_LINK_BAND = "10Gbps";
string LltcConfig::NETWORK_LINK_QUEUE_SIZE = "100p";
int LltcConfig::NETWORK_NODE_PROCESS_DELAY_US = (1 * 2);
int LltcConfig::NETWORK_LINK_QUEUE_DELAY_US = (15 * 2);
int LltcConfig::NETWORK_LINK_TRANS_DELAY_US = 100;

int LltcConfig::FAULT_SIM_TYPE = FAULT_SIM_TYPE_ERR_NONE;
double LltcConfig::FAULT_SIM_TYPE_ERR_FOR_FULL_RR_ERR_RATE = 0.15;
double LltcConfig::FAULT_SIM_TYPE_ERR_FOR_FULL_RR_START_TIME = 3;
double LltcConfig::FAULT_SIM_TYPE_ERR_FOR_FULL_PP_ERR_RATE = 0.15;
double LltcConfig::FAULT_SIM_TYPE_ERR_FOR_FULL_PP_START_TIME = 3;
double LltcConfig::FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK_START_TIME = 3;
double LltcConfig::FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK_ERR_RATE = 0.15;
int LltcConfig::FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK_ROUTER_1 = 0;
int LltcConfig::FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK_ROUTER_2 = 1;
double LltcConfig::FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_ERR_RATE = 0.15;
double LltcConfig::FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_START_TIME = 3;
int LltcConfig::FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_N_RAND_DISRUPTED_PP_LINKS = 2;
int LltcConfig::FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_N_RAND_DISRUPTED_NON_PP_LINKS = 3;

double LltcConfig::FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_FLAP_PERIOD = 60.0;
int LltcConfig::FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_FLAP_TIMES = 1;

string LltcConfig::SIM_LOG_DIR = "/home/zby/lltc-log/";
string LltcConfig::SIM_LOG_DIR_CP_TESTS = "/home/zby/lltc-log-cp/";
bool LltcConfig::ENABLE_LOG_IN_OUT_MSGS = true;
double LltcConfig::SIM_TIME_SECS = 10.0;
double LltcConfig::SIM_PDC_START_TIME_SECS = 0.0;
double LltcConfig::SIM_PMU_START_TIMES_SECS = 0.0;

string LltcConfig::LLTC_PREFIX = "/lltc";
double LltcConfig::LLTC_ASSUMED_LINK_RELIA_RATE = 0.85;
int LltcConfig::LLTC_BETA = 5;
int LltcConfig::LLTC_MAX_PATH_DELAY_US = 35000;
double LltcConfig::LLTC_REROUTE_PATH_GEN_ALPHA = 0.5;
double LltcConfig::LLTC_MAX_DRIFT_RANGE_RATIO_FOR_PMU_FREQ = 0.01;
int LltcConfig::LLTC_MAX_CONSECUTIVE_DRIFT_PACKETS = 100;
int LltcConfig::LLTC_NUM_DATA_RETRANS_REPORTS_TO_SEND = 3;
int LltcConfig::LLTC_NUM_RETRANS_REQUESTS = 3;
int LltcConfig::LLTC_QUEUE_SIZE_FOR_TRANSMITTED_DATA_IDS = 10000;
int LltcConfig::LLTC_CS_LIMITS_IN_ENTRIES = 10000;

int LltcConfig::LLTC_TIMES_FOR_CHECK_PATH_CONNECTIVITY = 3;
double LltcConfig::LLTC_PERIOD_FOR_CHECK_LINK_CONNECTIVITY_SECS = 5.0;
double LltcConfig::LLTC_INITIAL_DELAY_FOR_CHECK_LINK_CONNECTIVITY_SECS = 1.0;
double LltcConfig::LLTC_INTERVAL_TIME_FOR_CHECK_PATH_QOS_SECS = 1.0;
bool LltcConfig::LLTC_ENABLE_FAILOVER  = true;
bool LltcConfig::LLTC_ENABLE_FAILOVER_ACTIVE_CHECK = true;
double LltcConfig::LLTC_ENABLE_FAILOVER_ACTIVE_CHECK_THROUGHPUT_PERIOD_SECS = 10.0;
double LltcConfig::LLTC_ENABLE_FAILOVER_ACTIVE_CHECK_INTER_ARRIVAL_MAX_CONFIDENT_RATIO = 0.95;

int LltcConfig::LLTC_PDC_RESEQNEUCE_RANGE = 20;
double LltcConfig::LLTC_PIT_DURATION_MS = 100000;

bool LltcConfig::LLTC_DISABLE_RETRAN = false;
bool LltcConfig::ENABLE_PMU_EXP_RAND = false;

bool LltcConfig::loadConfigFile(string filePath) {
	ifstream f;
	f.open(filePath, ios::in);
	if (!f.is_open()) return false;

	string line;
	regex setPattern("\\s*([A-Za-z0-9_]+)\\s*=\\s*(\\S+)\\s*");
	smatch m;
	while (getline(f, line)) {
		if (regex_match(line, m, setPattern)) {
			string name = m[1].str();
			string value = m[2].str();

			if (name.compare("NETWORK_TOPO_FILE") == 0) {
				NETWORK_TOPO_FILE = value;
			} else if (name.compare("NETWORK_TOPO_FILE_TYPE") == 0) {
				NETWORK_TOPO_FILE_TYPE = value;
			} else if (name.compare("NETWORK_NUM_SUBSTATIONS") == 0) {
				NETWORK_NUM_SUBSTATIONS = atoi(value.c_str());
			} else if (name.compare("NETWORK_NODE_ID_PDC") == 0) {
				NETWORK_NODE_ID_PDC = atoi(value.c_str());
			} else if (name.compare("NETWORK_NODE_ID_PMU_STATIC") == 0) {
				NETWORK_NODE_ID_PMU_STATIC = atoi(value.c_str());
			} else if (name.compare("NETWORK_NUM_PMUS") == 0) {
				NETWORK_NUM_PMUS = atoi(value.c_str());
			} else if (name.compare("NETWORK_PMU_DATA_FREQ") == 0) {
				NETWORK_PMU_DATA_FREQ = atoi(value.c_str());
			} else if (name.compare("NETWORK_LINK_BAND") == 0) {
				NETWORK_LINK_BAND = value;
			} else if (name.compare("NETWORK_LINK_QUEUE_SIZE") == 0) {
				NETWORK_LINK_QUEUE_SIZE = value;
			} else if (name.compare("NETWORK_NODE_PROCESS_DELAY_US") == 0) {
				NETWORK_NODE_PROCESS_DELAY_US = atoi(value.c_str());
			} else if (name.compare("NETWORK_LINK_QUEUE_DELAY_US") == 0) {
				NETWORK_LINK_QUEUE_DELAY_US = atoi(value.c_str());
			} else if (name.compare("NETWORK_LINK_TRANS_DELAY_US") == 0) {
				NETWORK_LINK_TRANS_DELAY_US = atoi(value.c_str());
			} else if (name.compare("FAULT_SIM_TYPE") == 0) {  // ERR_FOR_ALL_LINKS | ERR_FOR_RANDOM_LINKS | NONE
				if (value.compare("ERR_FOR_FULL_RR") == 0) {
					FAULT_SIM_TYPE = FAULT_SIM_TYPE_ERR_FOR_FULL_RR;
				} else if (value.compare("ERR_FOR_FULL_PP") == 0) {
					FAULT_SIM_TYPE = FAULT_SIM_TYPE_ERR_FOR_FULL_PP;
				} else if (value.compare("ERR_FOR_SINGLE_PP_LINK") == 0) {
					FAULT_SIM_TYPE = FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK;
				} else if (value.compare("ERR_FOR_MULTIPLE_LINKS") == 0) {
					FAULT_SIM_TYPE = FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS;
				} else {
					FAULT_SIM_TYPE = FAULT_SIM_TYPE_ERR_NONE;
				}
			} else if (name.compare("FAULT_SIM_TYPE_ERR_FOR_FULL_RR_ERR_RATE") == 0) {
				FAULT_SIM_TYPE_ERR_FOR_FULL_RR_ERR_RATE = atof(value.c_str());
			} else if (name.compare("FAULT_SIM_TYPE_ERR_FOR_FULL_RR_START_TIME") == 0) {
				FAULT_SIM_TYPE_ERR_FOR_FULL_RR_START_TIME = atof(value.c_str());
			} else if (name.compare("FAULT_SIM_TYPE_ERR_FOR_FULL_PP_ERR_RATE") == 0) {
				FAULT_SIM_TYPE_ERR_FOR_FULL_PP_ERR_RATE = atof(value.c_str());
			} else if (name.compare("FAULT_SIM_TYPE_ERR_FOR_FULL_PP_START_TIME") == 0) {
				FAULT_SIM_TYPE_ERR_FOR_FULL_PP_START_TIME = atof(value.c_str());
			} else if (name.compare("FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK_START_TIME") == 0) {
				FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK_START_TIME = atof(value.c_str());
			} else if (name.compare("FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK_ERR_RATE") == 0) {
				FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK_ERR_RATE = atof(value.c_str());
			} else if (name.compare("FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK_ROUTER_1") == 0) {
				FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK_ROUTER_1 = atoi(value.c_str());
			} else if (name.compare("FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK_ROUTER_2") == 0) {
				FAULT_SIM_TYPE_ERR_FOR_SINGLE_PP_LINK_ROUTER_2 = atoi(value.c_str());
			} else if (name.compare("FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_ERR_RATE") == 0) {
				FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_ERR_RATE = atof(value.c_str());
			} else if (name.compare("FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_START_TIME") == 0) {
				FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_START_TIME = atof(value.c_str());
			} else if (name.compare("FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_N_RAND_DISRUPTED_PP_LINKS") == 0) {
				FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_N_RAND_DISRUPTED_PP_LINKS = atoi(value.c_str());
			} else if (name.compare("FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_N_RAND_DISRUPTED_NON_PP_LINKS") == 0) {
				FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_N_RAND_DISRUPTED_NON_PP_LINKS = atoi(value.c_str());
			} else if (name.compare("FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_FLAP_PERIOD") == 0) {
				FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_FLAP_PERIOD = atof(value.c_str());
			} else if (name.compare("FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_FLAP_TIMES") == 0) {
				FAULT_SIM_TYPE_ERR_FOR_MULTIPLE_LINKS_FLAP_TIMES = atoi(value.c_str());
			} else if (name.compare("SIM_LOG_DIR") == 0) {
				SIM_LOG_DIR = value;
			} else if (name.compare("SIM_LOG_DIR_CP_TESTS") == 0) {
				SIM_LOG_DIR_CP_TESTS = value;
			} else if (name.compare("ENABLE_LOG_IN_OUT_MSGS") == 0) {
				ENABLE_LOG_IN_OUT_MSGS = (value.compare("true") == 0);
			} else if (name.compare("SIM_TIME_SECS") == 0) {
				SIM_TIME_SECS = atof(value.c_str());
			} else if (name.compare("SIM_PDC_START_TIME_SECS") == 0) {
				SIM_PDC_START_TIME_SECS = atof(value.c_str());
			} else if (name.compare("SIM_PMU_START_TIMES_SECS") == 0) {
				SIM_PMU_START_TIMES_SECS = atof(value.c_str());
			} else if (name.compare("LLTC_PREFIX") == 0) {
				LLTC_PREFIX = value;
			} else if (name.compare("LLTC_ASSUMED_LINK_RELIA_RATE") == 0) {
				LLTC_ASSUMED_LINK_RELIA_RATE = atof(value.c_str());
			} else if (name.compare("LLTC_BETA") == 0) {
				LLTC_BETA = atoi(value.c_str());
			} else if (name.compare("LLTC_REROUTE_PATH_GEN_ALPHA") == 0) {
				LLTC_REROUTE_PATH_GEN_ALPHA = atof(value.c_str());
			} else if (name.compare("LLTC_MAX_PATH_DELAY_US") == 0) {
				LLTC_MAX_PATH_DELAY_US = atoi(value.c_str());
			} else if (name.compare("LLTC_MAX_DRIFT_RANGE_RATIO_FOR_PMU_FREQ") == 0) {
				LLTC_MAX_DRIFT_RANGE_RATIO_FOR_PMU_FREQ = atof(value.c_str());
			} else if (name.compare("LLTC_MAX_CONSECUTIVE_DRIFT_PACKETS") == 0) {
				LLTC_MAX_CONSECUTIVE_DRIFT_PACKETS = atoi(value.c_str());
			} else if (name.compare("LLTC_NUM_DATA_RETRANS_REPORTS_TO_SEND") == 0) {
				LLTC_NUM_DATA_RETRANS_REPORTS_TO_SEND = atoi(value.c_str());
			} else if (name.compare("LLTC_NUM_RETRANS_REQUESTS") == 0) {
				LLTC_NUM_RETRANS_REQUESTS = atoi(value.c_str());
			} else if (name.compare("LLTC_QUEUE_SIZE_FOR_TRANSMITTED_DATA_IDS") == 0) {
				LLTC_QUEUE_SIZE_FOR_TRANSMITTED_DATA_IDS = atoi(value.c_str());
			} else if (name.compare("LLTC_CS_LIMITS_IN_ENTRIES") == 0) {
				LLTC_CS_LIMITS_IN_ENTRIES = atoi(value.c_str());
			} else if (name.compare("LLTC_TIMES_FOR_CHECK_PATH_CONNECTIVITY") == 0) {
				LLTC_TIMES_FOR_CHECK_PATH_CONNECTIVITY = atoi(value.c_str());
			} else if (name.compare("LLTC_PERIOD_FOR_CHECK_LINK_CONNECTIVITY_SECS") == 0) {
				LLTC_PERIOD_FOR_CHECK_LINK_CONNECTIVITY_SECS = atof(value.c_str());
			} else if (name.compare("LLTC_INTERVAL_TIME_FOR_CHECK_PATH_QOS_SECS") == 0) {
				LLTC_INTERVAL_TIME_FOR_CHECK_PATH_QOS_SECS = atof(value.c_str());
			} else if (name.compare("LLTC_INITIAL_DELAY_FOR_CHECK_LINK_CONNECTIVITY_SECS") == 0) {
				LLTC_INITIAL_DELAY_FOR_CHECK_LINK_CONNECTIVITY_SECS = atof(value.c_str());
			} else if (name.compare("LLTC_ENABLE_FAILOVER") == 0) {
				LLTC_ENABLE_FAILOVER = (value.compare("true") == 0);
			} else if (name.compare("LLTC_ENABLE_FAILOVER_ACTIVE_CHECK") == 0) {
				LLTC_ENABLE_FAILOVER_ACTIVE_CHECK = (value.compare("true") == 0);
			} else if (name.compare("LLTC_ENABLE_FAILOVER_ACTIVE_CHECK_THROUGHPUT_PERIOD_SECS") == 0) {
				LLTC_ENABLE_FAILOVER_ACTIVE_CHECK_THROUGHPUT_PERIOD_SECS = atof(value.c_str());
			} else if (name.compare("LLTC_ENABLE_FAILOVER_ACTIVE_CHECK_INTER_ARRIVAL_MAX_CONFIDENT_RATIO") == 0) {
				LLTC_ENABLE_FAILOVER_ACTIVE_CHECK_INTER_ARRIVAL_MAX_CONFIDENT_RATIO = atof(value.c_str());
			} else if (name.compare("LLTC_PDC_RESEQNEUCE_RANGE") == 0) {
				LLTC_PDC_RESEQNEUCE_RANGE = atoi(value.c_str());
			} else if (name.compare("LLTC_PIT_DURATION_MS") == 0) {
				LLTC_PIT_DURATION_MS = atof(value.c_str());
			} else if (name.compare("LLTC_DISABLE_RETRAN") == 0) {
				LLTC_DISABLE_RETRAN = (strcmp(value.c_str(), "true") == 0);
			} else if (name.compare("ENABLE_PMU_EXP_RAND") == 0) {
				ENABLE_PMU_EXP_RAND = (strcmp(value.c_str(), "true") == 0);
			}
		}
	}

	return true;
}


