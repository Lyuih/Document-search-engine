# Yui Boost Searcher

Yui Boost Searcher 是一个基于 C++、Boost、cppjieba 实现的简易全文搜索引擎，支持对 Boost 官方文档的解析、索引构建和高效搜索，并带有简洁的 Web 前端界面。

## 功能特性

- **HTML 文档解析**：自动遍历并解析 Boost 官方文档 HTML 文件，提取标题、正文和 URL。
- **正排/倒排索引**：构建高效的正排索引（id->内容）和倒排索引（词->文档id+权重）。
- **分词与停用词过滤**：集成 cppjieba 分词，支持停用词过滤。
- **高亮摘要**：搜索结果自动生成摘要并高亮关键词。
- **Web 搜索接口**：基于 cpp-httplib 提供 RESTful 搜索服务，配套响应式前端页面。
- **日志系统**：自定义日志模块，支持多级别日志输出和文件保存。

## 目录结构

```
boost_searcher/
├── cppjieba/           # 分词库
├── data/
│   ├── input/          # 原始 HTML 文档
│   └── raw_html/       # 解析后的原始数据
├── dict/               # 分词词典
├── wwwroot/            # 前端页面
│   └── index.html
├── parser.cc           # 文档解析与预处理
├── index.hpp           # 索引构建
├── searcher.hpp        # 搜索逻辑
├── http_server.cc      # HTTP 搜索服务
├── tools.hpp           # 工具函数与分词封装
├── Log.hpp             # 日志系统
├── makefile
└── ...
```

## 依赖环境

- C++11 或更高
- [Boost](https://www.boost.org/)（filesystem, algorithm 等）
- [cppjieba](https://github.com/yanyiwu/cppjieba)
- [cpp-httplib](https://github.com/yhirose/cpp-httplib)  
- [jsoncpp](https://github.com/open-source-parsers/jsoncpp)

> **注意**：请确保 `dict/` 目录下已放置好 cppjieba 所需的词典文件。

## 编译与运行

1. **准备数据**  
   将 Boost 官方 HTML 文档放入 `data/input/` 目录。

2. **编译项目**  
   在项目根目录下执行：
   ```bash
   make
   ```

3. **生成原始数据**  
   运行解析程序，生成 `data/raw_html/raw.txt`：
   ```bash
   ./parser
   ```

4. **启动搜索服务**  
   启动 HTTP 搜索服务器：
   ```bash
   ./http_server
   ```
   默认监听 `0.0.0.0:8080`。

5. **访问前端页面**  
   打开浏览器访问 [http://localhost:8080/](http://localhost:8080/)，即可使用搜索功能。

## 使用说明

- 在搜索框输入关键词（如 `asio`、`shared_ptr`、`string algorithm`），点击“搜索”或回车，即可获得高亮摘要和相关文档链接。
- 点击右侧问号按钮可查看项目功能说明。

## 常见问题

- **日志文件**：日志默认输出到 `log.log`，可通过 `able_save()`/`enable_save()` 控制是否保存到文件。
- **分词词典**：如遇分词异常，请检查 `dict/` 下词典文件是否齐全。
- **Boost 依赖**：如编译报错，请确认已正确安装 Boost 开发库。

## License

MIT

---

如有建议或问题，欢迎提 Issue 或 PR！
