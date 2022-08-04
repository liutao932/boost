#pragma once
#include "index.hpp"
#include <algorithm>
#include <jsoncpp/json/json.h>
// struct Com
// {
//     bool operator>(const InvertedElem& e1,const InvertedElem& e2)
//     {
//         return e1.weigth > e2.weigth;
//     }
// }
struct InvertedElemPrint
{
    uint64_t doc_id;
    int weight;
    std::vector<std::string> words;
    InvertedElemPrint() : doc_id(0), weight(0) {}
};
namespace ns_searcher
{
    class Searcher
    {
    private:
        ns_index::Index *index;

    public:
        Searcher() = default;
        ~Searcher() = default;
        void InitSearcher(const std::string &input)
        {
            // 1.获取或者创建index对象
            index = ns_index::Index::GetInstance(); //获得单例
            //std::cout << "获取单例成功" << std::endl;
            LOG(NORMAL, "获取index单例成功...");
            // 2.根据index对象建立索引
            index->BuildIndex(input);
           // std::cout << "建立正排和倒排索引成功...." << std::endl;
            LOG(NORMAL, "建立正排和倒排索引成功...");
        }

        std::string GetDesc(const std::string &html_src, const std::string &word)
        {
            const int prev_step = 50;
            const int next_step = 100;
            //找到首次出现的位置
            // std::size_t pos = html_src.find(word);  //错误原文档没有忽略大小写
            auto it = std::search(html_src.begin(), html_src.end(), word.begin(), word.end(),
                                  [](int a, int b)
                                  { return std::tolower(a) == std::tolower(b); });
            int pos = std::distance(html_src.begin(), it);
            if (pos == std::string::npos)
            {
                return "None1"; //不存在这种情况
            }
            // 2获取start end
            int start = 0;
            int end = html_src.size() - 1;

            if (pos > start + prev_step)
                start = pos - prev_step;
            if (pos < end - next_step)
                end = pos + next_step;

            if (start >= end)
                return "None2";
            return html_src.substr(start, end - start) + "...";
        }
        // query：搜索关键字
        // josn_string:返回给用户的搜索结果
        void Search(const std::string &query, std::string *json_string)
        {
            // 1.[分词]：对我们的query进行按照searcher的要求进行分词
            std::vector<std::string> words;
            ns_util::JiebaUtil::CurString(query, &words);
            // 2.[触发]：就是根据分词的各个“词”，进行Index查找
            // ns_index::InvertedList inverted_list_all;
            std::vector<InvertedElemPrint> inverted_list_all;

            std::unordered_map<uint64_t, InvertedElemPrint> tokens_map;
            for (auto &e : words)
            {
                boost::to_lower(e);
                ns_index::InvertedList *inverted_list = index->GetInvertedList(e);
                if (inverted_list == nullptr)
                    continue;
                //不完美的地方，可能有重复的文档
                //  inverted_list_all.insert(inverted_list_all.end(),inverted_list->begin(),inverted_list->end());
                for (const auto &elem : *inverted_list)
                {
                    auto &item = tokens_map[elem.doc_id]; //[]:如果存在直接获取，如果不存在新建
                    // item一定是doc_id相同的print节点
                    item.doc_id = elem.doc_id;
                    item.weight += elem.weigth;
                    item.words.push_back(elem.word);
                }
                for (const auto &elem : *inverted_list)
                {
                    auto &item = tokens_map[elem.doc_id]; //[]:如果存在直接获取，如果不存在新建
                    // item一定是doc_id相同的print节点
                    item.doc_id = elem.doc_id;
                    item.weight += elem.weigth;
                    item.words.push_back(elem.word);
                }
                for (const auto &item : tokens_map)
                {
                    inverted_list_all.push_back(std::move(item.second));
                }
            }
            // 3.[合并排序]：汇总查找结果，按照相关性（weight）降序排序
            //  std::sort(inve rted_list_all.begin(), inverted_list_all.end(),\
            //               []( const ns_index::InvertedElem e1,  const ns_index::InvertedElem e2){
            //                return e1.weigth > e2.weigth;
            //                });
            //  std::sort(inverted_list_all.begin(),inverted_list_all.end(),Com());

            std::sort(inverted_list_all.begin(), inverted_list_all.end(),
                      [](const InvertedElemPrint &e1, const InvertedElemPrint &e2)
                      {
                          return e1.weight > e2.weight;
                      });
            // 4.[构建]：根据查找出来的结果，构建jsonc串 -----jsoncpp
            Json::Value root;
            for (auto &item : inverted_list_all)
            {
                ns_index::DocInfo *doc = index->GetForWardIndex(item.doc_id);
                if (doc == nullptr)
                    continue;

                Json::Value elem;
                elem["title"] = doc->title;
                elem["desc"] = GetDesc(doc->content, item.words[0]); // content是文档的去标签的结果，但是不是我们想要的，我们要的是一部分 TODO
                elem["url"] = doc->url;
                // for deubg, for delete
                elem["id"] = (int)item.doc_id;
                elem["weight"] = item.weight; // int->string

                root.append(elem);
            }

            Json::StyledWriter writer;
            *json_string = writer.write(root);
        }
    };
}