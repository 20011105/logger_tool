#include "fifo_manage.hpp"
#include "time_manage.hpp"
#include "log_mane.hpp"

fifo_manage::fifo_manage(const std::string& fstr, int mode, ENUM_FIFO_MODE fifomode)
{
    fifo_path_ = std::move(fstr);
    mode_ = mode;
    fifo_mode_ = fifomode;
    readfd_ = -1;
    writefd_ = -1;
}

fifo_manage::~fifo_manage()
{
    release();
    fifo_path_.clear();
}

bool fifo_manage::create()
{
    //with create, will process fifo create
    if (fifo_mode_ == FIFO_MODE_WR_CREATE
    || fifo_mode_ == FIFO_MODE_R_CREATE
    || fifo_mode_ == FIFO_MODE_W_CREATE)
    {
        unlink(fifo_path_.c_str());
        if (mkfifo(fifo_path_.c_str(), mode_) < 0)//0为成功
        {
            PRINT_NOW("%s:fifo %s make error!\n", PRINT_NOW_HEAD_STR, fifo_path_.c_str());
            return false;
        }
        else
        {
            PRINT_NOW("%s:fifo %s make success!\n", PRINT_NOW_HEAD_STR, fifo_path_.c_str());
        }
    }

    if (fifo_mode_ == FIFO_MODE_WR_CREATE 
    || fifo_mode_ == FIFO_MODE_WR 
    || fifo_mode_ == FIFO_MODE_R_CREATE
    || fifo_mode_ == FIFO_MODE_R)
    {
        //for Read, need open as O_RDWR, otherwise will block
        readfd_ = open(fifo_path_.c_str(), O_RDWR);//判断管道是否可读
        if (readfd_ < 0)
        {
            PRINT_NOW("%s:fifo open %s rx error:%d!\n", PRINT_NOW_HEAD_STR, fifo_path_.c_str(), readfd_);
        }
        else
        {
            PRINT_NOW("%s:fifo open %s rx success:%d!\n", PRINT_NOW_HEAD_STR, fifo_path_.c_str(), readfd_); 
        }
    }

    if (fifo_mode_ == FIFO_MODE_WR_CREATE 
    || fifo_mode_ == FIFO_MODE_WR 
    || fifo_mode_ == FIFO_MODE_W_CREATE
    || fifo_mode_ == FIFO_MODE_W)
    {
        writefd_ = open(fifo_path_.c_str(), O_RDWR);
        if (writefd_ < 0)
        {
            release();
            PRINT_NOW("%s:fifo open %s tx error:%d!\n", PRINT_NOW_HEAD_STR, fifo_path_.c_str(), writefd_);
            return false;
        }
        else
        {
            PRINT_NOW("%s:fifo open %s tx success:%d!\n", PRINT_NOW_HEAD_STR, fifo_path_.c_str(), writefd_); 
        }
    }

    return true;
}

void fifo_manage::release()
{
    if (writefd_ >= 0)
    {
        close(writefd_);
        writefd_ = -1;
    }

    if (readfd_ >= 0)
    {
        close(readfd_);
        readfd_ = -1;
    }
}

int fifo_manage::read(char *buf, int bufsize)
{
    int readbytes = -1;
    if (readfd_ >= 0)
    {
        readbytes = ::read(readfd_, buf, bufsize);
    }
    
    return readbytes;
}

int fifo_manage::write(const char *buf, int bufsize)
{
    int writebytes = -1;
    if (writefd_ >= 0 && bufsize > 0)
    {
        writebytes = ::write(writefd_, buf, bufsize);
    }
    return writebytes;
}