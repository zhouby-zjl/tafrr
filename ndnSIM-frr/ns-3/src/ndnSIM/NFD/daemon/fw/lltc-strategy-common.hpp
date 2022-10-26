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

#ifndef SRC_NDNSIM_NFD_DAEMON_FW_LLTC_STRATEGY_COMMON_HPP_
#define SRC_NDNSIM_NFD_DAEMON_FW_LLTC_STRATEGY_COMMON_HPP_


#include "ns3/nstime.h"
#include "ns3/ndnSIM/ndn-cxx/lp/tags.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/ndn-cxx/encoding/block-helpers.hpp"
#include "ns3/ndnSIM/ndn-cxx/name.hpp"

#include "lltc-common.hpp"
#include "lltc-fs.hpp"
#include "lltc-strategy.hpp"
#include "lltc-strategy-config.hpp"

#include "ns3/ndnSIM/model/lltc/lltc-config.hpp"
#include "ns3/ndnSIM/model/lltc/lltc-router.hpp"
#include "ns3/ndnSIM/model/lltc/lltc-resilient-routes-generation.hpp"
#include "NFD/daemon/fw/forwarder.hpp"
#include "NFD/daemon/face/face-common.hpp"
#include "model/ndn-net-device-transport.hpp"
#include "NFD/daemon/table/pit-entry.hpp"
#include "NFD/daemon/table/pit-in-record.hpp"
#include "NFD/daemon/table/cs-policy-priority-fifo.hpp"

#include "ns3/ndnSIM/model/lltc/lltc-utils.hpp"

#include <algorithm>

using namespace nfd::cs;
using namespace nfd::cs::priority_fifo;



#endif /* SRC_NDNSIM_NFD_DAEMON_FW_LLTC_STRATEGY_COMMON_HPP_ */
