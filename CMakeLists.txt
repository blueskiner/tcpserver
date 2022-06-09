cmake_minimum_required(VERSION 3.10.0)

# set(PROJECT_NAME "${FLODER_NAME}")
set(PROJECT_NAME "server")

project(${PROJECT_NAME} VERSION 0.1.0)

include_directories(
    "./net"
     "~/muduo"
) # 头文件目录
 
# link_directories("~/build/release-cpp11/lib") # 链接库目录

# 设置muduo网络静态库的目录.
set(muduo_lib_path "../build/release-cpp11/lib")

add_library(muduo_base STATIC IMPORTED)
set_target_properties(muduo_base PROPERTIES IMPORTED_LOCATION ${muduo_lib_path}/libmuduo_base.a)

add_library(muduo_net STATIC IMPORTED)
set_target_properties(muduo_net PROPERTIES IMPORTED_LOCATION ${muduo_lib_path}/libmuduo_net.a)
 
aux_source_directory(. SRCS) # 源文件
aux_source_directory(./net SRCS)

set(CMAKE_C_FLAGS "-O1") # 设置C工程的 CFLAGS
set(CMAKE_CXX_FLAGS "-O0 -Werror") # 设置C++ 工程的 CXX_FLAGS
 
add_executable(${PROJECT_NAME} ${SRCS}) # 生成可执行文件，这里程序名即为功能名
 
target_link_libraries(
    ${PROJECT_NAME}
    muduo_net# 依赖muduo_base
    muduo_base# muduo基础库依赖pthread
    protobuf# 协议序列化
    pthread# 越基础的库越要放在最后面
) # 链接库
 
# # 下面使用install作为项目打包使用
# set(CMAKE_INSTALL_PREFIX ./dist)  # 自定义安装目录，打包使用
# install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin) # 打包二进制文件
 
# set(CONFIGS ${PROJECT_SOURCE_DIR}/hello.cf) 

