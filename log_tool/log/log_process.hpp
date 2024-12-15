//  Name:
//      logger_process.hpp
//
//  Purpose:
//      logger fifo manage.

_Pragma("once")

#include "common_unit.hpp"

class log_process final
{
public:
    /// \brief constructor
    log_process() = default;

    /// \brief destructor, delete not allow for singleton pattern.
    ~log_process() = delete;

    /// \brief get_instance
    /// - This method is used to get the pattern of the class.
    /// \return the singleton pattern point of the object.
    static log_process* get_instance();

    /// \brief init
    /// - This method is used to init the object.
    /// \return Wheather initialization is success or failed.
    //log_process::get_instance()->init();创建不同设备的通信fifo
    //同时创建logger的接收线程
    bool init();

    /// \brief release
    /// - This method is used to release the object.
    /// \return Wheather release is success or failed.
    void release();

    /// \brief send_buffer
    /// - This method is used update buffer to device.
    /// \return Wheather send is success or failed.
    int send_buffer(char *ptr, int length);

private:
    /// \brief logger_rx_run
    /// - rx thread to process logger fifo rx.
    void logger_rx_run();
    /*log_process的logger_rx_run为接收运行函数，用于处理log的fifo的接收数据，
    接收到马上通过log_server：：send_buffer发到asio套接字，最后写入文件或者发到远端客户端*/

    /// \brief show_help
    /// - show help information.
    void show_help();

    /// \brief login
    /// - login process.
    bool login(char *ptr, int size);

private:
    /// \brief instance_pointer_
    /// - object used to implement the singleton pattern.
    static log_process *instance_pointer_;

    /// \brief logger_rx_thread_
    /// - logger rx thread object.
    std::thread logger_rx_thread_;//创建接收线程
    //########################################################################
    /*为不同的设备创建fifo管理*/
    /// \brief logger_rx_fifo_
    /// - fifo used for logger rx and write to remote.
    std::unique_ptr<fifo_manage> logger_rx_fifo_{nullptr};//接收rx和远程写入

    /// \brief logger_gui_tx_fifo_
    /// - fifo used for logger server rx and write to gui.
    std::unique_ptr<fifo_manage> logger_gui_tx_fifo_{nullptr};

    /// \brief logger_loc_dev_tx_fifo_
    /// - fifo used for logger server rx and write to local device.
    std::unique_ptr<fifo_manage> logger_locd_tx_fifo_{nullptr};

    /// \brief logger_low_dev_tx_fifo_
    /// - fifo used for logger server rx and write to lower device.
    std::unique_ptr<fifo_manage> logger_low_dev_tx_fifo_{nullptr};

    /// \brief logger_mp_tx_fifo_
    /// - fifo used for logger server rx and write to lower device.
    //fifo用于日志服务器rx和写入下级设备。
    std::unique_ptr<fifo_manage> logger_mp_tx_fifo_{nullptr};

    /// \brief is_login_
    /// - for login information.
    bool is_login_{false};
};