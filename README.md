# NseMQ
NseMQ是NSE实验室内部使用的消息中间件，用于完成消息的统一封装和传输。
NseMQ基于`C`实现，集成了目前流行的 [Kafka](http://kafka.apachecn.org/) 消息系统和 [Avro](http://avro.apache.org/) 序列化工具，
底层使用`Kafka`的C/C++客户端库 [librdkafka](https://github.com/edenhill/librdkafka) 实现， 屏蔽了大量的使用细节，简单易用。

> 目前主要使用C版本NseMQ，C++版本仍在完善中。

## 特点

* 实现了消息`发布-订阅`通信模式；
* 封装简化`librdkafka`和`Avro`库函数，并提供依赖的库文件；
* 屏蔽了数据的`序列化/反序列化`过程，即生产者直接发送某一结构体对象，消费者接收处理对应的结构体对象；

## 安装

下载安装Kafka的过程，请参照 [Kafka快速开始](http://kafka.apache.org/quickstart/) ，此处仅介绍NseMQ库的安装及使用。

下载最新版的NseMQ发布版本，解压后执行以下步骤：

> 以Visual Studio为例

1. 将include路径添加到项目属性中的`C/C++ - 常规 - 附加包含目录`；
2. 将lib路径添加到项目属性中的`链接器 - 常规 - 附加库目录`；
3. 向`链接器 - 输入 - 附加依赖项`添加如下内容：

```c
librdkafka.lib
pthreadVC2.lib
nsemq.lib
```

此时已经完成库环境的配置，接下来只需要在使用NseMQ库的头文件中引用`nsemq.h`即可。

## 使用

### 数据准备

NseMQ库中的数据结构使用JSON文件进行定义，可通过`bin目录`工具中的`avrogen.jar`生成对应的.h和.c头文件。

> 注：使用`avrogen.jar`前，需要配置好jdk环境。

例如，若需要定义一个`cpx`结构体对象，可通过如下步骤。

* 创建`cpx.json`文件，写入如下JSON描述语句。

```json
{
    "type": "record", 
    "name": "cpx",
    "fields" : [
        {"name": "re", "type": "double"},    
        {"name": "im", "type" : "double"},
        {"name": "s", "type": {
            "type": "record",
            "name": "Person",
            "fields": [
                {"name": "name", "type": "string"},
                {"name": "age", "type": "int"}
            ]
        }},
        {"name": "arr", "type": {
            "type": "array",
            "items": "string"
        }}
    ]
}
```
上述JSON描述语句中，定义了`cpx`结构体及其相关属性。

值得注意的是，结构体可以实现嵌套，如`cpx`结构体中嵌套了`Person`结构体。

* 进入`avrogen.jar`根目录，执行如下语句生成.h和.c文件。

```shell script
java -jar avrogen.jar cpx.json . cpx
```

上述语句中，`cpx.json`指定输入的.json文件，`.`指定输出的.h和.c的目录，`cpx`指定源的名称。

生成的详细数据结构请参考 [cpx.h](c/examples/cpx.h) , [cpx.c](c/examples/cpx.c) 。

### 数据生产

生产者头文件 [nsemq_producer.h](c/include/nsemq_producer.h)，完成数据的序列化和发送。

> 注：NseMQ的API函数，通常返回`ErrorCode`错误回执码，函数执行成功返回`ERR_NO_ERROR`，其他回执码详见 [nsemq_base.h](c/include/nsemq_base.h) 。

**生产者API**

```c
ErrorCode nsemq_producer_init(const char *broker_addr, void *dr_msg_cb);  // initialize producer
ErrorCode nsemq_producer_produce(void *msg, const char *topic_name);      // produce message with 'msg' and topic 'topic_name'
ErrorCode nsemq_producer_close();                                         // close producer and clear memory
```

**生产者范例**

```c
#include "nsemq.h"
#include "cpx.h"
// delivery report callback function
void produce_callback(char *msg_topic, void *msg_data, int msg_size){
    printf("this is produce_callback!\n");
    fprintf(stderr,"%% Message delivered (%d bytes, topic %s)\n",
    msg_size, msg_topic);
}
// main function
int main(){
    if(nsemq_producer_init("localhost:9092", produce_callback) != ERR_NO_ERROR){
        printf("initialize failed!\n");
        return -1;
    }
    
    person = nse_person_create();
    person->name = kaa_string_copy_create("nse");
    person->age = 24;
    
    nsemq_producer_produce(person, topic_name);
    nsemq_producer_close();
    return 0;
}
```
使用`produce`生产数据后，将生产的数据插入本地队列后（等待发送）立即返回，不提示是否发送成功。
可通过自身实现`produce_callback`函数，实现消息发送成功的回调。

### 数据消费

消费者头文件 [nsemq_consumer.h](c/include/nsemq_consumer.h) ，完成数据的接收和反序列化。

**消费者API**

```c
ErrorCode nsemq_consumer_init(const char *broker_addr);       // initialize consumer
ErrorCode nsemq_consumer_subscribe(const char *topic_name,    // subscribe topic and bind consume callback
                                   const char *msg_type,
                                   void (*consume_callback)(void *, char *, char *));
ErrorCode nsemq_consumer_unsubscribe(const char *topic_name); // unsubscribe topic
ErrorCode nsemq_consumer_subscription(char **topic_array, int *topic_count);  // get subscribed topic names.
ErrorCode nsemq_consumer_start();           // start to consume message from broker.
ErrorCode nsemq_consumer_close();           // close the consumer.
```

* 使用时，需要用户自定义回调函数，实现数据的处理；
* 一个`topic`对应一个消费回调函数`consume_callback`，需要在订阅时将两者绑定。

**消费者范例:**
```c
// consumer callback function.
void msg_callback(void *msg_data, char *msg_topic, char *msg_type){
    if(strcmp(msg_type, "nse_cpx") == 0){
        nse_cpx_t *cpx2 = (nse_cpx_t *)msg_data;
        printf("cpx2: im: %lf, re: %lf, name: %s, age: %d\n",
                cpx2->im, cpx2->re, cpx2->s->name->data, cpx2->s->age);
        printf("cpx2 msg_topic:%s\n",msg_topic);
    }
}
// main function
int main(){
    if(nsemq_consumer_init("localhost:9092") != ERR_NO_ERROR) {
        return -1;
    }
    
    nsemq_consumer_subscribe("test", nse_cpx, msg_callback1);
    nsemq_consumer_subscribe("test1", nse_person, msg_callback1);

    nsemq_consumer_start();
    // keep the main() running for 20s
    while(time_count < 20){
        Sleep(1000);
        time_count++;
    }
    nsemq_consumer_close();
}
```
消费者异步的从Broker拉取数据，当接收数据后可以正常触发回调函数。

**注意：**
1. 需要维持主程序处于运行状态，数据消费的进程才能正常运行（待修复）;
2. 不要在回调函数中执行长时间的处理程序，否则会堵塞数据接收。