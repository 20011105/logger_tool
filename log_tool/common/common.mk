common_path = $(ENV_PATH_ROOT)/common

#cpp 
cpp_objects +=  ${common_path}/fifo/fifo_manage.o
cpp_objects +=  ${common_path}/jsonconfig/json_config.o
cpp_objects +=  ${common_path}/logger/log_mane.o
cpp_objects +=  ${common_path}/server/asiose.o
cpp_objects +=  ${common_path}/time_manage.o
cpp_objects +=  $(common_path)/parameter_parser.o

#include
INCLUDES += -I ${common_path}/
INCLUDES += -I ${common_path}/event/
INCLUDES += -I ${common_path}/fifo/
INCLUDES += -I ${common_path}/jsonconfig/
INCLUDES += -I ${common_path}/logger/
INCLUDES += -I ${common_path}/server/
INCLUDES += -I $(ENV_PATH_ROOT)/thirdparts/fmt/include/