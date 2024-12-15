#include "asiose.hpp"
#include "time_manage.hpp"
#include "logger_manage.hpp"

///////////////////////////// session group ////////////////////////
void session_group::init(std::function<void(char* ptr, int size)> handler)
{
    set_.clear();
    handler_ = handler;
}

void session_group::join(share_session_pointer cur_session)
{
    std::lock_guard<std::mutex> lock(mutex_);
    set_.insert(cur_session);
}

void session_group::leave(share_session_pointer cur_session)
{
    std::lock_guard<std::mutex> lock(mutex_);
    set_.erase(cur_session);
}

share_session_pointer session_group::get_session()
{
    share_session_pointer current_Session;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (set_.size() == 0)
            current_Session = nullptr;
        else
            current_Session = *set_.begin(); //only send message to first session
            //单连接服务器每次只有一个活跃会话，所以读取第一个即可
    }
    return current_Session;
}

const std::set<share_session_pointer>& session_group::get_session_list() 
{
    return set_;
}

void session_group::run(char *pbuf, int size)
{
    handler_(pbuf, size);
}

bool session_group::is_valid()
{
    if (set_.size() != 0) 
    {
        return true;
    }

    return false;
}

void session_group::do_write(const char *buffer, int size)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (set_.size() != 0)//当前会话可用
    {
        share_session_pointer session_ptr = *set_.begin();  
        session_ptr->do_write(buffer, size);//调用session的写函数
    }
}

///////////////////////////// session ////////////////////////
void session::start()
{
    group_.join(shared_from_this());//加入当前会话
    do_read();
}

void session::do_read()
{
    auto self(shared_from_this());//通过share_ptr获得一个指向自己的指针，this
    //async_read_some：asio的实现，接收数据并发起处理操作，完成后通过[this, self]回调通知
    socket_.async_read_some(asio::buffer(data_, MAX_LENGTH),
        [this, self](std::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                data_[length] = 0;//在接收的数据尾置零，用于处理字符串
                group_.run(data_, length);
                do_read();//递归调用读取直到会话出错或断开
            }
            else
            {
                group_.leave(shared_from_this());
                std::cout<<ec<<"\n";
            }
        });//STL库中新的写法，很像仿函数
}

void session::do_write(const char *pdate, std::size_t length)
{
    auto self(shared_from_this());

    asio::async_write(socket_, asio::buffer(pdate, length),
        [this, self](std::error_code ec, std::size_t /*length*/)
        {
        if (!ec)
        {
            //do write callback
            //asio日志服务器
        }
        else
        {
            group_.leave(shared_from_this());
        }
        });
}

void session::do_close()
{
    socket_.close();
}


void asio_server::run(){
    io_context_.run();//启动事件循环，执行已提交的异步操作。
}

const std::set<share_session_pointer>& asio_server::get_session_list(){
    return group_.get_session_list();
}

share_session_pointer asio_server::get_valid_session()
{
    return group_.get_session();
}

void asio_server::close_all_session()
{
    auto session = group_.get_session();
    while (session != nullptr)
    {
        session->do_close();
        group_.leave(session);
        session = group_.get_session();
    }
}

bool asio_server::is_valid()
{
    return group_.is_valid();
}

void asio_server::do_write(const char *buffer, int size)
{
    group_.do_write(buffer, size);
}

void asio_server::init(const std::string& address,const std::string& port,
                std::function<void(char* ptr,int size)> handler){
    group_.init(handler);

    PRINT_LOG(LOG_FATAL,xGetCurrentTimes(),"asio_server start,bing:%s:%s!",address.c_str(),port.c_str());

    asio::ip::tcp::resolver resolver(io_context_);
    asio::ip::tcp::endpoint endpoint = 
        *resolver.resolve(address,port).begin();
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    do_accept();
}

void asio_sever::do_accept(){
    //async_accept异步接收一个连接
    acceptor_.async_accept(
        [this](std::error_code ec,asio::ip::tcp::socket socket){
            if(!acceptor_.is_open()){
                return;
            }
            if(!ec){//0是没错 
                close_all_session();//控制单一连接
                //启动创建一个新的会话并开始异步接收和发送数据
                std::make_shared<session>(std::move(socket),group_)->start();
                PRINT_LOG(LOG_FATAL,xGetCurrentimes(),"Connect from client!");
            }

            do_accept();
        })
}