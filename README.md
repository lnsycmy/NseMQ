# NseMQ
NseMQ是NSE实验室内部使用的消息中间件，用于完成消息的统一封装和传送。
NseMQ基于`C++`实现，集成了目前流行的`Kafka`消息系统和`Avro`序列化工具，屏蔽了大量的使用细节，简单易用。

## 特点

* 实现了`发布-订阅`通信模式；
* 屏蔽数据的序列化过程；

## 安装


## 使用
NseMQ的API函数，通常返回`NseMQ::ErrorCode`错误回执码，函数执行成功返回`ERR_NO_ERROR`，其他回执码详见`NseMqHandle.h`。

### 数据生产

生产者类`NseMqProducer`，完成数据的序列化和发送。

**生产者API**

```c++
NseMQ::ErrorCode NseMqProducer::init(std::string broker_addr);
NseMQ::ErrorCode NseMqProducer::produce(T &t, std::string topic_name);
NseMQ::ErrorCode NseMqProducer::close();
```

**生产者范例-1**

```c++
NseMqProducer producer;
producer.init("localhost:9092");

NseMQ::cpx c1;
c1.re = 1.2;
c1.im = 2.4;

if(producer.produce<NseMQ::cpx>(c1, "test_topic") == NseMQ::ERR_NO_ERROR){
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
```c++
// implement class RdKafka::DeliveryReportCb
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

消费者类`NseMqConsumer`，完成数据的接收和反序列化。

**消费者API**

```c++
NseMQ::ErrorCode NseMqConsumer::init(std::string broker_addr);
NseMQ::ErrorCode NseMqConsumer::subscribe(std::string topic_name,
                                          RdKafka::ConsumeCb &consume_cb,
                                          int64_t start_offset /* optional property */);
NseMQ::ErrorCode NseMqConsumer::unSubscribe(std::string topic_name);
NseMQ::ErrorCode NseMqConsumer::subscription(std::vector<std::string> &topics);
NseMQ::ErrorCode NseMqConsumer::start();
NseMQ::ErrorCode NseMqConsumer::pause();
NseMQ::ErrorCode NseMqConsumer::resume();
NseMQ::ErrorCode NseMqConsumer::close();
NseMQ::ErrorCode NseMqConsumer::poll();
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
也可自己实现线程循环调用`poll()`函数，范例如下。

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
NseMqProducer类对象和NseMqConsumer类对象都可以调用通用接口。

**通用API**
```c++
bool judgeConnection();                                 // test connection with broker.
void getBrokerTopics(std::vector<std::string> &topics); // get all topics from broker.
```

**通用接口范例**
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