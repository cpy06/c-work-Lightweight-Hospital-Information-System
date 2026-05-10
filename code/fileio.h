#ifndef FILEIO_H
#define FILEIO_H

/*
 * fileio.h
 * 放文件读写模块对外的接口
 * 主要就是把数据从 txt 里读进来 再存回去
 */

#include "common.h"

/* 把所有业务数据文件都读进来，成功返回 1，失败返回 0 */
int load_all_data(HospitalSystem *system);

/* 把所有业务数据文件都存回去，全成了就返回 1，不然返回 0 */
int save_all_data(HospitalSystem *system);

/* 会尝试切到有数据文件的目录里，这样直接跑 main 也比较方便 */
void prepare_data_directory(const char *argv0, const char *source_file);

#endif
