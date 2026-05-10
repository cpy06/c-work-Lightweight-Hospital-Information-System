#ifndef TESTDATA_H
#define TESTDATA_H

/*
 * testdata.h
 * 放演示数据模块对外的接口
 * 需要造默认数据的时候 直接调这里
 */

#include "common.h"

/* 一次性生成整套演示数据，直接塞到系统链表里 */
void generate_demo_data(HospitalSystem *system);

#endif
