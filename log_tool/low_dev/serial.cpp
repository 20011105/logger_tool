//  Name:
//      UartThread.cpp
//
//  Purpose:
//      Uart通讯执行模块，根据原项目为同步模式
/////////////////////////////////////////////////////////////////////////////
#include "serial.hpp"
#include <sys/termios.h>
#include "asio_client.hpp"
#include "common_unit.hpp"

serial_manage* serial_manage::instance_pointer_ = nullptr;
serial_manage* serial_manage::get_instance()
{
    if (instance_pointer_ == nullptr)
    {
        instance_pointer_ = new(std::nothrow) serial_manage();
        if (instance_pointer_ == nullptr)
        {
            PRINT_LOG(LOG_ERROR, xGetCurrentTimes(), "serial_manage new failed!");
        }
    }
    return instance_pointer_;
}

//从客户端串口获取输入的调试信息并放到客户端发送fifo，用于传输远程调试信息到主机
void serial_manage::uart_server_run()
{
    PRINT_LOG(LOG_INFO, xGetCurrentTimes(), "%s start", __func__);

    for (;;)
    {   //因为接收到的日志信息之间打印到串口输出了，所以这里从串口读取rx_buffer_，
        //以方便管理串口和客户端的两个rx_buffer_
        int size = tty_.read(rx_buffer_, SERIAL_RX_MAX_BUFFER_SIZE);
        if (size > 0) {
            //put int asio client to send
            asio_client::get_instance()->send_msg(rx_buffer_, size);
        } else if (size == 0) {
            //do nothing
        } else {
            PRINT_LOG(LOG_INFO, xGetCurrentTimes(), "tty_ maybe close, loop stop!");
            break;
        }
    }
}

//从串口fifo读取接收的服务端数据；并发送到串口输出
void serial_manage::uart_tx_run()
{
    int size;

    PRINT_LOG(LOG_INFO, xGetCurrentTimes(), "%s start", __func__);

    while (1) {
        size = serial_tx_fifo_->read(tx_buffer_, SERIAL_RX_MAX_BUFFER_SIZE);
        if (size > 0) {
            tty_.write(tx_buffer_, size);
        } else {
            //do nothing
        } 
    }
}

int serial_manage::send_msg(char *buffer, uint16_t size)
{
    return serial_tx_fifo_->write(buffer, size);
}

bool serial_manage::init()
{
    const auto& serial_config = system_config::get_instance()->get_lower_device_serial_config();

    // 打开tty设备
    if(!tty_.init(serial_config.dev, O_RDWR)) {
        PRINT_NOW("%s:serial_manage Open Device %s failed!", PRINT_NOW_HEAD_STR, serial_config.dev.c_str());
        return false;
    }

    // 配置tty功能//配置串口波特率，数据位等信息
    if(!tty_.set_opt(serial_config.baud, serial_config.dataBits, serial_config.parity, serial_config.stopBits)) {
        PRINT_NOW("%s:serial_manage set opt failed!", PRINT_NOW_HEAD_STR);
        return false;
    }

    serial_tx_fifo_ = std::make_unique<fifo_manage>(SERVER_UART_TX_FIFO, S_FIFO_WORK_MODE);

    uart_server_thread_ = std::thread(std::bind(&serial_manage::uart_server_run, this));
    uart_server_thread_.detach();
    uart_tx_thread_ = std::thread(std::bind(&serial_manage::uart_tx_run, this));
    uart_tx_thread_.detach();

    PRINT_LOG(LOG_INFO, xGetCurrentTimes(), "serial_manage init success!");
    return true;
}

void serial_manage::release()
{
    tty_.close();
}