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

JSON描述语句中，支持的类型包括：`int, long, float, double, boolean, string, bytes, array, fixed, enum, union, null`。

值得注意的是，结构体可以实现嵌套，如`cpx`结构体中嵌套了`Person`结构体。
此种情况下，需要对变量`s`进行初始化，保证在序列化前为其分配内存。


* 进入`avrogen.jar`根目录，执行如下语句生成.h和.c文件。

```shell script
java -jar avrogen.jar -schema cpx.json -output .  -filename cpx
```

上述语句中，
* `-schema cpx.json`指定输入的.json文件路径；
* `-output .`指定输出的.h和.c的目录；
* `-filename cpx`指定输出.h和.c的文件名；

生成的详细数据结构请参考 [cpx.h](c/examples/cpx.h) , [cpx.c](c/examples/cpx.c) 。

生成结构体时，需要注意以下3个类型的赋值方式：
* `string`可以使用`kaa_string_copy_create(const char *data)`赋值；
* `bytes`可以使用`kaa_bytes_copy_create(const uint8_t *data, size_t data_len)`赋值；
* `fixed`可以使用`kaa_fixed_copy_create(const uint8_t *data, size_t data_len)`赋值；

### 数据生产

生产者头文件 [nsemq_producer.h](c/include/nsemq_producer.h)，完成数据的序列化和发送。

> 注：NseMQ的API函数，通常返回`ErrorCode`错误回执码，函数执行成功返回`ERR_NO_ERROR`，其他回执码详见 [nsemq_base.h](c/include/nsemq_base.h) 。

**生产者API**

```c
ErrorCode nsemq_producer_init(const char *broker_addr, 
                              dr_cb_func dr_msg_cb);            // initialize producer and bind deliver report function.
ErrorCode nsemq_producer_produce(void *msg, const char *topic); // produce message with 'msg' to topic with `topic'.
ErrorCode nsemq_producer_close();                               // close the producer handle.
```

**使用说明**

数据生产的流程：`初始化生产者` -> `生产消息` -> `关闭生产者`。

* 初始化生产者时，需要指定`broker`服务器的ip地址和端口号，如`"localhost:9092"`；
* 生产消息时，传入生产的消息对象以及消息所属的主题即可，内部实现消息对象的序列化；
    * 消息对象由用户自己创建和销毁，例如创建`nse_cpx_t *cpx = nse_cpx_create()`以及销毁`cpx->destroy(cpx)`；
    * 若需要在生产数据后接收消息传递的反馈回调，可在初始化函数中传入消息传递回调函数`dr_msg_cb`，其函数类型如下；
    * 若无需接收消息传递回调，初始化函数中的`dr_msg_cb`参数传入`NULL`即可。
```c
/**
 * Used to define the deliver report callback function.
 * @param msg_topic: topic of the deliver message.
 * @param msg_data: message buffer which have been delivered.
 * @param msg_size: size of message buffer.
 */
void dr_cb_func(const char *msg_topic, void *msg_data, int msg_size){ ... }
```
* 生产完成后，需要调用`close()`函数关闭生产者，在此函数内部会销毁为生产者分配的内存。

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
    // 1. use nse_person_create() to create data object.
    nse_person_t *person;
    person = nse_person_create(); 
    person->name = kaa_string_copy_create("nse");
    person->age = 24;

    // 2. initialize the producer with broker address and producer callback function.
    if(nsemq_producer_init("localhost:9092", produce_callback) != ERR_NO_ERROR){
        printf("initialize failed!\n");
        return -1;
    }

    // 3. produce one data message `person` to the topic `topic_test`.
    nsemq_producer_produce(person, "topic_test");

    // 4. destroy the data object, and close the producer.
    person->destroy(person);
    nsemq_producer_close();
    return 0;
}
```
使用`produce`生产数据后，将生产的数据插入本地队列后（等待发送）立即返回，不提示是否发送成功。
可通过实现`produce_callback`回调函数，获取消息传递到`broker`的成功通知。

