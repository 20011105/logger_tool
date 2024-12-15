//  Name:
//      asio_client.cpp
//
//  Purpose:
//      asio客户端模块
/////////////////////////////////////////////////////////////////////////////
#include "asio_client.hpp"
#include "serial.hpp"

asio_client* asio_client::instance_pointer_ = nullptr;
asio_client* asio_client::get_instance(){
    if (instance_pointer_ == nullptr)
    {
        instance_pointer_ = new(std::nothrow) asio_client;
        if (instance_pointer_ == nullptr)
        {
            //do something
        }
    }
    return instance_pointer_;
}

bool asio_client::init(){
    //create the tx fifo;用于修改主机配置
    client_tx_fifo_ = std::make_unique<fifo_manage>(ASIO_CLENET_FIFO, S_FIFO_WORK_MODE);
    if (client_tx_fifo_ == nullptr)
        return false;
    if (!client_tx_fifo_->create())
        return false;

    client_rx_thread_ = std::thread(std::bind(&asio_client::asio_client_rx_run, this));//创建asio_client传输和接收的线程，并且分离线程使其异步执行
    client_rx_thread_.detach();
    client_tx_thread_ = std::thread(std::bind(&asio_client::asio_client_tx_run, this));
    client_tx_thread_.detach();

    return true;
}

int asio_client::send_msg(char *buffer, uint16_t size)
{
    int out_size;
    out_size = client_tx_fifo_->write(buffer, size);
    return out_size;
}

void asio_client::asio_client_tx_run()//从串口读出调试指令并发送到服务端
{
    int size;

    PRINT_LOG(LOG_INFO, xGetCurrentTimes(), "%s start", __func__);
    while (1)
    {   //从client_tx_fifo_读取串口发来的调试信息并发到服务端
        size = client_tx_fifo_->read(tx_buffer_, CLIENT_TX_MAX_BUFFER_SIZE);
        if (size > 0)
        {
            write_data(tx_buffer_, size);//数据写入服务端
        }
        else
        {
            //do nothing
        }
    }
}

//从asio服务器读取信息
void asio_client::asio_client_rx_run()//
{
    asio::ip::tcp::resolver resolver(io_context_);//resolver 对象：用于解析 ipaddress 和 port，将其转换为 Boost.Asio 所需的 tcp::endpoint 格式。
    const auto& ipaddress = system_config::get_instance()->get_local_ipaddress();//获取本地ip地址
    const auto& port = system_config::get_instance()->get_lower_device_remote_port();//获取本地设备端口

    PRINT_LOG(LOG_INFO, xGetCurrentTimes(), "%s start", __func__);

    while(1){//每一次读失败，等待15s再进行第二次连接
        try{
            PRINT_LOG(LOG_INFO, xGetCurrentTimes(), "start asio client connet:%s:%d", ipaddress.c_str(), port);//提示开始连接
            asio::connect(socket_, resolver.resolve(ipaddress, std::to_string(port)));  //连接远程tcp服务器
            is_client_link_ = true;//原子变量保护消息传递
            PRINT_LOG(LOG_INFO, xGetCurrentTimes(), "asio client connet success!");

            do
            {   //连接上从服务器读取信息并存到rx_buffer_中；
                size_t reply_length = asio::read(socket_, asio::buffer(rx_buffer_, CLIENT_RX_MAX_BUFFER_SIZE));//读取长度为CLIENT_RX_MAX_BUFFER_SIZE
                if (reply_length > 0)
                {//接收的数据输出到串口
                    serial_manage::get_instance()->send_msg(rx_buffer_, reply_length);//接收到的数据传到串口？
                }
                else
                {
                    //do nothing
                }
            }while (1);
        } catch(std::execption e){
            is_client_link_ = false;
            PRINT_LOG(LOG_INFO, xGetCurrentTimes(), "asio client run error:%s", e.what());

            //for client, detect link every 15s
            std::this_thread::sleep_for(std::chrono::seconds(15));
        }
    }

}

int asio_client::write_data(char *pbuffer, uint16_t size)//将数据从缓冲区发送到远程服务器
{
    int send_size = -1;

    if (is_client_link_)//std::atomic<bool> is_client_link_{false};atomic原子变量，判断客户端是否连接
    {
        send_size = asio::write(socket_, asio::buffer(pbuffer, size));//返回的值 send_size 是实际写入的字节数，成功时它会等于 size
    }

    return send_size;
}

