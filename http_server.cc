#include "cpp-httplib/httplib.h"
#include "searcher.hpp"
#include "Log.hpp"

const std::string input = "data/raw_html/raw.txt";
const std::string root_path = "./wwwroot";

int main()
{
    able_save();
    Searcher searcher;
    searcher.init_search(input);
    httplib::Server svr;
    svr.set_base_dir(root_path.c_str());//添加主网页
    svr.Get("/s",[&searcher](const httplib::Request& req, httplib::Response& res) {
        if(!req.has_param("query"))
        {
            res.set_content("param query is required", "text/plain");
            return;
        }
        std::string query = req.get_param_value("query");
        LOG(INFO,"query:%s",query.c_str());
        std::string json_string;    
        searcher.search(query,json_string);
        res.set_content(json_string,"application/json; charset=utf-8");
    });
    LOG(INFO,"start server");
    svr.listen("0.0.0.0", 8080);
    return 0;
}
