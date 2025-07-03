/**
 * yui的boost搜索引擎的初始化与解析官方HTML的内容
 * 功能：
 * 根据官方文档，提取出其中的html文件，对html文件进行解析
 * 提取出html文件的title、content、url内容
 * 其中url内容为自己构建：https://www.boost.org/doc/libs/1_78_0/doc/html+文件名
 * 然后将每个html文件中提取出来的内容，用\3作为分隔符，将title、content、url分隔开
 * 最后将其保存在data/raw_html/raw.txt
 * 
 * 定义html文件在data/input目录下
 * 定义保存在data/raw_html/raw.txt
 * 
 *:cosnt &表示输入
 *:* 表示输出
 *:& 表示输入输出
内容解析与预处理的步骤：
1.提取出所有HTML的文件路径. 提示使用boost库的文件系统，更方便/头文件 #include <boost/filesystem.hpp>
2.根据提取出的文件路径读取文件，将html文件内容解析，解析为title、content、url。
用结构体保存解析内容。同时用一个数组存储该结构体。
3.将保存结构体数组消息保存到raw.txt文件中，注意结构体内属性的分隔符为\3 不同html的分隔符为\n
 */
#include <boost/filesystem.hpp>
#include <fstream>
#include<iostream>
#include <string>
#include <vector>
#include"Log.hpp"

//定义html存储路径、最后内容的保存位置
const std::string html_src_path = "data/input";
const std::string html_save_path = "data/raw_html/raw.txt";

//定义结构体
class file_content{
public:
    file_content(){}
    file_content(std::string title,std::string content,std::string url)
        :title_(title)
        ,content_(content)
        ,url_(url)
        {}
public:
    std::string title_;
    std::string content_;
    std::string url_;
};
/*
 *:cosnt &表示输入
 *:* 表示输出
 *:& 表示输入输出
*/
/*
提取目标路径下的html文件路径
参数：输入参数目标路径，输入输出参数带回html路径的数组
*/
bool extract_html(const std::string&src_path,std::vector<std::string>&htmls);

/*
解析html文件
需要的参数有：存储了html路径的数组，存储了file_content的数组（输入输出）
*/
bool parse_html(const std::vector<std::string>&htmls,std::vector<file_content>&file_contents);

/**
 * 保存所有文件的file_content的内容
 * 需要的参数：file_contents
 */
bool save_file(const std::vector<file_content>&file_contents);

bool get_title(const std::string& file_path,std::string& title);
bool get_content(const std::string& file_path,std::string& content);
bool get_url(const std::string& file_path,std::string& url);

std::string get_file_all(const std::string file_path);

int main()
{
    able_save();
    std::vector<std::string> htmls;
    if(!extract_html(html_src_path,htmls)){
        LOG(FATAL,"提取html文件路径失败");
        exit(1);
    }
    LOG(INFO,"提取html文件路径成功");
    std::vector<file_content> file_contents;
    if(!parse_html(htmls,file_contents)){
        LOG(FATAL,"解析html文件失败");
        exit(2);
    }
    LOG(INFO,"解析html文件成功");
    if(!save_file(file_contents)){
        LOG(FATAL,"存储html内容失败");
        exit(3);
    }
    LOG(INFO,"存储html内容成功");
    return 0;
}

bool extract_html(const std::string&src_path,std::vector<std::string>&htmls){
    /**
     * 利用boost库中的文件系统操作，namespace fs = boost::filesystem 简化操作
     * 具体操作为，先定义一个path对象（fs::path xxx），检查后，在定义一个空迭代器，用于判断递归的结束(fs::recursive_directory_iterator end)
     * fs::recursive_directory_iterator iter(xxx)头迭代器
     * 判断是否为普通文件（fs::is_regular_file(*iter)）
     * 判断是否后缀为html(iter->path().extension()!=".html")
     */
    namespace fs = boost::filesystem;
    fs::path root_path(src_path);//初始化根节点
    if(!fs::exists(root_path)){
        LOG(FATAL,"根节点不存在");
        return false;
    }
    //定义迭代器
    fs::recursive_directory_iterator end;
    // int num = 0;
    for(fs::recursive_directory_iterator iter(root_path);iter!=end;++iter){
        //筛选出满足条件的html普通文件
        if(!fs::is_regular_file(*iter)){
            continue;//跳过
        }
        if(iter->path().extension()!=".html"){
            continue;//跳过
        }

        //未跳过的文件都是满足要求的,存储到htmls中
        htmls.push_back(iter->path().string());
        // if(num++<10)
        // LOG(DEBUG,"%s",iter->path().string().c_str());
    }
    return true;
}



