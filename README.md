# NseMQ
NseMQ是NSE实验室内部使用的消息中间件，用于完成消息的统一封装和传输。
NseMQ基于`C++`实现，集成了目前流行的 [Kafka](http://kafka.apachecn.org/) 消息系统和 [Avro](http://avro.apache.org/) 序列化工具，
底层使用`Kafka`的C/C++客户端库 [librdkafka](https://github.com/edenhill/librdkafka) 实现， 屏蔽了大量的使用细节，简单易用。

## 特点

* 实现了消息`发布-订阅`通信模式；
* 封装简化`librdkafka`和`Avro`库函数，并提供依赖的库文件；
* 屏蔽了数据的`序列化/反序列化`过程，即生产者直接发送某一类对象，消费者接收处理对应的类对象；

## 安装
**（待完成）**

## 使用

### 数据准备

NseMQ库中的类数据对象使用JSON文件进行定义，可通过bin目录中的`avrogencpp.exe`生成对应的.hh头文件，包含类定义和`encode`, `decode`函数。

例如，若定义一个`student`类数据对象，可通过如下步骤。

* 创建`student.json`文件，写入如下JSON描述语句。

```json
{
    "type": "record", 
    "name": "student",
    "fields" : [
        {"name": "name","type" : "string",  "default": "null"},
        {"name": "age", "type" : "int",     "default": 0},
        {"name": "sex", "type" : "string",  "default": "null"}
    ]
}
```
上述JSON描述语句中，定义了`student`类及其三个属性`name`, `age`, `sex`，并为属性赋予默认值。

* 进入`avrogencpp.exe`根目录，使用如下语句生成.hh头文件。

```shell script
avrogencpp -i student.json -o student.hh -n NseMQ
```

上述语句中，`-i student.json`指定输入的.json文件，`-o student.hh`指定输出的.hh文件，`-n NseMQ`指定类的命名空间。

生成的`student.hh`详细内容请参考 [student.hh](examples/student.hh) 。

### 数据生产

生产者类 [NseMqProducer](src/NseMqProducer.h)，完成数据的序列化和发送。

> 注：NseMQ的API函数，通常返回`NseMQ::ErrorCode`错误回执码，函数执行成功返回`ERR_NO_ERROR`，其他回执码详见 [NseMqHandle.h](src/NseMqHandle.h) 。

**生产者API**

```c++
NseMQ::ErrorCode NseMqProducer::init(std::string broker_addr);          // initialize producer
NseMQ::ErrorCode NseMqProducer::produce(T &t, std::string topic_name);  // produce message with object 't' and topic 'topic_name'
NseMQ::ErrorCode NseMqProducer::close();                                // close producer and clear memory
```

**生产者范例-1**

```c++
NseMqProducer producer;
producer.init("localhost:9092");

NseMQ::student s1;
s1.name = "cmy";
s1.sex = "boy";
s1.age = 24;

if(producer.produce<NseMQ::student>(s1, "test_topic") == NseMQ::ERR_NO_ERROR){
    std::cout << "produce successful! " << std::endl;
}
```

使用`produce`生产数据后，将生产的数据插入本地队列后（等待发送）立即返回，不提示是否发送成功。
可通过调用以下`init`函数，实现消息发送成功的回调。
```c++
NseMQ::ErrorCode NseMqProducer::init(std::string broker_addr,
                                     RdKafka::DeliveryReportCb *producer_cb);
```

**生产者范例-2**

由用户定义类继承`RdKafka::DeliveryReportCb`，实现其`dr_cb()`函数，拿到参数`message`可查看发送信息。

> 注：用户定义类需要使用对象指针，如`ProducerCallback *producer_cb`，从而保证它的生命周期长于`producer`。（后续版本将无此要求）
```c++
// implement class RdKafka::DeliveryReportCb dr_cb()
class ProducerCallback : public RdKafka::DeliveryReportCb {
public:
    void dr_cb(RdKafka::Message &message){
        // If message.err() is non-zero the message delivery failed permanently for the message.
        if (message.err()){
            std::cerr << "Message delivery failed: " << message.errstr() << std::endl;
        }else{
            std::cerr << "Message delivered to topic " << message.topic_name() <<
                      " [" << message.partition() << "] at offset " <<
                      message.offset() << std::endl;
        }
    }
};

// main()
NseMqProducer producer;
ProducerCallback *producer_cb = new ProducerCallback();
producer.init("localhost:9092",producer_cb);

NseMQ::student s1;
s1.name = "cmy";
s1.sex = "boy";
s1.age = 24;

if(producer.produce<NseMQ::student>(s1, "test_topic") == NseMQ::ERR_NO_ERROR){
    std::cout << "produce successful! " << std::endl;
}
```

### 数据消费

消费者类 [NseMqConsumer](src/NseMqConsumer.h) ，完成数据的接收和反序列化。

**消费者API**

```c++
NseMQ::ErrorCode NseMqConsumer::init(std::string broker_addr);                  // initialize consumer
NseMQ::ErrorCode NseMqConsumer::subscribe(std::string topic_name,
                                          RdKafka::ConsumeCb &consume_cb,
                                          int64_t start_offset/* optional */);  // subscribe topic and bind consume callback
NseMQ::ErrorCode NseMqConsumer::unSubscribe(std::string topic_name);            // unsubscribe topic
NseMQ::ErrorCode NseMqConsumer::subscription(std::vector<std::string> &topics); // get a list of subscribed topic names.
NseMQ::ErrorCode NseMqConsumer::start();        // start to consume message from broker
NseMQ::ErrorCode NseMqConsumer::pause();        // pause the consumer thread
NseMQ::ErrorCode NseMqConsumer::resume();       // resume the consumer thread.
NseMQ::ErrorCode NseMqConsumer::close();        // close the consumer
NseMQ::ErrorCode NseMqConsumer::poll();         // self-polled to call the topic consumer callback
```

**消费者范例-1:**
```c++
// implement NseMqConsumerCallback<T>
template<class T>
class ConsumerCallback : public NseMqConsumerCallback<T>{
public:
    void consume_callback(NseMQ::student &t) {
        std::cout << "student ConsumerCallback." << std::endl;
        std::cout << "NseMQ::student cb name:" << t.name << std::endl;
        std::cout << "NseMQ::student cb age:" << t.age << std::endl;
        std::cout << "NseMQ::student cb sex:" << t.sex << std::endl;
    };
};

// main()
NseMqConsumer consumer;
consumer.init("localhost:9092");

ConsumerCallback<NseMQ::student> callback;
consumer.subscribe("test_topic", callback);

consumer.start();
```

消费者类的`start()`函数实现了多线程循环调用`poll()`函数，使得程序可以正常触发回调函数。
也可以自己实现线程循环调用`poll()`函数，范例如下。

**消费者范例-2:**
```c++
// implement NseMqConsumerCallback<T> 
// same as the above example.

// main()
NseMqConsumer consumer;
consumer.init("localhost:9092");

ConsumerCallback<NseMQ::student> callback;
consumer.subscribe("test_topic", callback);

auto threadFunction = [&consumer]() {
    while (1){
        consumer.poll();
        std::cout << "receive poll()" << std::endl;
    }
};
std::thread thread(threadFunction);
thread.join();
```


### 通用接口
`NseMqProducer`类对象和`NseMqConsumer`类对象都可以调用通用接口，实现与`broker`的通信。

**通用API**
```c++
bool judgeConnection();                                 // test connection with broker.
void getBrokerTopics(std::vector<std::string> &topics); // get all topics from broker.
```

**通用API范例**
```c++
NseMqConsumer consumer;
consumer.init("localhost:9092");

if(consumer.judgeConnection()){
    std::cout << "connect successful!" << std::endl;
}

std::vector<std::string> topics;
consumer.getBrokerTopics(&topics);
if(!topics.empty()){
    for(std::vector<std::string>::iterator iter = topics.begin(); iter != topics.end(); iter++){
        std::cout << *iter << std::endl;
    }
}
```
