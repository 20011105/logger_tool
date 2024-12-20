//  Name:
//      main.cpp
//
//  Purpose:
//      用于logger打印的模块，支持包含两部分实现。
//      1.用于远程连接的web服务器。
//      2.提供其它模块访问的fifo接口。
/////////////////////////////////////////////////////////////////////////////
#include <getopt.h>
#include "common_unit.hpp"
#include "log_server.hpp"
#include "log_process.hpp"
#include "parameter_parser.hpp"

//internal data
static EVENT::semaphore global_exit_sem(0);
struct _Params gParameter;

//internal function
static bool system_init(int is_default,const char* path);

int main(int argc,char *argv[]){

}

static bool system_init(int is_default,const char* path){
    bool ret = true;

    //config file manage
    if(is_default == 0){
        ret = system_config::get_instance()->init(path);
        if(!ret){
            PRINT_NOW("%s:system_config init failed!\n", PRINT_NOW_HEAD_STR);
            return false;
        }
    }else {
        system_config::get_instance()->default_init();
    }
    LOG_LEVEL level = (LOG_LEVEL)system_config::get_instance()->get_logger_privilege().logger_device_level;
    log_manage::get_instance()->set_level(level);

    ret &=time_manage::get_instance()->init();
    ret &= log_process::get_instance()->init();//process_init中创建各设备间的通信fifo管道，建立接收线程并接收设备日志然后输出到远端或保存到文件
    ret &= log_manage::get_instance()->init();//在log_process后执行，需要等待fifo创建
    ret &= log_server::get_instance()->init();

    //std::cout<<*(system_config::get_instance())<<std::endl;
    return ret;
}

void exit_main_app(){
    global_exit_sem.signal();
}