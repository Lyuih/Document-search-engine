#pragma once

/**
yui的搜索引擎搜索篇
目前已经有了解析好的数据以及索引的建立
搜索的步骤：建立好索引，获取需要搜索的语句（query）,对query进行分词,获取到分词后的结果
就可以先去根据封好的词语去获取倒排拉链,倒排拉链中存储了包含该字词的id和该id下的权重。
因为一个query可能包含多个字词，为了后续的排序，需要再构建一个结构体，该结构体的属性有id、所有字词权重和、所有字词
对结构体进行排降序即可得到最终搜索后的结果
最后根据id获取正排索引进行打印（json化和内容接取）

 Json::Value root;
*/
#include <jsoncpp/json/json.h>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <string>
#include "Log.hpp"
#include "index.hpp"


/**
 * 存储query分词后字词在id中的权重和，以及记录字词
 */
class InvertedElemPrint{
public:
    InvertedElemPrint(uint64_t id = 0,int weight = 0)
    :id_(id)
    ,weight_(weight)
    {}
public:
    uint64_t id_;
    int weight_;
    std::vector<std::string> words_;
};

// const std::string input = "./data/raw_html/raw.txt";

/**
 * searcher具有的属性：Index、
 */
class Searcher{
private:
    Index* index;
public:
    Searcher(){}
    ~Searcher(){}
    void init_search(const std::string input){
        //初始化，创建index，建立索引
        index = Index::get_instance();
        LOG(Level::INFO,"创建单例模式");
        index->create_index(input);
        LOG(Level::INFO,"创建索引");
    }

    //开始进行搜索，需要的参数：搜索语句，返回值json_res(输入输出型)
    void search(const std::string query,std::string& json_res){
        /**
         * 先对搜索语句进行分词，然后根据分词的内容获取倒排拉链
         * 有了倒排拉链后，根据倒排拉链开始计算搜索语句在不同id下的权重和，利用哈希表存储
         * 后续把哈希表中的value 转储到数组中，对数组进行排序（desc）
         * 然后根据数组内容，获取正排索引，在截取部分内容后建立json串
         */
        std::vector<std::string> words;
        JiebaUtil::CutString(query,&words);//开始进行分词
        std::vector<InvertedElemPrint> inverted_all;
        std::unordered_map<uint64_t,InvertedElemPrint> cnt;
        for(std::string&word:words){
            //获取倒排拉链
            boost::to_lower(word);//转小写
            inverted_list* invertedList = index->get_inverted_index(word);
            if(nullptr == invertedList){
                continue;
            }
            for(Inverted_item&item:(*invertedList)){
                InvertedElemPrint& tmp_elem = cnt[item.id_];
                tmp_elem.id_ = item.id_;
                tmp_elem.weight_+=item.weight_;
                tmp_elem.words_.push_back(item.word_);
            }
        }
        for(auto&item:cnt){
            inverted_all.push_back(item.second);
        }
        //进行排序 desc
        std::sort(inverted_all.begin(),inverted_all.end(),[=](InvertedElemPrint&a,InvertedElemPrint&b){
            return a.weight_>b.weight_;
        });

        //排完序后，开始获取正排索引
        Json::Value root;
        for(InvertedElemPrint&item:inverted_all){
            Doc* tmp = index->get_forward_index(item.id_);
            if(nullptr == tmp){
                continue;
            }
            //得到正排索引
            Json::Value value;
            value["title"] = tmp->title_;
            // value["content"] = GetDesc(tmp->content_,item.words_[0]);
            value["content"] = GetDescWithHighlight(tmp->content_, item.words_); 
            value["url"] = tmp->url_;
            root.append(value);
        }
        Json::FastWriter write;
        json_res = write.write(root);
    }

    std::string GetDesc(const std::string& html_content,const std::string&word)
    {
      //节选部分内容
      const int prev_step = 50;
      const int next_step = 100;

      //1.找到首次出现的位置
      auto iter = std::search(html_content.begin(),html_content.end(),word.begin(),word.end(),[](int x,int y)
          {
          return (std::tolower(x) == std::tolower(y));
          });
      if(iter == html_content.end())
      {
        return "None1";
      }
      int pos = std::distance(html_content.begin(),iter);

      //2.获取start，end下标
      int start = 0;
      int end = html_content.size() - 1;
      //判断前方是否有足够字符
      if(pos>start+prev_step) start = pos - prev_step;
      if(pos<end-next_step) end = pos+next_step;

      // 3.截取字串
      if(start>=end) return "None2";
      std::string desc = html_content.substr(start,end-start);
      desc+="...";
      return desc;
    }
    // 优化，带有语法高亮
    std::string GetDescWithHighlight(const std::string& html_content, const std::vector<std::string>& words)
    {
        if (words.empty()) {
            // 如果没有关键词，截取开头部分
            return html_content.substr(0, 150) + "...";
        }

        const std::string& first_word = words[0]; // 简单起见，我们只高亮第一个词

        // 1. 不区分大小写地查找首次出现的位置
        auto it = std::search(html_content.begin(), html_content.end(), first_word.begin(), first_word.end(), 
            [](char c1, char c2) {
                return std::tolower(c1) == std::tolower(c2);
            });

        if (it == html_content.end()) {
            // 如果找不到，返回一个通用摘要
            return html_content.substr(0, 150) + "...";
        }

        int pos = std::distance(html_content.begin(), it);
        
        // 2. 确定截取范围
        const int prev_step = 50;
        const int next_step = 100;
        
        int start = (pos > prev_step) ? (pos - prev_step) : 0;
        int end = (pos + first_word.length() + next_step < html_content.length()) ? (pos + first_word.length() + next_step) : html_content.length();

        // 3. 截取原始子串
        std::string desc = html_content.substr(start, end - start);

        // 4. 在截取出的摘要中，高亮所有出现的关键词
        // 这里为了简单，只高亮第一个词
        std::string highlighted_word = "<em>" + first_word + "</em>";
        
        // 替换 (这里是一个简化的替换，实际中可能需要更复杂的正则替换来处理大小写)
        size_t found_pos = desc.find(first_word);
        if(found_pos != std::string::npos){
            desc.replace(found_pos, first_word.length(), highlighted_word);
        }
        
        return "..." + desc + "...";
    }

};

