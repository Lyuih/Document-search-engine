#pragma once

/**
 * yui的日志系统
 * 功能：支持日志的等级分类，可变参数，直接输出与文件保存，保存信息包括用户文件名、当前行号、自定义消息、进程id
 * 日志等级：level
 *  DEBUG = 0,  // 调试信息
    INFO,       // 普通信息
    WARNING,    // 警告信息
    ERROR,      // 错误信息
    FATAL       // 致命错误
 *
    函数可变参数规则：char* format,...
    va_list arg;
    va_start(arg,format);
    va_end(arg);
    最终日志字符串：时间+日志等级+当前pid+文件名+当前行号+输入内容

    获取当前时间的字符串表示
    time_t xxx = time(nullptr)
    struct tm* format_time = localtime(&xxx) 转化为本地时间
    格式化转化为字符串
    format_time->tm_year+1900 tm_mon+1 tm_mday tm_hour tm_min tm_sec

    宏定义获取文件名 __FILE__ 行号 __LINE__ 可变参数##__VA_ARGS__
 */

#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <cstdarg>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

//日志等级
enum Level{
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

//定义全局变量，判断是否保存日志内容、保存文件内容的文件路径
bool is_save = false;
std::string file_name = "log.log";

//定义宏
#define able_save()    \
    do{                \
        is_save = true;\
    }while(0)

#define enable_save()   \
    do{                 \
        is_save = false;\
    }while(0)


#define LOG(level,format,...)                                      \
    do{                                                            \
        log_message(__FILE__,__LINE__,level,format,##__VA_ARGS__); \
    }while(0)

void save_file(std::string); //保存到文件中
std::string get_time();//获取当前时间
std::string get_level(Level); //获取日志等级

void log_message(std::string file_name,int line,Level level,const char* format,...){
    //获取当前时间
    std::string time_ = get_time();
    std::string level_ = get_level(level);
    pid_t pid = getpid();
    char buff[1024];
    va_list arg;
    va_start(arg,format);
    vsnprintf(buff,sizeof(buff),format,arg);
    va_end(arg);
    std::string message = "["+time_+"]"+"["+level_+"]"+"["+std::to_string(pid)+"]"+"["+file_name+"]"+
                            "["+std::to_string(line)+"]"+"["+buff+"]"+"\n";
    if(is_save){
        save_file(message);
    }else{
        std::cout<<message;
    }
    
}

void save_file(std::string message){
    std::ofstream outfile(file_name,std::ios::app);//追加模式
    if(!outfile.is_open()){
        return;
    }    
    outfile<<message;
    outfile.close();
}

std::string get_time(){
    time_t time_cur = time(nullptr);
    struct tm* format_time = localtime(&time_cur);
    if(format_time == nullptr) return "null";
    //格式化转为字符串
    return std::to_string(format_time->tm_year+1900)+":"
            +std::to_string(format_time->tm_mon+1)+":"
            +std::to_string(format_time->tm_mday)+":"
            +std::to_string(format_time->tm_hour)+":"
            +std::to_string(format_time->tm_min)+":"
            +std::to_string(format_time->tm_sec);
}

std::string get_level(Level level){
    switch (level)
    {
    case Level::DEBUG:
        return "debug";
        break;
    case Level::ERROR:
        return "error";
        break;
    case Level::FATAL:
        return "fatal";
        break;
    case Level::INFO:
        return "info";
        break;
    case Level::WARNING:
        return "warning";
        break;
    default:
        return "nullptr";
        break;
    }
}