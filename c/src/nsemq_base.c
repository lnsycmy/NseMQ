#include "nsemq_base.h"

topic_map_t g_topic_map_;                       // topic and consume callback mapping.
void (*produce_callback)(char*, void *, int);   // produce callback.

// serialize msg_struct and return msg_buf
int nsemq_encode(void *msg_struct, char **msg_buf, char **msg_type){
    BaseType * msg_base;        // base type pointers for all data structures
    int struct_size = 0;        // struct size, get value from get_size()
    int buf_size = 0;           // buffer size, get value from avro_writer_memory()
    // use BaseType to access msg_struct
    msg_base = (BaseType *)msg_struct;
    struct_size = msg_base->get_size(msg_base);
    *msg_type = msg_base->get_type(msg_base);
    // allocate memory and complete serialization
    buf_size = sizeof(char) * struct_size;
    *msg_buf = (char *)malloc(buf_size);
    avro_writer_t writer = avro_writer_memory(*msg_buf, buf_size);
    msg_base->serialize(writer, msg_base);
    return buf_size;
}

// deserialize msg_buf and call the deserialized function
void* nsemq_decode(char *msg_buf, int buf_size, deserialize_func d_func){
    // 1. read fixed-length memory
    avro_reader_t reader = avro_reader_memory(msg_buf, buf_size);
    // 2. call the deserialization function
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
    // 1. search the callback function from topic_map, judging the validity.
    TopicItem *topicItem = map_get(&g_topic_map_, topic_name);
    if(topicItem == NULL){
        nsemq_write_error(NULL, "receive message from unknown topics.");
        return;
    }
    // 2. decode buffer to struct object.
    msg_data = nsemq_decode(msg_buf, msg_size, topicItem->deserialize_func);
    if(!msg_data){
        nsemq_write_error(NULL, "invalid data received.");
        return;
    }
    // 3. call user-defined callback function
    topicItem->consume_callback(msg_data, topic_name, msg_type);
}

// deliver report callback
void nsemq_produce_callback(rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque){
    const char * topic_name;
    if (rkmessage->err){
        nsemq_write_error(NULL, "Message delivery failed.");
        nsemq_write_error(NULL,  rd_kafka_err2str(rkmessage->err));
    }else{
        topic_name = rd_kafka_topic_name(rkmessage->rkt);
        produce_callback(rkmessage->payload, topic_name, rkmessage->len);
    }
}

/*** judge connection function ***/
BOOL nsemq_judge_connect(rd_kafka_t *handle){
    const struct rd_kafka_metadata *metadata;
    rd_kafka_resp_err_t err = rd_kafka_metadata(handle, 1, NULL, &metadata, 1000);
    return (err ==  RD_KAFKA_RESP_ERR_NO_ERROR)?TRUE:FALSE;
}

/*** inner logger function ***/
void nsemq_write_error(const rd_kafka_t *rk, char *errstr){
    rd_kafka_log_print(rk, LOG_ERR, "NseMQ", errstr);
}

void nsemq_write_info(const rd_kafka_t *rk, char *infostr){
    rd_kafka_log_print(rk, LOG_INFO, "NseMQ", infostr);
}

void nsemq_write_debug(const rd_kafka_t *rk, char *debugstr){
    rd_kafka_log_print(rk, LOG_DEBUG, "NseMQ", debugstr);
}