bool parse_html(const std::vector<std::string>&htmls,std::vector<file_content>&file_contents){
    /**
     * 解析html文件，分3步：
     * 提取title
     * 提取content
     * 构建url
     */
    if(htmls.empty()){
        LOG(WARNING,"htmls内容为空");
        return false;
    }
    // int num = 0;
    for(const std::string& file_path:htmls){
        std::string title,content,url;
        if(!get_title(file_path,title)){
            LOG(WARNING,"%s文件title提取失败",file_path.c_str());
        }
        if(!get_content(file_path,content)){
            LOG(WARNING,"%s文件content提取失败",file_path.c_str());
        }
        if(!get_url(file_path,url)){
            LOG(WARNING,"%s文件url构建失败",file_path.c_str());
        }
        // if(num++>10) break;
        file_content item(title,content,url);
        file_contents.push_back(item);
    }
    return true;
}

std::string get_file_all(const std::string file_path){
    std::ifstream ifs(file_path,std::ios::in);//读取
    if(!ifs.is_open()){
        return "";
    }
    std::string tmp,res;
    while(getline(ifs,tmp)){ //按行读取文件内容
        res+=tmp;
    }
    return res;
}

bool get_title(const std::string& file_path,std::string& title){
    //读取文件内容
    std::string file_all = get_file_all(file_path);
    // LOG(DEBUG,"%s",file_all.c_str());
    if(file_all.empty()){
        return false;
    }
    //开始提取title内容
    //<title> xxx </title>
    size_t sz = std::string("<title>").size();
    size_t begin = file_all.find("<title>");
    if(begin == std::string::npos){
        return false;
    }
    size_t end = file_all.find("</title>");
    if(end == std::string::npos){
        return false;
    }
    if(begin>=end){
        return false;
    }
    size_t len = end-begin-sz;
    title = file_all.substr(begin+sz,len);
    // LOG(DEBUG,"%s",title.c_str());
    return true;
    
}
bool get_content(const std::string& file_path,std::string& content){
    //读取文件内容
    std::string file_all = get_file_all(file_path);
    if(file_all.empty()){
        return false;
    }
    content.clear();
    //提取内容，比较难，html文件中有众多的标签。
    //<>xxx<> 为了提取出内容，就必须要知道，目前属于什么状态(遍历提取需要的字符)
    //可以写一个简易的状态机，一共两种状态：标签状态(label)、内容状态
    enum State{
        LABEL,
        BODY
    };
    State state = LABEL;
    for(char c:file_all){
        if(state == LABEL){
            if(c == '>'){
                state = BODY;
            }
        }else if(state == BODY){
            if(c == '<'){
                state = LABEL;
            }else{
                if(c == '\n') c = ' ';
                content+=c;
            }
        }
    }
    // LOG(DEBUG,"%d内容：%s",file_all.size(),content.c_str());
    return true;
}
bool get_url(const std::string& file_path,std::string& url){
    std::string head = "https://www.boost.org/doc/libs/1_78_0/doc/html";
    // size_t begin = file_path.find("data/input");
    size_t sz = std::string("data/input").size();
    std::string tail = file_path.substr(sz);
    url = head+tail;
    return true;
}

bool save_file(const std::vector<file_content>&file_contents){
    //分隔符
    const std::string SEP = "\3";
    std::ofstream ofm(html_save_path,std::ios::app);
    if(!ofm.is_open()){
        return false;
    }
    for(const file_content& item:file_contents){
        std::string tmp = item.title_+SEP+item.content_+SEP+item.url_+'\n';
        ofm.write(tmp.c_str(),tmp.size());
    }
    ofm.close();
    return true;
}