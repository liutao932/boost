#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include "util.hpp"
#include <mutex>
#include"log.hpp"
namespace ns_index
{
    struct DocInfo
    {
        std::string title;   //文档标题
        std::string content; //文档对应的去标签之后的内容
        std::string url;     //官网的url
        uint64_t doc_id;     //文档的id
    };

    struct InvertedElem
    {
        uint64_t doc_id;
        std::string word;
        int weigth;
    };

    //倒排拉链
    typedef std::vector<InvertedElem> InvertedList;

    class Index
    {
    private:
        std::vector<DocInfo> forward_index;                           //正排索引
        std::unordered_map<std::string, InvertedList> inverted_index; //倒排索引
        static Index *Instance;
        static std::mutex mtx;

    private:
        Index() = default;
        Index(const Index &) = delete;
        Index &operator=(const Index &) = delete;

    public:
        ~Index() = default;
        static Index *GetInstance()
        {
            if (nullptr == Instance)
            {
                mtx.lock();
                if (nullptr == Instance)
                {
                    Instance = new Index();
                }
                mtx.unlock();
            }

            return Instance;
        }
        //根据doc_id找到文档内容
        DocInfo *GetForWardIndex(uint64_t doc_id)
        {
            if (doc_id >= forward_index.size())
            {
                std::cerr << "doc_id is error" << std::endl;
                return nullptr;
            }
            return &forward_index[doc_id];
        }
        //根据关键字string,获得倒排拉链
        InvertedList *GetInvertedList(const std::string &word)
        {
            auto it = inverted_index.find(word);
            if (it == inverted_index.end())
            {
                std::cerr << word << "have no InvertedList" << std::endl;
                return nullptr;
            }
            return &(it->second);
        }
        //根据去标签，格式化之后的文档，构建正排索引和倒排索引
        // data/raw_html/raw.txt
        bool BuildIndex(const std::string &input)
        {
            std::ifstream in(input, std::ios::in | std::ios::binary);
            if (!in.is_open())
            {
                std::cerr << "sorry" << input << "open sorry" << std::endl;
                return false;
            }
            std::string line;

            int count = 0;
            while (std::getline(in, line))
            {
                DocInfo *doc = BuildForwardIndex(line); //构建正排
                if (doc == nullptr)
                {
                    std::cerr << "build" << line << std::endl; // for debug
                    continue;
                }
                BuildInvertedIndex(*doc);
                count++;
                if(count % 50 == 0) //std::cout<<"当前已经建立的索引文档："<<count<<std::endl;
                LOG(NORMAL, "当前的已经建立的索引文档: " + std::to_string(count));
            }
            return true;
        }

    private:
        DocInfo *BuildForwardIndex(const std::string &line)
        {
            // 1进行字符串切分
            std::vector<std::string> results;
            const std::string seq = "\3";
            ns_util::StringUtil::Split(line, &results, seq);
            if (results.size() != 3)
            {
                return nullptr;
            }
            // 2将字符串进行填充到DocInfo
            DocInfo doc;
            doc.title = results[0];
            doc.content = results[1];
            doc.url = results[2];
            doc.doc_id = forward_index.size();
            // 3插入到正排索引的vector中
            forward_index.push_back(std::move(doc));
            return &forward_index.back();
        }
        bool BuildInvertedIndex(const DocInfo &doc)
        {
            // word 倒排拉链
            struct word_cnt
            {
                /* data */
                int title_cnt;
                int content_cnt;
                word_cnt() : title_cnt(0), content_cnt(0) {}
            };
            std::unordered_map<std::string, word_cnt> word_map;
            //对标题进行分词
            std::vector<std::string> title_words;
            ns_util::JiebaUtil::CurString(doc.title, &title_words);
            for (auto s : title_words)
            {
                boost::to_lower(s); //转化成小写
                word_map[s].title_cnt++;
            }
            //对文档内容进行分词
            std::vector<std::string> contnet_word;
            ns_util::JiebaUtil::CurString(doc.content, &contnet_word);
            for (auto s : contnet_word)
            {
                boost::to_lower(s); //转化成小写
                word_map[s].content_cnt++;
            }
#define X 10
#define Y 1
            for (auto &word_pair : word_map)
            {
                InvertedElem item;
                item.doc_id = doc.doc_id;
                item.word = word_pair.first;
                //相关性
                item.weigth = X * word_pair.second.title_cnt + Y * word_pair.second.content_cnt;
                InvertedList &inverted_list = inverted_index[word_pair.first];
                inverted_list.push_back(std::move(item));
            }
            return true;    
        }
    };
    Index* Index::Instance = nullptr;
    std::mutex Index::mtx;
}