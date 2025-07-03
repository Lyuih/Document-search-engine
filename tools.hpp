#pragma once
#include <boost/algorithm/string.hpp>
#include "cppjieba/Jieba.hpp"
#include <string>
#include <vector>
#include "Log.hpp"
#include <mutex>
/**
boost库的字符串切割函数
头文件：#include <boost/algorithm/string.hpp>

void split(std::vector<std::string>& result, 
           const std::string& input, 
           const boost::algorithm::detail::is_any_ofF<char>& separator, 
           boost::algorithm::token_compress_mode_type compress = boost::algorithm::token_compress_off);

- `result`：用于存储分割后的子字符串的 `std::vector<std::string>` 。
- `input`：待分割的字符串。
- `separator`：分隔符，可以是单个字符，也可以是多个字符。
- `compress`：
    - `boost::algorithm::token_compress_on`：合并多个相邻的分隔符，避免空字符串。
    - `boost::algorithm::token_compress_off`（默认）：不会合并分隔符，相邻分隔符会导致空字符串。
*/
void split_string(std::string&str,std::vector<std::string>&res,const std::string sep){
    //参数：要分割的字符串，存储去除分隔符后的内容的数组，分隔符
    boost::split(res,str,boost::is_any_of(sep),boost::algorithm::token_compress_on);
}


const char* const DICT_PATH = "./dict/jieba.dict.utf8";
const char* const HMM_PATH = "./dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "./dict/user.dict.utf8";
const char* const IDF_PATH = "./dict/idf.utf8";
const char* const STOP_WORD_PATH = "./dict/stop_words.utf8";

 class JiebaUtil
  {
    private:
      cppjieba::Jieba jieba;
      std::unordered_map<std::string,bool> stop_words;
    private:
      JiebaUtil()
        :jieba(DICT_PATH,HMM_PATH,USER_DICT_PATH,IDF_PATH,STOP_WORD_PATH)
      {}
      JiebaUtil(const JiebaUtil&) = delete ;
    public:
      static JiebaUtil* get_instance()
      {
        if(nullptr == instance)
        {
          mtx.lock();
          if(nullptr == instance)
          {
            instance = new JiebaUtil();
            instance->InitJiebaUtil();
          }
          mtx.unlock();
        }
        return instance;
      }
      void InitJiebaUtil()
      {
        std::ifstream in(STOP_WORD_PATH);
        if(!in.is_open())
        {
          LOG(Level::FATAL,"load stop wordsfile error");
          return;
        }
        std::string line;
        while(std::getline(in,line))
        {
          stop_words.insert({line,true});
        }
        in.close();
      }
      void CutStringHelper(const std::string& src,std::vector<std::string>*out)
      {
        jieba.CutForSearch(src,*out);
        for(auto iter = out->begin();iter != out->end();)
        {
          auto it = stop_words.find(*iter);
          if(it!=stop_words.end())
          {
            //说明当前的string是暂停词，需要去除
            iter = out->erase(iter);
          }
          else
          {
            iter++;
          }
        }
      }
    public:
      static void CutString(const std::string&src,std::vector<std::string>*out)
      {
        JiebaUtil::get_instance()->CutStringHelper(src,out);
      }
    private:
      static JiebaUtil* instance;
      static std::mutex mtx;
  };
  JiebaUtil* JiebaUtil::instance = nullptr;
  std::mutex JiebaUtil::mtx;
