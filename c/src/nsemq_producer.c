#include "nsemq_producer.h"
rd_kafka_t *producer_;                  // Producer instance handle.
rd_kafka_conf_t *producer_conf_;        // Temporary configuration object.
rd_kafka_topic_conf_t *topic_conf_;     // topic configuration object.

RunStatus producer_run_status_ = NO_INIT;        // consumer current status.
char errstr_[512];                      // librdkafka API error reporting buffer.
char errtemp_[512];                     // inner function error.

ErrorCode nsemq_producer_init(const char * broker_addr, void *dr_msg_cb){
    // judge current status
    if(producer_run_status_ != NO_INIT){
        nsemq_write_error("Don't initialize producer multiple times.");
        return ERR_P_INIT_MULTIPLE_INIT;
    }
    // Create Kafka client configuration place-holder
    producer_conf_ = rd_kafka_conf_new();
    // Set bootstrap broker(s) as a comma-separated list of host or host:port (default port 9092).
    if (rd_kafka_conf_set(producer_conf_, "bootstrap.servers", broker_addr,
                          errstr_, sizeof(errstr_)) != RD_KAFKA_CONF_OK) {
        nsemq_write_error(errstr_);
        return ERR_P_INIT_BROKER_ADDRESS;
    }
    // Set the delivery report callback.
    if(NULL != dr_msg_cb){
        rd_kafka_conf_set_dr_msg_cb(producer_conf_, dr_msg_cb);
    }
    // Create producer instance.
    producer_ = rd_kafka_new(RD_KAFKA_PRODUCER, producer_conf_, errstr_, sizeof(errstr_));
    if (!producer_) {
        strcpy_s(errtemp_, sizeof(errtemp_), "Failed to create new producer.");
        strcat_s(errtemp_, sizeof(errtemp_), errstr_);
        nsemq_write_error(errtemp_);
        return ERR_P_CREATE_PRODUCER;
    }
    // judge connection with broker.
    if(TRUE != nsemq_judge_connect(producer_)){
        nsemq_write_error("Failed to connect broker.");
        return ERR_FAIL_CONNECT_BROKER;
    }
    producer_run_status_ = INIT_STATUS;
    return ERR_NO_ERROR;
}

ErrorCode nsemq_producer_produce(void *msg, const char *topic_name){
    rd_kafka_resp_err_t err;
    BOOL resend_flag = FALSE;
    ErrorCode err_temp = ERR_NO_ERROR;
    char *msg_buf;      // 序列化后的字符数组
    char *msg_type;     // 原结构体的类型。
    // struct is serialized into char*
    int buf_size = nsemq_encode(msg, &msg_buf, &msg_type);

    printf("msg_type:%s, len:%d\n", msg_type, strlen(msg_type)+1);

    // judge the run status.
    if(producer_run_status_ == CLOSE_STATUS){
        nsemq_write_error("Failed to produce: don't allow call produce() after close().");
        return ERR_P_RUN_STATUS;
    }else if(producer_run_status_ != START_STATUS){
        producer_run_status_ = START_STATUS;
    }
    // judge message whether empty.
    if (buf_size == 0) {
        rd_kafka_poll(producer_, 0/*non-blocking */);
        return ERR_P_SEND_MSG_EMPTY; // 退出
    }
    // Send/Produce message.
    retry:
    err = rd_kafka_producev(
            /* Producer handle */
            producer_,
            /* Topic name */
            RD_KAFKA_V_TOPIC(topic_name),
            /* Make a copy of the payload. */
            RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
            /* Message value and length */
            RD_KAFKA_V_VALUE(msg_buf, buf_size),
            /* Message key pointer and length (const void *, size_t) */
            RD_KAFKA_V_KEY(msg_type,strlen(msg_type)+1),
            /* Per-Message opaque */
            RD_KAFKA_V_OPAQUE(NULL),
            /* End sentinel */
            RD_KAFKA_V_END);
    if (err) {
        // Failed to *enqueue* message for producing.
        strcpy_s(errtemp_, sizeof(errtemp_), "Failed to produce to topic( ");
        strcat_s(errtemp_, sizeof(errtemp_), topic_name);
        strcat_s(errtemp_, sizeof(errtemp_), " ):");
        strcat_s(errtemp_, sizeof(errtemp_), rd_kafka_err2str(err));
        nsemq_write_error(errtemp_);
        if (err == RD_KAFKA_RESP_ERR__QUEUE_FULL) {
            rd_kafka_poll(producer_, 1000/*block for max 1000ms*/);
            if (!resend_flag) { // allow once resend.
                resend_flag = TRUE;
                goto retry;
            } else {
                err_temp =  ERR_P_SEND_QUEUE_FULL;
            }
        } else if (err == RD_KAFKA_RESP_ERR_MSG_SIZE_TOO_LARGE) {
            strcpy_s(errtemp_, sizeof(errtemp_), "message size it too large: ");
            strcat_s(errtemp_, sizeof(errtemp_), rd_kafka_err2str(err));
            nsemq_write_error(errtemp_);
            err_temp = ERR_P_SEND_MSG_TOO_LARGE;
        } else if (err == RD_KAFKA_RESP_ERR__UNKNOWN_TOPIC) {
            strcpy_s(errtemp_, sizeof(errtemp_), "unknown the topic(");
            strcat_s(errtemp_, sizeof(errtemp_), topic_name);
            strcat_s(errtemp_, sizeof(errtemp_), "):");
            strcat_s(errtemp_, sizeof(errtemp_), rd_kafka_err2str(err));
            nsemq_write_error(errtemp_);
            err_temp = ERR_P_SEND_UNKNOWN_TOPIC;
        } else {
            err_temp = ERR_P_SEND_FAIL;
        }
    } else {
        printf("%% Enqueued message (%d bytes) for topic %s\n", buf_size, topic_name);
        err_temp = ERR_NO_ERROR;
    }
    rd_kafka_poll(producer_, 0/*non-blocking*/);
    // 4. release the buffer memory
    free(msg_buf);
    return err_temp;
}
/*
 * close producer and clear memory.
 */
ErrorCode nsemq_producer_close(){
    // judge the run status.
    if(producer_run_status_ == CLOSE_STATUS){
        nsemq_write_error("Failed to close: can't multiple called close() function.");
        return ERR_P_RUN_STATUS;
    }
    // flush message: make sure all outstanding requests are transmitted and handled
    rd_kafka_flush(producer_, 5 * 1000 /* wait for max 5 seconds */);
    if (rd_kafka_outq_len(producer_) > 0){
        // strcpy_s(errtemp_, sizeof(errtemp_), rd_kafka_outq_len(producer_));
        strcat_s(errtemp_, sizeof(errtemp_), "message(s) were not delivered");
        nsemq_write_info(errtemp_);
    }
    // Destroy the producer instance
    rd_kafka_destroy(producer_);
    rd_kafka_wait_destroyed(5000);
    producer_run_status_ = CLOSE_STATUS;
    return ERR_NO_ERROR;
}



