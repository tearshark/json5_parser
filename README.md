# json5_parser
一个支持**json5**的json解析器。<br>解析器和DOM拆分为两个部分，以便于外部定制DOM实现。<br>

可以通过开关 **JSON_ENABLE_JSON5** 控制是否支持**json5**。<br>

#### 一、问题

目前已知问题：对字符串标准支持较弱。<br>

#### 二、效率

在I7 8700K 3.7GHz(OC 4.3GHz)，使用VS 2019 / clang 9.0.0编译(取成绩好的编译器结果），测试data目录下附带的三个json，获得数据如下：<br>

###### 1、关闭json5选项
```
dummy
benchmark_json5_parser parse file: .\data\canada.json
parse 2251060 bytes cost 0.706516 s, speeds is 607.708 MB/s
benchmark_json5_parser parse file: .\data\twitter.json
parse 631514 bytes cost 0.12996 s, speeds is 926.84 MB/s
benchmark_json5_parser parse file: .\data\citm_catalog.json
parse 1727204 bytes cost 0.222991 s, speeds is 1477.36 MB/s

DOM
benchmark_json5_parser parse file: .\data\canada.json
parse 2251060 bytes cost 0.997353 s, speeds is 430.495 MB/s
benchmark_json5_parser parse file: .\data\twitter.json
parse 631514 bytes cost 0.138747 s, speeds is 868.142 MB/s
benchmark_json5_parser parse file: .\data\citm_catalog.json
parse 1727204 bytes cost 0.24073 s, speeds is 1368.5 MB/s

benchmark_rapidjson parse file: ./data/canada.json
parse 2251060 bytes cost 0.74908 s, speeds is 573.177 MB/s
benchmark_rapidjson parse file: ./data/twitter.json
parse 631514 bytes cost 0.304658 s, speeds is 395.367 MB/s
benchmark_rapidjson parse file: ./data/citm_catalog.json
parse 1727204 bytes cost 0.37469 s, speeds is 879.227 MB/s
```
<br>

| 项目                | 编译器 | json文件          | 文件大小（Bytes） | 重复次数 | 用时(秒) | 速度(MB/S)  |
| :------------------ | :----: | :---------------- | :---------------: | :------: | :------: | :---------: |
| json5_parser(dummy) |  msvc  | canada.json       |      2251060      |   200    | 0.706516 | **607.708** |
| json5_parser(dummy) |  msvc  | twitter.json      |      631514       |   200    | 0.125344 | **926.84**  |
| json5_parser(dummy) |  msvc  | citm_catalog.json |      1727204      |   200    | 0.218263 | **1477.36** |
| json5_parser(DOM)   |  msvc  | canada.json       |      2251060      |   200    | 0.997353 | **430.495** |
| json5_parser(DOM)   |  msvc  | twitter.json      |      631514       |   200    | 0.138747 | **868.142** |
| json5_parser(DOM)   |  msvc  | citm_catalog.json |      1727204      |   200    | 0.24073  | **1368.5**  |
| rapidjson           | clang  | canada.json       |      2251060      |   200    | 0.74908  | **573.177** |
| rapidjson           | clang  | twitter.json      |      631514       |   200    | 0.304658 | **395.367** |
| rapidjson           | clang  | citm_catalog.json |      1727204      |   200    | 0.37469  | **879.227** |

<br>

###### 2、开启json5选项
```
dummy
benchmark_json5_parser parse file: .\data\canada.json
parse 2251060 bytes cost 0.756322 s, speeds is 567.689 MB/s
benchmark_json5_parser parse file: .\data\twitter.json
parse 631514 bytes cost 0.158458 s, speeds is 760.148 MB/s
benchmark_json5_parser parse file: .\data\citm_catalog.json
parse 1727204 bytes cost 0.299909 s, speeds is 1098.46 MB/s

DOM
benchmark_json5_parser parse file: .\data\canada.json
parse 2251060 bytes cost 1.04293 s, speeds is 411.682 MB/s
benchmark_json5_parser parse file: .\data\twitter.json
parse 631514 bytes cost 0.165559 s, speeds is 727.547 MB/s
benchmark_json5_parser parse file: .\data\citm_catalog.json
parse 1727204 bytes cost 0.307497 s, speeds is 1071.35 MB/s
```
<br>

| 项目                | 编译器 | json文件          | 文件大小（Bytes） | 重复次数 | 用时(秒) | 速度(MB/S)  |
| :------------------ | :----: | :---------------- | :---------------: | :------: | :------: | :---------: |
| json5_parser(dummy) |  msvc  | canada.json       |      2251060      |   200    | 0.756322 | **567.689** |
| json5_parser(dummy) |  msvc  | twitter.json      |      631514       |   200    | 0.158458 | **760.148** |
| json5_parser(dummy) |  msvc  | citm_catalog.json |      1727204      |   200    | 0.299909 | **1098.46** |
| json5_parser(DOM)   |  msvc  | canada.json       |      2251060      |   200    | 1.04293  | **411.682** |
| json5_parser(DOM)   |  msvc  | twitter.json      |      631514       |   200    | 0.165559 | **727.547** |
| json5_parser(DOM)   |  msvc  | citm_catalog.json |      1727204      |   200    | 0.307497 | **1071.35** |

<br>

按照习惯，走一波广告：

如果你发现了任何bug、有好的建议、或使用上有不明之处，可以提交到issue，也可以直接联系作者:

```
email: tearshark@163.net
QQ交流群: 296561497
```

