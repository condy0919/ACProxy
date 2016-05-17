What does AC mean?
==================

坑太多...

TODO
------------------
* 监控服务内连接情况, 将数据打进 influxdb 内，采用 grafana 来展示
* 远端 Connection close 之间，让相应的连接也断掉.
* Socket pool(reference count)
* Filter
* Cache(定时功能)

BUGS
------------------
* Nothing


Benchmark
------------------
100k 请求，50 并发下测试(测试数据太小，不准确)

Program    | P95 (ms) | Request/s
-----------|----------|----------
nginx(raw) |    5     | 16400.91
squid      |    9     | 6330.65
acproxy(4) |    4     | 14947.48
acproxy(1) |    6     | 13119.73
mproxy     |   31     | 2265.60
