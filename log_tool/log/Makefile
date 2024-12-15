# This is object should be build and will link to generate
# the executables
BUILD=e

CCFLAGS := -O3 -std=c++17 -fno-strict-aliasing -lrt 

cpp_objects = main.o \
              log_server.o \
              log_process.o
                

INCLUDES =  -I include/ \
            -I module/
                
#INCLUDES += -I $(ENV_PATH_ROOT)/thirdparts/asio/include/ \
            -I $(ENV_PATH_ROOT)/thirdparts/jsoncpp/include/

INCLUDES += -I /usr/local/include/boost/asio/ \
            -I $(ENV_PATH_ROOT)/thirdparts/jsoncpp/include/

#这个libjsoncpp.a文件的问题需要处理
library = $(SUPPORT_ENV_INSTALL)/lib/libjsoncpp.a

# This is the file name of the executables and path where 
# place the executables
buildout = logger_tool

# Including this file allows access to the Build System.
include $(ENV_PATH_ROOT)/common/common.mk
include $(ENV_PATH_ROOT)/build/buildrules.mk
#需要把buildrules.mk的内容取出化到此文件中