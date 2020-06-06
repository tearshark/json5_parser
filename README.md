# json5_parser
一个支持**json5**的json解析器。<br>

可以通过开关 **JSON_ENABLE_JSON5** 控制是否支持**json5**。<br>

##### 一、问题

目前已知问题：对字符串标准支持较弱。<br>

##### 二、效率

在I7 8700K 3.7GHz(OC 4.3GHz)，使用VS 2019 / clang 9.0.0编译(取成绩好的编译器结果），测试data目录下附带的三个json，获得数据如下：<br>

###### 1、关闭json5选项
```
benchmark_parser_json parse file: .\data\canada.json
parse 2251060 bytes cost 0.966729 s, speeds is 444.132 MB/s
benchmark_parser_json parse file: .\data\twitter.json
parse 631514 bytes cost 0.125344 s, speeds is 960.97 MB/s
benchmark_parser_json parse file: .\data\citm_catalog.json
parse 1727204 bytes cost 0.218263 s, speeds is 1509.36 MB/s

benchmark_rapidjson parse file: ./data/canada.json
parse 2251060 bytes cost 0.72091 s, speeds is 595.575 MB/s
benchmark_rapidjson parse file: ./data/twitter.json
parse 631514 bytes cost 0.305112 s, speeds is 394.779 MB/s
benchmark_rapidjson parse file: ./data/citm_catalog.json
parse 1727204 bytes cost 0.363635 s, speeds is 905.959 MB/s
```

| 项目         | 编译器 | json文件          | 文件大小（Bytes） | 重复次数 | 用时(秒) | 速度(MB/S)  |
| :----------- | :----: | :---------------- | :---------------: | :------: | :------: | :---------: |
| json5_parser |  msvc  | canada.json       |      2251060      |   200    | 0.966729 | **444.132** |
| json5_parser |  msvc  | twitter.json      |      631514       |   200    | 0.125344 | **960.97**  |
| json5_parser |  msvc  | citm_catalog.json |      1727204      |   200    | 0.218263 | **1509.36** |
| rapidjson    | clang  | canada.json       |      2251060      |   200    | 0.72091  | **595.575** |
| rapidjson    | clang  | twitter.json      |      631514       |   200    | 0.305112 | **394.779** |
| rapidjson    | clang  | citm_catalog.json |      1727204      |   200    | 0.363635 | **905.959** |

<br>

###### 2、开启json5选项
```
benchmark_parser_json parse file: .\data\canada.json
parse 2251060 bytes cost 1.05525 s, speeds is 406.875 MB/s
benchmark_parser_json parse file: .\data\twitter.json
parse 631514 bytes cost 0.139481 s, speeds is 863.57 MB/s
benchmark_parser_json parse file: .\data\citm_catalog.json
parse 1727204 bytes cost 0.278296 s, speeds is 1183.77 MB/s
```

| 项目         | 编译器 | json文件          | 文件大小（Bytes） | 重复次数 | 用时(秒) | 速度(MB/S)  |
| :----------- | :----: | :---------------- | :---------------: | :------: | :------: | :---------: |
| json5_parser |  msvc  | canada.json       |      2251060      |   200    | 1.05525  | **406.875** |
| json5_parser |  msvc  | twitter.json      |      631514       |   200    | 0.139481 | **863.57**  |
| json5_parser |  msvc  | citm_catalog.json |      1727204      |   200    | 0.278296 | **1183.77** |

<br>

按照习惯，走一波广告：

如果你发现了任何bug、有好的建议、或使用上有不明之处，可以提交到issue，也可以直接联系作者:

```
email: tearshark@163.net
QQ交流群: 296561497
```

