//  Name:
//      logger.cpp
//
//  Purpose:
//      基于asio_server实现的tcp服务器, 提供命令的接收处理以及异步的logger打印接口

#include "log_mane.hpp"
#include "time_manage.hpp"
#include "json_config.hpp"
#include "asiose.hpp"

#define FMT_HEADER_ONLY
#include "fmt/core.h"

log_manage* log_manage::instance_pointer_ = nullptr;
log_manage* log_manage::get_instance()
{
    if (instance_pointer_ == nullptr)
    {
        instance_pointer_ = new(std::nothrow) log_manage;
        if (instance_pointer_ == nullptr)
        {
            //do something
        }
    }
    return instance_pointer_;
}

char memoryBuffer[LOGGER_MESSAGE_BUFFER_SIZE+1];
bool log_manage::init()
{
    memory_start_pointer_ = memoryBuffer;
    memory_end_pointer_ = &memoryBuffer[LOGGER_MESSAGE_BUFFER_SIZE];

    //init and Create logger fifo, must before thread run.
    /*LOGGER_RX_FIFO为定义的管道的名字，默认读模式，自身可读，其他成员只可写    FIFO_MODE_W？*/
    logger_fifo_ = std::make_unique<fifo_manage>(LOGGER_RX_FIFO, S_FIFO_WORK_MODE, FIFO_MODE_W);
    if (logger_fifo_ == nullptr)
        return false;
    if (!logger_fifo_->create())
        return false;

    return true;
}

void log_manage::release()
{
    logger_fifo_->release();
}
/*似乎是复用的buffer*/
char *log_manage::get_memory_buffer_pointer(uint16_t size)
{
    char *pCurrentMemBuffer;

    pCurrentMemBuffer = memory_start_pointer_;
    memory_start_pointer_ = pCurrentMemBuffer+size;
    if (memory_start_pointer_ >  memory_end_pointer_)
    {
        pCurrentMemBuffer = memoryBuffer;
        memory_start_pointer_ = pCurrentMemBuffer + size;
    }
    return(pCurrentMemBuffer);
}

std::string log_manage::convert_timer(uint32_t timer)
{
    uint16_t sec, min, hours, day;

    sec = timer%60;
    timer = timer/60;
    min = timer%60;
    timer = timer/60;
    hours = timer%24;
    timer = timer/24;
    day = timer;

    std::string fmt_str = fmt::format("{0:0>4} {1:0>2}:{2:0>2}:{3:0>2}", day, hours, min, sec);
    return fmt_str;
}

static char logger_tx_buffer[LOGGER_MAX_BUFFER_SIZE];//日志数组要储存满才会释放一次
int log_manage::print_log(LOG_LEVEL level, uint32_t time, const char* fmt, ...)
{
    int len;
    char *pbuf = logger_tx_buffer;
    int tx_len = 0;
    int bufferlen = LOGGER_MAX_BUFFER_SIZE;
    //减少低级日志的不必要打印
    if (level < log_level_)//如果调用print_log时输入的日志等级小于日志的默认优先级则直接退出
    {
        return 0;
    }

    mutex_.lock();//读取数据锁保护
    
    //add logger header
    len = snprintf(pbuf, bufferlen, "[%s][%s][%d]:", convert_timer(time).c_str(), TOOLS_NAME, level);
    if ((len<=0) || (len>=bufferlen))
    {
        PRINT_NOW("%s: %s not support buffer-1!\n", PRINT_NOW_HEAD_STR, __func__);
        mutex_.unlock();
        return 0;
    }
    pbuf = &pbuf[len];//日志数组要储存满才会释放一次######################################3
    bufferlen -= len;
    tx_len += len;

    //add logger info
    va_list valist;//储存可变参数
    va_start(valist, fmt);//格式化fmt字符串和alist中的可变参数
    len = vsnprintf(pbuf, bufferlen, fmt, valist);
    va_end(valist);

    if ((len<=0) || (len>=bufferlen))
    {
        PRINT_NOW("%s: %s not support buffer, len:%d-2!\n", PRINT_NOW_HEAD_STR, __func__, len);
        mutex_.unlock();
        return 0;
    }
    pbuf = &pbuf[len];
    bufferlen -= len;
    tx_len += len;
    mutex_.unlock();

    //add logger tail \r\n
    if (bufferlen < 3)
    {
        PRINT_NOW("%s: %s not support buffer-3!\n", PRINT_NOW_HEAD_STR, __func__);
        return 0;
    }
    pbuf[0] = '\r';
    pbuf[1] = '\n';
    tx_len += 2;

    //send the logger info.
    if (!logger_fifo_->is_write_valid())
    {//如果 FIFO 不可写（is_write_valid() 返回 false），数据会被直接写到标准输出。
        len = write(STDOUT_FILENO, logger_tx_buffer, tx_len);
        fflush(stdout);
    }
    else
    {//可写则向fifo写入数据
        len = logger_fifo_->write(logger_tx_buffer, tx_len);
        if (len<=0)
        {
            PRINT_NOW("%s:%s not support buffer-4!\n", PRINT_NOW_HEAD_STR, __func__);
        }
    }

    return  tx_len;
}
