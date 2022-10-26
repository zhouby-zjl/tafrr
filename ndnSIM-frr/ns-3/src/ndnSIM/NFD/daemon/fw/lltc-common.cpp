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

#include "lltc-common.hpp"
#include <stdlib.h>

namespace nfd {
namespace fw {

LltcDataIdQueue::LltcDataIdQueue() {}

LltcDataIdQueue::~LltcDataIdQueue() {}

void LltcDataIdQueue::setLen(size_t n) {
	this->n = n;
}

void LltcDataIdQueue::push(LltcDataID x, uint64_t time) {
	if (set.find(x) != set.end()) {
		return;
	}
	if (q.size() == n) {
		LltcDataID x0 = q.front();
		q.pop();
		set.erase(x0);
		dataIdAddTimeMap.erase(x0);

		--n;
	}
	q.push(x);
	set.insert(x);
	dataIdAddTimeMap[x] = time;
	++n;
}

LltcDataID LltcDataIdQueue::pop() {
	LltcDataID x = q.front();
	q.pop();
	set.erase(x);
	dataIdAddTimeMap.erase(x);
	--n;
	return x;
}

bool LltcDataIdQueue::contains(LltcDataID x) {
	return set.find(x) != set.end();
}

size_t LltcDataIdQueue::size() {
	return this->n;
}

uint64_t LltcDataIdQueue::getDataIdAddTime(LltcDataID dataID) {
	if (dataIdAddTimeMap.find(dataID) == dataIdAddTimeMap.end()) return 0;
	return dataIdAddTimeMap[dataID];
}

}
}
