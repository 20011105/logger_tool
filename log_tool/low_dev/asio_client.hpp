//  Name:
//      asio_client.hpp
//
//  Purpose:
//      asio客户端模块
/////////////////////////////////////////////////////////////////////////////
#include "boost/asio.hpp"
#include "common_unit.hpp"

#define CLIENT_RX_MAX_BUFFER_SIZE        1024
#define CLIENT_TX_MAX_BUFFER_SIZE        1024

class asio_client{
public:
    explicit asio_client():io_context_(2),socket(io_context_){
        memset(rx_buffer_, 0, CLIENT_RX_MAX_BUFFER_SIZE);
        memset(tx_buffer_, 0, CLIENT_TX_MAX_BUFFER_SIZE);
    }

    ~asio_client() = delete;
public:
    int send_msg(char *buffer,uint16_t size);

    bool init();

    static asio_client *get_instance();
private:
    /// \brief asio_client_rx_run
    /// - tx thread to process asio client rx info.
    void asio_client_rx_run();

    /// \brief logger_rx_server_run
    /// - rx thread to process logger interface received.
    void asio_client_tx_run();

    /// \brief write_data
    /// - this method is used to write data to uart interface.
    /// \param pbuffer - the buffer of data write to uart interface.
    /// \param size - size of data write to uart interface.
    /// \return data already send to uart interface.
    int write_data(char *pbuffer, uint16_t size);

private:
    /// \brief instance_pointer_
    /// - object used to implement the singleton pattern.
    static asio_client *instance_pointer_;//静态指针，用于创建单例对象

    /// \brief is_client_link
    /// - wheather client is link.
    std::atomic<bool> is_client_link_{false};

    /// \brief client_tx_thread_
    /// - client tx thread object.
    std::thread client_tx_thread_;

    /// \brief client_rx_thread_
    /// - client rx thread object.
    std::thread client_rx_thread_;

    /// \brief logger_fifo_
    /// - fifo used for logger manage.
    std::unique_ptr<fifo_manage> client_tx_fifo_{nullptr};

    /// \brief rx_buffer_
    /// - rx buffer for receive.
    char rx_buffer_[CLIENT_RX_MAX_BUFFER_SIZE];

    /// \brief tx_buffer_
    /// - tx buffer for transfer.
    char tx_buffer_[CLIENT_TX_MAX_BUFFER_SIZE];

    /// \brief io_context_
    /// - manage the context of the server. //asio::io_context 是一个类，属于 Boost.Asio 库，用于管理异步操作的上下文。
    asio::io_context io_context_;           //io_context_ 是一个 I/O 上下文对象，用来执行和调度异步 I/O 操作。它充当了事件循环的核心，负责调度所有待处理的 I/O 操作。

    /// \brief acceptor_
    /// - manage the tcp information.       //asio::ip::tcp::socket 是一个类，表示 TCP 套接字。
    asio::ip::tcp::socket socket_;          //socket_ 是一个 TCP 套接字对象，用于在 TCP 网络协议下进行通信。它允许你在网络上建立连接，进行数据传输，发送和接收信息。
};