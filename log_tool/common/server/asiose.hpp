//  Name:
//      asio_server.hpp
//
//  Purpose:
//      基于asio实现的tcp服务器, 单连接，有新连接则断开旧连接
_Pragma("once")
#if !__has_include("asio.hpp")
#error "asio library not exist, need release in lib directory"
#endif

#include <boost/asio.hpp>
#include <set>
#include <memory>
#include <iostream>
#include <mutex>

#define MAX_LENGTH     1024

class session;
using share_session_pointer = std::shared_ptr<session>;

class session_group{
public:
    /// \brief constructor
    session_group()=default;

    /// \brief init
    /// - This method is used to init the group
    /// \param handler -- function for the server main workflow.
    void init(std::function<void(char* ptr, int size)> handler);//处理函数是该服务器的主要工作流

    /// \brief run
    /// - This method is the session run.
    /// \param pbuf -- rx data pointer.
    /// \param size -- rx data size.
    void run(char *pbuf, int size);

    /// \brief is_valid
    /// - This method is used to check wheather have valid session.
    /// \return wheate have valid session.
    bool is_valid();

    /// \brief do_write
    /// - This method is used to write buffer.
    /// \param buffer -- tx data pointer.
    /// \param size -- tx data size.
    void do_write(const char *buffer, int size);

    /// \brief join
    /// - This method is push session into group.
    /// \param cur_session -- session push to group.
    void join(share_session_pointer cur_session);//当前会话添加到组

    /*jion时使用互斥量保护会话的添加过程*/

    /// \brief leave
    /// - This method is delete session in the group.
    /// \param cur_session -- session delete from group
    void leave(share_session_pointer cur_session);//移除当前会话

    /// \brief get_session
    /// - This method is get current work session.
    /// \return the current work session.
    share_session_pointer get_session();//获取当前工作的会话信息

    /// \brief get_session_list
    /// - This method is get current work session list.
    /// \return the current work session list.
    const std::set<share_session_pointer>& get_session_list();
private:
    /// \brief mutex_
    /// - mutex used to protect group manage.
    std::mutex mutex_;

    /// \brief set_
    /// - set used to store session.//set_储存会话集合
    std::set<share_session_pointer> set_;

    /// \brief handler_
    /// - hander used to work when run.
    std::function<void(char* ptr, int size)> handler_;
};

class session:
    public std::enable_shared_from_this<session>
{
public:
    /// \brief constructor
    session(asio::ip::tcp::socket socket, session_group& group)
    :socket_(std::move(socket)), group_(group)//传入参数赋给对应值
    {
        memset(data_, 0, MAX_LENGTH);
    }

    /// \brief start
    /// - This method is used to start a connect and read.
    void start();//开始一个连接并且开始读

    /// \brief do_read
    /// - This method is used to do read buffer.
    void do_read();

    /// \brief do_write
    /// - This method is used to do write buffer.
    /// \param pdata - pointer to start of write buffer.
    /// \param length - length of the write buffer.
    void do_write(const char *pdate, std::size_t length);

    /// \brief do_close
    /// - This method is used to do close session.
    void do_close();

private:
    /// \brief socket_
    /// - socket used to manage
    asio::ip::tcp::socket socket_;

    /// \brief data_
    /// - data_ used to save read data
    char data_[MAX_LENGTH];

    /// \brief group_
    /// - group_ used to save all session.
    session_group& group_;
};

class asio_server
{
public:
    /// \brief no copy and moveable constructor
    asio_server(const asio_server&) = delete;
    asio_server& operator=(const asio_server&) = delete;//禁止拷贝构造

    explicit asio_server()
    :io_context_(2), acceptor_(io_context_){}

    void init(
        const std::string& address, 
        const std::string& port, 
        std::function<void(char* ptr, int size)> handler);

    /// \brief run
    /// - This method is used to start the asio server.
    void run();

    /// \brief close_all_session
    /// - This method is used to close all session.
    void close_all_session();

    /// \brief is_valid
    /// - This method is used to check wheather have valid session.
    /// \return wheate have valid session.
    bool is_valid();

    /// \brief do_write
    /// - This method is used to write buffer.
    /// \param buffer -- tx data pointer.
    /// \param size -- tx data size.
    void do_write(const char *buffer, int size);

    /// \brief get_session
    /// - This method is get current work session.
    /// \return the current work session.
    share_session_pointer get_valid_session();

    /// \brief get_session_list
    /// - This method is get current work session list.
    /// \return the current work session list.
    const std::set<share_session_pointer>& get_session_list();

    private:
    /// \brief do_accept
    /// - This method is process socket accept action.
    void do_accept();

private:
    /// \brief io_context_
    /// - manage the context of the server.
    /*  io_context：这是Boost.Asio的核心，负责调度所有异步操作。
        所有的异步任务都通过io_context来执行。*/
    asio::io_context io_context_;

    /// \brief acceptor_
    /// - manage the tcp information.
    asio::ip::tcp::acceptor acceptor_;

    /// \brief group_
    /// - save all the link session.
    session_group group_;

}