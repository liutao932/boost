    #include<iostream>
    #include<string>
    #include<vector>
    #include<boost/filesystem.hpp>
    #include"util.hpp"


    const std::string src_path = "data/input/";   //所有的html
    const std::string output = "data/raw_html/raw.txt"; //解析所有完的html

    typedef  struct DocInfo54
    {
        std::string title;    //文档的标题
        std::string content;  //文档的内容
        std::string url;      //该文档在官网的url
    }DocInfo_t;

    bool EnumFile(const std::string &src_path,std::vector<std::string> *file_list);
    bool ParseHtml(std::vector<std::string>& file_list,std::vector<DocInfo_t> *results);
    bool SaveHtml(std::vector<DocInfo_t>& results,const std::string &output);



    static bool ParseTitle(const std::string &result,std::string *title) 
    {
        size_t begin = result.find("<title>");
        if(begin == std::string::npos)
        {
            return false;
        }
        size_t end = result.find("</title>");
        if(end == std::string::npos)
        {
            return false;
        }
        begin += std::string("<title>").size();
        if(begin > end)
        {
            return false;
        }
        *title = result.substr(begin,end - begin);

        return true;
    }
    static bool ParseContent(const std::string &file,std::string *content)
    {
        //去标签，基于一个简单的状态机
        enum status
        {
            LABLE,
            CONTENT
        };
        enum status s = LABLE;
        for(auto e :file)
        {
            switch (s)
            {
            case LABLE:
                if(e == '>')  //代表结束
                    s = CONTENT;
                /* code */
                break;
            case CONTENT:
                if(e == '<') //代表开始
                    s = LABLE;
                else
                {
                    if(e == '\n') e = ' '; 
                    *content += e;
                }
            break;
            default:
                break;
            }
        }
        return true;
    }
    static bool ParseUrl(const std::string &file,std::string *url)
    {
        std::string url_head = "https://www.boost.org/doc/libs/1_79_0/doc/html/";
        std::string url_tail = file.substr(src_path.size());
        *url = url_head + url_tail;
        return true;
    }

    int main()
    {
        //第一步拿到所有文件名
        std::vector<std::string> files_list;
        if(!EnumFile(src_path,&files_list))
        {
            std::cerr<<"enum file name error"<<std::endl;
            return 1;
        }
        //第二步解析文件
        std::vector<DocInfo_t> results;
        if(!ParseHtml(files_list,&results))
        {
            std::cerr<<"parse is error"<<std::endl;
            return 2;
        }
        //第三步,把解析完毕的各个文件内容，写入output,按照\3作为每个文档的分隔符
        if(!SaveHtml(results,output))
        {
            std::cerr<<"save html error"<<std::endl;
            return 3;
        }
        return 0;
    }

    bool EnumFile(const std::string &src_path,std::vector<std::string> *file_list)  //拿到所有html文件名
    {
        namespace fs = boost::filesystem;
        fs::path root_path(src_path);   //创建一个路径名对象
        if(!fs::exists(root_path))  //根据路径创建的对象不存在
        {
            std::cerr<<src_path<<"not exists"<<std::endl;
            return false;
        }
        //定义一个空迭代器，用来判断递归结束
        fs::recursive_directory_iterator end;
        for(fs::recursive_directory_iterator it(root_path); it != end; ++it)
        {
            if(!fs::is_regular_file(*it)) //如果不是普通文件继续
            {
                continue;
            }
            if(it->path().extension() != ".html")
            {
                continue;
            }
            //测试
            //std::cout<<"debug"<<it->path().string()<<std::endl;
            file_list->push_back(it->path().string());
        }

        return true;
    }
    void ShowInfo(const DocInfo_t &doc)
    {
        std::cout<<doc.title<<std::endl;
        std::cout<<doc.content<<std::endl;
        std::cout<<doc.url<<std::endl;
    }
    bool ParseHtml(std::vector<std::string>& file_list,std::vector<DocInfo_t> *results)//拿到所有html的标题，内容，url
    {
        for(const auto  file : file_list)
        {
            //1读取文件
            std::string result;
            if(!ns_util::FileUtil::ReadFile(file,&result))
            {
                //文件读取失败
                continue;
            }
            DocInfo_t doc;
            //2提取标签
            if(!ParseTitle(result,&doc.title))
            {
                continue;;
            }
            //3提取内容
            if(!ParseContent(result,&doc.content))
            {
                continue;
            }
            //4提取url
            if(!ParseUrl(file,&doc.url))
            {
                continue;
            }
            //将结果出入到vector，这里有拷贝问题，以后在优化
            results->push_back(std::move(doc)); //采用右值，资源转移
            //for debug
            //ShowInfo(doc);
            //break;
        }
        return true;
    }
    bool SaveHtml(std::vector<DocInfo_t>& results,const std::string &output)
    {
    #define SEP '\3'
        std::ofstream of(output,std::ios::out | std::ios::binary);
        if(!of.is_open())
        {
            std::cerr<<"open"<<output<<"fail"<<std::endl;
            return false;
        }
        //写入文件
        for(const auto &item : results)
        {
            std::string  out_result;
            out_result = item.title;
            out_result += SEP;
            out_result += item.content;
            out_result += SEP;
            out_result += item.url;
            out_result += '\n';

            of.write(out_result.c_str(),out_result.size());
        }
        of.close();
        return true;
    }