### 数据消费

消费者头文件 [nsemq_consumer.h](c/include/nsemq_consumer.h) ，完成数据的接收和反序列化。

**消费者API**

```c
ErrorCode nsemq_consumer_init(const char *broker_addr);         // initialize consumer with one broker address.
ErrorCode nsemq_consumer_subscribe(const char *topic_name,      // subscribe topic and bind consume callback.
                                   macro_t msg_type,            // @param msg_type:message struct name, i.e. nse_cpx
                                   msg_cb_func msg_callback);   // @param consume message callback function pointer.
ErrorCode nsemq_consumer_unsubscribe(const char *topic);        // unsubscribe one topic.
ErrorCode nsemq_consumer_subscriptions(list_t *topic_list);     // get subscribed topic names.
ErrorCode nsemq_consumer_start(int async);                      // start to consume message from broker.
ErrorCode nsemq_consumer_stop();                                // stop to consume message from broker.
ErrorCode nsemq_consumer_close();                               // close the consumer.
```

**使用说明**

数据消费的流程：`初始化消费者` -> `订阅主题` -> `开始消费` -> `触发回调` -> `结束消费` -> `关闭消费者`。

* 初始化消费者时，需要指定`broker`服务器的ip地址和端口号，如`"localhost:9092"`；
* 订阅主题时，将主题`topic`、消息类型`msg_type`和消费回调函数`msg_callback`三者绑定；
    * 消息类型`msg_type`不是字符串（不加引号），而是宏定义类型，例如`nse_cpx`，而非`"nse_cpx"`；
    * 需要自定义消费回调函数（如`msg_callback`），函数内可以处理接收到的数据，消费回调函数的类型如下；
    * 一个主题对应一个消息类型，多个主题可以使用同一个消费回调函数，可以通过调用多次`subscribe()`函数订阅多个主题；
```c
/**
 * Used to define the consume callback function.
 * @param msg_data: Serialized message data, can be converted to other data types.
 * @param msg_topic: consume the topic of this message.
 * @param msg_type: message data type, i.e. "nse_cpx".
 */
void msg_callback(void *msg_data, char *msg_topic, char *msg_type){ ... }
``` 
* 开始消费时，指定消费模式是同步`NSEMQ_SYNC`还是异步`NSEMQ_ASYNC`，接收到数据后可以正常触发回调函数；
    * 同步消费：堵塞主程序，直到调用`stop()`函数时才能停止消费，继续执行主程序代码；
    * 异步消费：不堵塞主程序，消费者在独立的线程中接收消息，但要维持主程序处于运行状态；
* 消费过程后，可调用`stop()`函数停止消费，之后可再次调用`start()`函数恢复消费；
* 消费完成后，调用`close()`函数关闭消费者，在此函数内部会销毁为消费者分配的内存。

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
    int time_count = 0;

    // 1. initialize consumer with the broker addrsss.
    if(nsemq_consumer_init("localhost:9092") != ERR_NO_ERROR) {
        return -1;
    }

    // 2. subscribe two topic, bind data type and callback function 'msg_callback'
    nsemq_consumer_subscribe("test", nse_cpx, msg_callback);
    nsemq_consumer_subscribe("test1", nse_person, msg_callback);

    // 3. start to consume message from broker asynchronously.
    nsemq_consumer_start(NSEMQ_ASYNC);

    // 4. keep main() running for 100s to ensure asynchronous consumption.
    while(time_count < 100){
        if(time_count == 10){ // stop consumption at 10s
            nsemq_consumer_stop();
        }else if(time_count == 20){ // resume consumption at 20s
            nsemq_consumer_start(NSEMQ_ASYNC);
        }
        Sleep(1000);
        time_count++;
    }
    
    // 5. close the consumer.
    nsemq_consumer_close();
}
```

**注意：** 不要在回调函数中执行长时间的处理程序，否则会堵塞数据接收。