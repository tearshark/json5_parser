# json5_parser
一个支持**json5**的json解析器。<br>

可以通过开关 **JSON_ENABLE_JSON5** 控制是否支持**json5**。<br>

##### 一、问题

目前已知问题：对字符串标准支持较弱。<br>

##### 二、效率

在I7 8700K 3.7GHz(OC 4.3GHz)，使用clang 9.0.0编译，测试data目录下附带的三个json，获得数据如下：<br>

###### 1、关闭json5选项
```
benchmark_parser_json parse file: ./data/canada.json
parse 2251060 bytes cost 0.648113 s, speeds is 331.235 MB/s
benchmark_parser_json parse file: ./data/twitter.json
parse 631514 bytes cost 0.053727 s, speeds is 1120.96 MB/s
benchmark_parser_json parse file: ./data/citm_catalog.json
parse 1727204 bytes cost 0.120617 s, speeds is 1365.64 MB/s

benchmark_rapidjson parse file: ./data/canada.json
parse 2251060 bytes cost 0.377634 s, speeds is 568.481 MB/s
benchmark_rapidjson parse file: ./data/twitter.json
parse 631514 bytes cost 0.154151 s, speeds is 390.693 MB/s
benchmark_rapidjson parse file: ./data/citm_catalog.json
parse 1727204 bytes cost 0.195189 s, speeds is 843.896 MB/s
```

| 项目         | json文件          | 文件大小（Bytes） | 重复次数 | 用时(秒) | 速度(MB/S)  |
| :----------- | :---------------- | :---------------: | :------: | :------: | :---------: |
| json5_parser | canada.json       |      2251060      |   100    | 0.648113 | **331.235** |
| json5_parser | twitter.json      |      631514       |   100    | 0.053727 | **1120.96** |
| json5_parser | citm_catalog.json |      1727204      |   100    | 0.120617 | **1365.64** |
| rapidjson    | canada.json       |      2251060      |   100    | 0.377634 | **568.481** |
| rapidjson    | twitter.json      |      631514       |   100    | 0.154151 | **390.693** |
| rapidjson    | citm_catalog.json |      1727204      |   100    | 0.195189 | **843.896** |

<br>

###### 2、开启json5选项
```
benchmark_parser_json parse file: ./data/canada.json
parse 2251060 bytes cost 0.756168 s, speeds is 283.902 MB/s
benchmark_parser_json parse file: ./data/twitter.json
parse 631514 bytes cost 0.0538533 s, speeds is 1118.33 MB/s
benchmark_parser_json parse file: ./data/citm_catalog.json
parse 1727204 bytes cost 0.116481 s, speeds is 1414.13 MB/s
```

| 项目         | json文件          | 文件大小（Bytes） | 重复次数 | 用时(秒)  | 速度(MB/S)  |
| :----------- | :---------------- | :---------------: | :------: | :-------: | :---------: |
| json5_parser | canada.json       |      2251060      |   100    | 0.756168  | **283.902** |
| json5_parser | twitter.json      |      631514       |   100    | 0.0538533 | **1118.33** |
| json5_parser | citm_catalog.json |      1727204      |   100    | 0.116481  | **1414.13** |

<br>

按照习惯，走一波广告：

如果你发现了任何bug、有好的建议、或使用上有不明之处，可以提交到issue，也可以直接联系作者:

```
email: tearshark@163.net
QQ交流群: 296561497
```

