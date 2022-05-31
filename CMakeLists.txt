cmake_minimum_required(VERSION 3.10.0)

# set(PROJECT_NAME "${FLODER_NAME}")
set(PROJECT_NAME "server")

project(${PROJECT_NAME} VERSION 0.1.0)

include_directories("./net") # 头文件目录
 
# link_directories("/usr/local/lib/") # 链接库目录
 
aux_source_directory(. SRCS) # 源文件
aux_source_directory(./net SRCS)

set(CMAKE_C_FLAGS "-O1") # 设置C工程的 CFLAGS
set(CMAKE_CXX_FLAGS "-O0 -Werror") # 设置C++ 工程的 CXX_FLAGS
 
add_executable(${PROJECT_NAME} ${SRCS}) # 生成可执行文件，这里程序名即为功能名
 
target_link_libraries(${PROJECT_NAME} pthread) # 链接库
 
# # 下面使用install作为项目打包使用
# set(CMAKE_INSTALL_PREFIX ./dist)  # 自定义安装目录，打包使用
# install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin) # 打包二进制文件
 
# set(CONFIGS ${PROJECT_SOURCE_DIR}/hello.cf) 

