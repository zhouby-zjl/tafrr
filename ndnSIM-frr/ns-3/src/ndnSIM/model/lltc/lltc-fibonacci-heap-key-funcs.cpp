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

#include "lltc-fibonacci-heap-key-funcs.hpp"

#include "lltc-resilient-routes-generation.hpp"

using namespace lltc;

// ============================================================================================

double FHKeyFuncLossRate::PRECISION = 1E18;

FHKeyFuncLossRate::FHKeyFuncLossRate(double* lossRates) {
	this->lossRates = lossRates;
}

long FHKeyFuncLossRate::getKey(FHObjectPtr nodeObject) {
	int nodeId = *((int*) (nodeObject));
	double lossRate = lossRates[nodeId];
	long key = -(long) (lossRate * PRECISION);
	return key;
}

void FHKeyFuncLossRate::setKey(FHObjectPtr nodeObject, long newKey) {
	int nodeId = *((int*) (nodeObject));
	double lossRate = -(double)(newKey) / PRECISION;
	lossRates[nodeId] = lossRate;
}

// ============================================================================================

double FHKeyFuncNodeRelias::PRECISION = 1000000.0;

FHKeyFuncNodeRelias::FHKeyFuncNodeRelias(double* nodeRelias) {
	this->nodeRelias = nodeRelias;
}

long FHKeyFuncNodeRelias::getKey(FHObjectPtr nodeObject) {
	int nodeId = *((int*) (nodeObject));
	double reliaInLog = nodeRelias[nodeId];
	long r;
	if (reliaInLog == DOUBLE_NEGATIVE_INFINITY) {
		r = INT_MIN;
	} else {
		r = (int) (reliaInLog * PRECISION);
	}
	return r;
}

void FHKeyFuncNodeRelias::setKey(FHObjectPtr nodeObject, long newKey) {
	int nodeId = *((int*) (nodeObject));
	double nodeRelia = (double) newKey / PRECISION;
	nodeRelias[nodeId] = nodeRelia;
}


// ===========================================================================================

double FHKeyFuncRSPTimeoutForRetrans::PRECISION = 1000000.0;

long FHKeyFuncRSPTimeoutForRetrans::getKey(FHObjectPtr nodeObject) {
	RedundantSubPath* rsp = (RedundantSubPath*) nodeObject;
	long key = -(long) (rsp->retransDelayInMultiTimes * PRECISION);
	return key;
}

void FHKeyFuncRSPTimeoutForRetrans::setKey(FHObjectPtr nodeObject, long newKey) {
	RedundantSubPath* rsp = (RedundantSubPath*) nodeObject;
	rsp->retransDelayInMultiTimes = -(double) newKey / PRECISION;
}
