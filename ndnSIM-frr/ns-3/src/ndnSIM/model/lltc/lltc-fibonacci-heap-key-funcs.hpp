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

#ifndef FIBONACCI_HEAP_KEY_FUNCS_HPP_
#define FIBONACCI_HEAP_KEY_FUNCS_HPP_

#include <unordered_set>
#include <climits>

#include "lltc-fibonacci-heap.hpp"

using namespace std;

namespace lltc {

#define DOUBLE_NEGATIVE_INFINITY (-1.0 / 0.0)

class FHKeyFuncLossRate : public FibonacciHeapKeyFunc {
public:
	static double PRECISION;

	FHKeyFuncLossRate(double* lossRates);
	long getKey(FHObjectPtr nodeObject);
	void setKey(FHObjectPtr nodeObject, long newKey);

private:
	double* lossRates;
};

class FHKeyFuncNodeRelias : public FibonacciHeapKeyFunc {
public:
	static double PRECISION;

	FHKeyFuncNodeRelias(double* nodeRelias);
	long getKey(FHObjectPtr nodeObject);
	void setKey(FHObjectPtr nodeObject, long newKey);

private:
	double* nodeRelias;
};


class FHKeyFuncRSPTimeoutForRetrans : public FibonacciHeapKeyFunc {
public:
	static double PRECISION;

	long getKey(FHObjectPtr nodeObject);
	void setKey(FHObjectPtr nodeObject, long newKey);
};

}

#endif /* FIBONACCI_HEAP_KEY_FUNCS_HPP_ */
