#include "nsemq_base.h"
TopicList topic_list_;  // topic and consume callback mapping list.
// 实现结构体的序列化，返回内存大小
int nsemq_encode(void *msg_struct, char **msg_buf, char **msg_type){
    BaseType * msg_base;        // 所有数据结构体的基类型指针
    int struct_size = 0;        // 结构体大小
    int buf_size = 0;           // 字符数组的内存大小

    // 使用BaseType类型访问结构体变量，可获取变量值
    msg_base = (BaseType *)msg_struct;
    struct_size = msg_base->get_size(msg_base);
    *msg_type = msg_base->get_type(msg_base);

    // 根据结构体的内存大小，分配内存，并完成序列化
    buf_size = sizeof(char) * struct_size;
    *msg_buf = (char *)malloc(buf_size);
    avro_writer_t writer = avro_writer_memory(*msg_buf, buf_size);
    msg_base->serialize(writer, msg_base);
    return buf_size;
}

// 实现字符串的反序列化
void* nsemq_decode(char *msg_buf, int buf_size, deserialize_func d_func){
    // 1. 读取固定长度的内存
    avro_reader_t reader = avro_reader_memory(msg_buf, buf_size);
    // 2. 根据变量的类型，调用反序列化的函数
    return d_func(reader);
}

// consumer callback
void nsemq_consume_callback(rd_kafka_message_t *rkmessage, void *opaque){
    char *msg_buf;
    int msg_size = 0;
    char *msg_type;
    char *topic_name;
    void *msg_data;
    // 0. get the core parameter.
    msg_type = rkmessage->key;
    msg_buf = rkmessage->payload;
    msg_size = rkmessage->len;
    topic_name = (char *)rd_kafka_topic_name(rkmessage->rkt);
    // printf("received msg_type:%s, msg_size:%d\n", msg_type, msg_size);
    // printf("received topic_name:%s\n", topic_name);
    // 1. search the callback function from topic_list, Judging the validity.
    TopicList topicNode = find_item(topic_list_, topic_name);
    if(!topicNode){
        nsemq_write_error("receive message from unknown topics.");
        return;
    }
    // 2. decode buffer to struct object.
    msg_data = nsemq_decode(msg_buf, msg_size, topicNode->deserialize_func);
    if(!msg_data){
        nsemq_write_error("invalid data received.");
        return;
    }
    // 3. call user-defined callback function
    topicNode->consume_callback(msg_data, topic_name, msg_type);
}

/*** public function ***/
BOOL nsemq_judge_connect(rd_kafka_t *handle){
    const struct rd_kafka_metadata *metadata;
    rd_kafka_resp_err_t err = rd_kafka_metadata(handle, 1, NULL, &metadata, 1000);
    return (err ==  RD_KAFKA_RESP_ERR_NO_ERROR)?TRUE:FALSE;
}

void nsemq_write_error(char *errstr_){
    fprintf(stderr,"%% [NseMQ][ERR] %s\n", errstr_);
}

void nsemq_write_info(char *infostr){
    fprintf(stderr,"%% [NseMQ][INFO] %s\n", infostr);
}

/*** list function ***/
void insert_list(TopicList *topic_list,
                 const char *topic_name,
                 rd_kafka_topic_t *topic_object,
                 void* (*deserialize_func)(void*),   // deserialize function
                 void (*consume_callback)(void *, char *, char *)) {
    TopicList curNode = *topic_list;
    TopicList newNode = malloc(sizeof(struct TopicNode));
    if (!newNode) {
        return;
    }
    newNode->topic_name = (char *)topic_name;
    newNode->topic_object = topic_object;
    newNode->deserialize_func= deserialize_func;
    newNode->consume_callback = consume_callback;
    newNode->next = NULL;
    // if link list is empty, add new node directly
    if (!*topic_list) {
        *topic_list = newNode;
        return;
    }
    // traverse to last node, add a new node.
    while (curNode->next) {
        curNode = curNode->next;
    }
    // add the new node to the end of the list
    curNode->next = newNode;
}

void display_list(TopicList topic_list) {
    TopicList curNode = topic_list;
    while (curNode) {
        printf("%s ", curNode->topic_name);
        curNode = curNode->next;
    }
    printf("\n");
}

TopicList find_item(TopicList topic_list,const char *topic_name) {
    while (topic_list) {
        if (strcmp(topic_list->topic_name, topic_name) == 0) {
            return topic_list;
        }
        topic_list = topic_list->next;
    }
    return NULL;
}

void delete_item(TopicList *topic_list,const char *topic_name) {
    TopicList p1, p2 = NULL;
    if (!*topic_list) {
        return;
    }
    p1 = *topic_list;
    while (p1 != NULL) {
        if (strcmp(p1->topic_name, topic_name) != 0) {
            p2 = p1;
            p1 = p1->next;
        } else {
            break;
        }
    }
    if (p1 == *topic_list) { // first node
        *topic_list = p1->next;
        free(p1);
    } else if (p1 != NULL) { // middle node
        p2->next = p1->next;
        free(p1);
    } else {
        printf("not exist.\n");
    }
}

void clear_list(TopicList *topic_list) {
    TopicList curNode, preNode = NULL;
    curNode = *topic_list;
    while (curNode) {
        preNode = curNode;
        curNode = curNode->next;
        free(preNode);  // free the topic node memory.
    }
}