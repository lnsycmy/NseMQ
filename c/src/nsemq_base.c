#include "nsemq_base.h"

// Global veriable,
topic_map_t g_topic_map_;       // topic and consume callback mapping.
dr_cb_func  produce_callback;   // user-defined produce_callback.
int         log_level;          // current log level.
static char strtemp_[512];      // inner function str.

// serialize msg_struct and return msg_buf, msg_type and buf_size
int nsemq_encode(void *msg_struct, char **msg_buf, char **msg_type){
    BaseType * msg_base;        // base type pointers for all data structures
    int struct_size = 0;        // struct size, get value from get_size()
    int buf_size = 0;           // buffer size, get value from avro_writer_memory()
	avro_writer_t writer;
    // use BaseType to access msg_struct
    msg_base = (BaseType *)msg_struct;
    struct_size = msg_base->get_size(msg_base);
    *msg_type = msg_base->get_type(msg_base);
    // allocate memory and complete serialization
    buf_size = sizeof(char) * struct_size;
    *msg_buf = (char *)malloc(buf_size);
    writer = avro_writer_memory(*msg_buf, buf_size);
    msg_base->serialize(writer, msg_base);
    return buf_size;
}

// deserialize msg_buf and call the deserialized function
void* nsemq_decode(char *msg_buf, int buf_size, ds_msg_func d_func){
    // 1. read fixed-length memory
    avro_reader_t reader = avro_reader_memory(msg_buf, buf_size);
    // 2. call the deserialization function
    return (void *)d_func(reader);
}

// deliver report callback function pointer,  called internally.
void nsemq_produce_callback(rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque){
    if (rkmessage->err){
        nsemq_write_error(NULL, "Message delivery failed.");
        nsemq_write_error(NULL,  (char *)rd_kafka_err2str(rkmessage->err));
    }else if(produce_callback != NULL){
        produce_callback((char *)rd_kafka_topic_name(rkmessage->rkt), rkmessage->payload, rkmessage->len);
    }
}

// consumer callback function, called internally.
void nsemq_consume_callback(rd_kafka_message_t *rkmessage, void *opaque){
    int msg_size = 0;   // from message
    char *msg_type;     // from message
    char *msg_buf;      // from message
    char *topic_name;   // from message
    void *msg_data;     // need to decode msg_buf
	TopicItem *topic_item;  // get topic_item by topic_name
    char *data_type;    // from topic_item
    // 0. get the topic name, used to get the topic_item.
    topic_name = (char *)rd_kafka_topic_name(rkmessage->rkt);
    // 1. search the topic_item from topic_map, judging the validity.
    topic_item = map_get(&g_topic_map_, topic_name);
    if(topic_item == NULL){
        nsemq_write_debug(NULL, "receive message from unknown topics.");
        return;
    }
    // 2. judging type consistency. if so, decode buffer to struct object.
    msg_type = (char *)rkmessage->key;
    data_type = topic_item->bind_data_type;
    if(msg_type && data_type && (strcmp(msg_type, data_type) == 0)) {  // received data is consistent with deserialize function
        msg_buf = rkmessage->payload;
        msg_size = rkmessage->len;
        msg_data = nsemq_decode(msg_buf, msg_size, topic_item->deserialize_func);
        if(!msg_data){
            nsemq_write_error(NULL, "invalid data received.");
            return;
        }
        // call user-defined callback function
        topic_item->consume_callback(msg_data, topic_name, msg_type);
    }else if(msg_type != NULL){
        sprintf(strtemp_ ,"received an unparseable data, the data type is %s", msg_type);
        nsemq_write_debug(NULL, strtemp_);
    }else {
        sprintf(strtemp_ ,"received an null topic type data, the data is %s", (char *)rkmessage->payload);
        nsemq_write_debug(NULL, strtemp_);
    }
    // The provided consume_cb function is called for each message,
    // the application MUST NOT call rd_kafka_message_destroy() on the provided rkmessage.
}

/*** judge connection function ***/
BOOL nsemq_judge_connect(rd_kafka_t *handle){
    const struct rd_kafka_metadata *metadata;
    rd_kafka_resp_err_t err = rd_kafka_metadata(handle, 1, NULL, &metadata, 1000);
    return (err == RD_KAFKA_RESP_ERR_NO_ERROR)?TRUE:FALSE;
}

/*** inner logger function ***/
void nsemq_write_error(const rd_kafka_t *rk, char *errstr){
    rd_kafka_log_print(rk, LOG_ERR, "NseMQ", errstr);
}

void nsemq_write_info(const rd_kafka_t *rk, char *infostr){
    if(log_level >= LOG_INFO) {
        rd_kafka_log_print(rk, LOG_INFO, "NseMQ", infostr);
    }
}

void nsemq_write_debug(const rd_kafka_t *rk, char *debugstr){
    if(log_level >= LOG_DEBUG) { // manually judge the log level
        rd_kafka_log_print(rk, LOG_DEBUG, "NseMQ", debugstr);
    }
}