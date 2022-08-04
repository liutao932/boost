#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include "cppjieba/Jieba.hpp"
namespace ns_util
{
    class FileUtil
    {
    public:
        static bool ReadFile(const std::string &file_path, std::string *out)
        {
            std::ifstream in(file_path, std::ios::in);
            if (!in.is_open())
            {
                std::cerr << "open file" << file_path << std::endl;
                return false;
            }
            std::string line;
            while (std::getline(in, line)) // getline返回的是一个对象，但是这个对象重载了强制类型装换
            {
                *out += line;
            }
            return true;
        }
    };
    class StringUtil
    {
    private:
    public:
        static void Split(const std::string &line, std::vector<std::string> *out, const std::string seq)
        {
            boost::split(*out, line, boost::is_any_of(seq), boost::token_compress_on); //字符串切分
        }
    };



    const char *const DICT_PATH = "./dict/jieba.dict.utf8";
    const char *const HMM_PATH = "./dict/hmm_model.utf8";
    const char *const IDF_PATH = "./dict/idf.utf8";
    const char *const STOP_WORD_PATH = "./dict/stop_words.utf8";
    const char *const USER_DICT_PATH = "./dict/user.dict.utf8";

    class JiebaUtil 
    {  
    private:
        static cppjieba::Jieba jieba;
    public: 
        static void CurString(const std::string &src, std::vector<std::string> *out)
        {
            jieba.CutForSearch(src, *out);
        }
    };
    cppjieba::Jieba JiebaUtil::jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH);
    //cppjieba::Jieba JiebaUtil::jieba(DICT_PATH,HMM_PATH,IDF_PATH,STOP_WORD_PATH,USER_DICT_PATH);
}
