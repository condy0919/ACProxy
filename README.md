What does AC mean?
==================

坑太多...

TODO
------------------
* 监控服务内连接情况, 将数据打进 influxdb 内，采用 grafana 来展示
* 远端 Connection close 之间，让相应的连接也断掉.
* Socket pool(reference count)
* Filter
* Cache

BUGS
------------------
* 有时 Logger::~Logger 会跪，coredump文件定位到 std::string::_M_append 有问题，无法复现 _(:з」∠)_  目前预先 reserve 了，暂时没有再出现问题.


Benchmark
------------------
100k 请求，50 并发下测试
Program    | P95 (ms) | Request/s
nginx(raw) |    5     | 16400.91
squid      |    9     | 6330.65
acproxy(4) |   11     | 6258.72
acproxy(1) |   16     | 3846.09
mproxy     |   31     | 2265.60
