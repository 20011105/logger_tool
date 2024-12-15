# 定义编译器
CXX = g++

workspaceFolder = /home/lixin/IMX6ULL/linux_project/asiostu/
# 定义编译选项，包括包含路径
CXXFLAGS = -I${workspaceFolder}/common \
           -I${workspaceFolder}/common/event \
           -I${workspaceFolder}/low_dev/include \
           -I${workspaceFolder}/log/include \
           -I/home/lixin/IMX6ULL/linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek/include \
           -I/home/lixin/IMX6ULL/linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek/arch/arm/include \
           -I/home/lixin/IMX6ULL/linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek/arch/arm/include/generated/ \
           -I/usr/local/include/

# 定义源文件目录
SRC_DIR = ${workspaceFolder}/low_dev

# 定义源文件
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# 定义目标文件
OBJS = $(SRCS:.cpp=.o)

# 定义可执行文件名
TARGET = main

# 生成目标文件
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 编译源文件
%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -f $(OBJS) $(TARGET)

# 伪目标，用于显示Makefile变量
.PHONY: clean