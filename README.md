# json5_parser
一个支持**json5**的json解析器。<br>解析器和DOM拆分为两个部分，以便于外部定制DOM实现。<br>

可以通过开关 **JSON_ENABLE_JSON5** 控制是否支持**json5**。<br>

#### 一、问题

目前已知问题：对字符串标准支持较弱。<br>

#### 二、效率

在I7 8700K 3.7GHz(OC 4.3GHz)，使用VS 2019 / clang 9.0.0编译(取成绩好的编译器结果），测试data目录下附带的三个json，获得数据如下：<br>

###### 1、关闭json5选项，开启simd选项
```
dummy
benchmark_json5_parser parse file: ./data/canada.json
parse 2251060 bytes cost 2.01487 s, speeds is 852.372 MB/s
benchmark_json5_parser parse file: ./data/twitter.json
parse 631514 bytes cost 0.42715 s, speeds is 1127.96 MB/s
benchmark_json5_parser parse file: ./data/citm_catalog.json
parse 1727204 bytes cost 0.845587 s, speeds is 1558.39 MB/s

DOM
benchmark_json5_parser parse file: ./data/canada.json
parse 2251060 bytes cost 2.40061 s, speeds is 715.41 MB/s
benchmark_json5_parser parse file: ./data/twitter.json
parse 631514 bytes cost 0.458313 s, speeds is 1051.26 MB/s
benchmark_json5_parser parse file: ./data/citm_catalog.json
parse 1727204 bytes cost 0.914645 s, speeds is 1440.73 MB/s

benchmark_rapidjson parse file: ./data/canada.json
parse 2251060 bytes cost 2.91519 s, speeds is 589.128 MB/s
benchmark_rapidjson parse file: ./data/twitter.json
parse 631514 bytes cost 1.20984 s, speeds is 398.239 MB/s
benchmark_rapidjson parse file: ./data/citm_catalog.json
parse 1727204 bytes cost 1.48519 s, speeds is 887.261 MB/s
```
注一：dummy仅测试解析器的效率，不保存任何解析结果。

注二：DOM由附带的JSON_DOMWalker实现，没有正确转义字符串，用于需要快速定位json用。

<br>

| 项目                | 编译器 | json文件          | 文件大小（Bytes） | 重复次数 | 用时(秒) | 速度(MB/S)  |
| :------------------ | :----: | :---------------- | :---------------: | :------: | :------: | :---------: |
| json5_parser(dummy) | clang  | canada.json       |      2251060      |   800    | 2.01487  | **852.372** |
| json5_parser(dummy) | clang  | twitter.json      |      631514       |   800    | 0.42715  | **1127.96** |
| json5_parser(dummy) | clang  | citm_catalog.json |      1727204      |   800    | 0.845587 | **1558.39** |
| json5_parser(DOM)   | clang  | canada.json       |      2251060      |   800    | 2.40061  | **715.41**  |
| json5_parser(DOM)   | clang  | twitter.json      |      631514       |   800    | 0.458313 | **1051.26** |
| json5_parser(DOM)   | clang  | citm_catalog.json |      1727204      |   800    | 0.914645 | **1440.73** |
| rapidjson           | clang  | canada.json       |      2251060      |   800    | 2.91519  | **589.128** |
| rapidjson           | clang  | twitter.json      |      631514       |   800    | 1.20984  | **398.239** |
| rapidjson           | clang  | citm_catalog.json |      1727204      |   800    | 1.48519  | **887.261** |

<br>

###### 2、开启json5选项，开启simd选项
```
dummy
benchmark_json5_parser parse file: ./data/canada.json
parse 2251060 bytes cost 2.37301 s, speeds is 723.732 MB/s
benchmark_json5_parser parse file: ./data/twitter.json
parse 631514 bytes cost 0.453873 s, speeds is 1061.55 MB/s
benchmark_json5_parser parse file: ./data/citm_catalog.json
parse 1727204 bytes cost 1.00962 s, speeds is 1305.19 MB/s

DOM
benchmark_json5_parser parse file: ./data/canada.json
parse 2251060 bytes cost 2.55664 s, speeds is 671.749 MB/s
benchmark_json5_parser parse file: ./data/twitter.json
parse 631514 bytes cost 0.473795 s, speeds is 1016.91 MB/s
benchmark_json5_parser parse file: ./data/citm_catalog.json
parse 1727204 bytes cost 1.08489 s, speeds is 1214.64 MB/s
```
<br>

| 项目                | 编译器 | json文件          | 文件大小（Bytes） | 重复次数 | 用时(秒) | 速度(MB/S)  |
| :------------------ | :----: | :---------------- | :---------------: | :------: | :------: | :---------: |
| json5_parser(dummy) | clang  | canada.json       |      2251060      |   800    | 2.37301  | **723.732** |
| json5_parser(dummy) | clang  | twitter.json      |      631514       |   800    | 0.453873 | **1061.55** |
| json5_parser(dummy) | clang  | citm_catalog.json |      1727204      |   800    | 1.00962  | **1305.19** |
| json5_parser(DOM)   | clang  | canada.json       |      2251060      |   800    | 2.55664  | **671.749** |
| json5_parser(DOM)   | clang  | twitter.json      |      631514       |   800    | 0.473795 | **1016.91** |
| json5_parser(DOM)   | clang  | citm_catalog.json |      1727204      |   800    | 1.08489  | **1214.64** |

<br>

按照习惯，走一波广告：

如果你发现了任何bug、有好的建议、或使用上有不明之处，可以提交到issue，也可以直接联系作者:

```
email: tearshark@163.net
QQ交流群: 296561497
```

