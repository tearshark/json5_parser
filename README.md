# json5_parser
一个支持**json5**的json解析器。<br>

可以通过开关 **JSON_ENABLE_JSON5** 控制是否支持**json5**。<br>

##### 一、问题

?	目前已知问题：对字符串标准支持较弱。<br>

##### 二、效率

?	在I7 8700K 3.7GHz(OC 4.3GHz)，使用clang 9.0.0编译，测试data目录下附带的三个json，获得数据如下：<br>

```
benchmark_parser_json parse file: ./data/canada.json
parse 2251060 bytes cost 0.761064 s, speeds is 282.076 MB/s
benchmark_parser_json parse file: ./data/twitter.json
parse 631514 bytes cost 0.0488284 s, speeds is 1233.42 MB/s
benchmark_parser_json parse file: ./data/citm_catalog.json
parse 1727204 bytes cost 0.100364 s, speeds is 1641.22 MB/s
```

| json文件          | 文件大小（Bytes）  | 重复次数     |  用时(秒)      | 速度(MB/S)    |
| ----------------- | :---------------: | :----------: | :------------: | :----------: |
| canada.json       |      2251060      |     100      |   0.761064     | **282.076**   |
| twitter.json      |      631514       |     100      |   0.0488284    | **1233.42**   |
| citm_catalog.json |      1727204      |     100      |    0.100364    | **1641.22**   |



按照习惯，走一波广告：

如果你发现了任何bug、有好的建议、或使用上有不明之处，可以提交到issue，也可以直接联系作者:

```
email: tearshark@163.net
QQ交流群: 296561497
```

