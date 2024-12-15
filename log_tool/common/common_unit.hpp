//  Name:
//      common.hpp
//
//  Purpose:
//      全局的用于支持项目编译包含的库

_Pragma("once")

#include "common.hpp"

//common unit for all application
#include "tty.hpp"
#include "time_manage.hpp"
#include "fifo/fifo_manage.hpp"
#include "jsonconfig/json_config.hpp"
#include "logger/log_mane.hpp"

uint16_t crc16(uint16_t crc, uint8_t const *buffer, uint16_t len);