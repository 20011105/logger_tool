//  Name:
//      UartThread.hpp
//
//  Purpose:
//      Uart通讯执行模块，根据原项目为同步模式
/////////////////////////////////////////////////////////////////////////////
_Pragma("once")

#include "common_unit.hpp"

#define SERIAL_RX_MAX_BUFFER_SIZE        1024
#define SERIAL_TX_MAX_BUFFER_SIZE        1024

class serial_manage
{
public:
    /// \brief constructor
    serial_manage() = default;

    /// \brief destructor, delete not allow for singleton pattern.
    ~serial_manage() = delete;
    
    /// \brief get_instance
    /// - This method is used to get the pattern of the class.
    /// \return the singleton pattern point of the object.
    static serial_manage* get_instance();

    /// \brief init
    /// - This method is used to init the object.
    /// \return Wheather initialization is success or failed.
    bool init();

    /// \brief release
    /// - This method is used to release the object.
    void release();

    /// \brief set_opt//串口设置：波特率，数据位，优先级，停止位
    /// - This method is used to option the uart informate.
    /// \param nBaud - baud rate of the uart option
    /// \param nDataBits - data bits of the uart option
    /// \param cParity - parity of the uart option
    /// \param nStopBits - stop bits of the uart option
    /// \return wheather the option is sucess or failed
    int set_opt(int nBaud, int nDataBits, std::string cParity, int nStopBits);

    /// \brief send_msg
    /// - this method is used to send data with protocol to remote.
    /// \param buffer - the buffer of data area to remote.
    /// \param size - size of data area to remote.
    /// \return data already send to remote.
    int send_msg(char *buffer, uint16_t size);

private:
    /// \brief uart_server_run
    /// - server thread to process tcp received, tranform to rx fifo.
    void uart_server_run();//处理 TCP 收到的数据，并将这些数据转换到 RX FIFO（接收队列）中。

    /// \brief tcp_rx_run
    /// - read from protocol tx fifo and do write data.//说明该函数从一个 TX FIFO 队列 中读取数据。
    void uart_tx_run();

private:
    /// \brief instance_pointer_
    /// - object used to implement the singleton pattern.
    static serial_manage* instance_pointer_;

    /// \brief tty_
    /// - tty used to control the interface.
    tty_control tty_;

    /// \brief mutex_
    /// - mutex used to protect com send.
    std::mutex mutex_;

    /// \brief uart_server_thread_
    /// - uart server thread object.
    std::thread uart_server_thread_;

    /// \brief uart_rx_thread_
    /// - uart rx thread object.
    std::thread uart_rx_thread_;

    /// \brief uart_tx_thread_
    /// - uart tx thread object.
    std::thread uart_tx_thread_;

    /// \brief serial_tx_fifo_
    /// - fifo used for logger manage.
    std::unique_ptr<fifo_manage> serial_tx_fifo_{nullptr};

    /// \brief rx_buffer_
    /// - rx buffer for receive.    
    char rx_buffer_[SERIAL_RX_MAX_BUFFER_SIZE];

    /// \brief tx_buffer_
    /// - tx buffer for trans.
    char tx_buffer_[SERIAL_TX_MAX_BUFFER_SIZE];
};
