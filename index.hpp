#pragma once

/*
yui的boost搜索引擎，索引篇
在parser.cc文件中，已经把boost官方库的索引html内容提取并排列出来了 位于./data/raw_html/raw.txt
索引篇的目标：建立正排索引和倒排索引
建立索引类（单例模式，防止重复建立索引）：
属性：利用数组存储的正排索引、利用哈希表存储的倒排索引、静态的索引指针、静态的共享锁
正排索引，id -> 内容。利用数组下标充当id，内容为html的title content url （构建结构体）
倒排索引，内容 -> id。 一个内容可以指向多个id(有点类似于邻接表的结构)。
为了实现这个结构 哈希表的key为内容，value为对应id的数组 值得一提的是数组中存储不是单纯的id，而是对应id中该内容及该内容的权重 所以还要构造一个结构体。

方法：
public：建立索引，根据关键词 获取倒排拉链
private：建立正排索引、建立倒排索引
其他。。。

建立索引：读取raw.txt文件，然后建立正排索引、倒排索引
建立正排索引：根据输入的一行内容，提取出 title content url 构建id 返回结构体doc
构建倒排索引：对doc进行词频统计（tiitl content）在统计前要进行分词 分完词开始计算权重


*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>
#include "Log.hpp"
#include "tools.hpp"

class Doc{
public:
    Doc(){}
    Doc(std::string title,std::string content,std::string url,uint64_t id)
    :title_(title)
    ,content_(content)
    ,url_(url)
    ,id_(id)
    {}
public:
    std::string title_;
    std::string content_;
    std::string url_;
    uint64_t id_;
};

class Inverted_item{
public:
    Inverted_item(){}
    Inverted_item(uint64_t id,std::string word,int weight = 0)
    :id_(id)
    ,word_(word)
    ,weight_(weight)
    {}
public:
    uint64_t id_;
    std::string word_;
    int weight_;
};

using inverted_list = std::vector<Inverted_item>;

class Index{
private:
    std::vector<Doc> forward_index; // 正排索引
    std::unordered_map<std::string,inverted_list> inverted_index; // 倒排索引
public:
    
private:
    Index(){}
    Index(const Index&) = delete;
    Index& operator=(const Index&) = delete;
public:
    ~Index(){}
    static Index* get_instance(){
        if(instance == nullptr){
            mtx.lock();
            if(instance == nullptr){
                instance = new Index();
            }
            mtx.unlock();
        }
        return instance;
    }
    bool create_index(const std::string input){
        //创建索引
        /**
        input为raw.txt文件的路径
        后续按行读取
        读取成功数据后，将读取成功的数据先建立正排索引然后再建立倒排索引
         */
        std::ifstream ifm(input,std::ios::in);
        if(!ifm.is_open()){
            // LOG(FATAL,"%s文件打开失败",input.c_str());
            // LOG(FATAL,"111");
            return false;
        }
        //开始读取数据
        std::string data_line;
        int count = 0;
        while(std::getline(ifm,data_line)){
            //读取一行数据后开始建立正排索引，同时接收返回值
            // Doc* doc_item = create_forward_index(data_line);
            // if(nullptr == doc_item){
            //     // LOG(WARNING,"正排索引创建失败");
            //     continue;
            // }
            if(create_forward_index(data_line) == false){
                LOG(Level::WARNING,"正排索引创建失败");
                continue;
            }
            Doc* doc_item = &forward_index.back();
            //根据建立正排索引的返回值，开始建立倒排索引
            create_inverted_index(*doc_item);//传递的地址
            // if(count++<100){
                LOG(Level::DEBUG,"建立索引成功%d",++count);
            // }
        }
        return true;
    }

    bool create_forward_index(std::string&data_line){
        //一个文件内的数据在raw.txt只占有一行，不同属性间的分隔符为'\3'
        //利用boost库中的split函数可以快速得到被分隔符分开后的内容
        std::vector<std::string> res;
        split_string(data_line,res,"\3");
        if(res.size()!=3){
            // LOG(WARNING,"切割字符串失败");
            return false;
        }
        //切割成功
        Doc tmp(res[0],res[1],res[2],forward_index.size()); // 定义一个临时的Doc变量，后续可以直接右值move
        forward_index.push_back(std::move(tmp));
        return true;
    }

    void create_inverted_index(const Doc& doc){
        //建立倒排索引
        //需要对doc属性中的title content 进行分词，还要进行词频统计。注意字词全转小写，可以只有boost库的to_lower函数
        //建立一个结构体，属性为title_num content_num.分别表示一个词分别在标题和正文出现的次数
        struct word_num{
            int title_num = 0;
            int content_num = 0;
        };
        //定义一个哈希表来映射一个词在文中出现的频率
        std::unordered_map<std::string,word_num> word_cnt;
        //先统计title，统计前先分词
        std::vector<std::string> title_word;
        JiebaUtil::CutString(doc.title_,&title_word);
        for(std::string&word:title_word){
            boost::to_lower(word);
            word_cnt[word].title_num+=1;
        }
        //后统计content
        std::vector<std::string> content_word;
        JiebaUtil::CutString(doc.content_,&content_word);
        for(std::string&word:content_word){
            boost::to_lower(word);
            word_cnt[word].content_num+=1;
        }

        //统计完词频后，开始计算权重，定义标题中出现的权重为5，正文中出现的权重为1
        #define TITLE 5
        #define CONTENT 1
        
        for(auto&item:word_cnt){
            int weight = item.second.title_num*TITLE+item.second.content_num*CONTENT;
            Inverted_item tmp(doc.id_,item.first,weight);
            
            inverted_list& tmp_list = inverted_index[item.first];
            tmp_list.push_back(std::move(tmp));
        }
    }

    //根据id查看正排索引
    Doc* get_forward_index(uint64_t id){
        if(id>=forward_index.size()){
            LOG(Level::WARNING,"id超出范围");
            return nullptr;
        }
        return &forward_index[id];
    }

    //根据字词返回倒排拉链
    inverted_list* get_inverted_index(const std::string& word){
        auto iter = inverted_index.find(word);
        if(iter == inverted_index.end()){
            //没找到
            LOG(Level::WARNING,"字词对应的倒排拉链未找到");
            return nullptr;
        }
        return &(inverted_index[word]);
    }
private:
    static Index* instance;
    static std::mutex mtx;
};

Index* Index::instance = nullptr;
std::mutex Index::mtx;