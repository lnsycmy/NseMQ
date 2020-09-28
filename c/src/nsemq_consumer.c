#include "nsemq_consumer.h"
rd_kafka_t *consumer_;               // consumer instance handle.
rd_kafka_conf_t *consumer_conf_;     // consumer configuration object.
rd_kafka_topic_conf_t *topic_conf_;  // topic configuration object.
rd_kafka_resp_err_t err_;            // kafka error code.
rd_kafka_queue_t *topic_queue_;      // topic queue.

RunStatus consumer_run_status_ = NO_INIT;     // consumer current status.
char errstr_[512];                   // librdkafka API error reporting buffer.
char errtemp_[512];                  // inner function error.
char consumer_group_id_;             // consumer group id.
pthread_t consume_thread_;           // consumer thread.
pthread_mutex_t topic_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t status_mutex = PTHREAD_MUTEX_INITIALIZER;

extern TopicList topic_list_;        // topic and consume callback mapping list.

/* consumer function */
ErrorCode nsemq_consumer_init(const char *broker_addr) {
    rd_kafka_conf_res_t conf_res;
    const char *group_id = "1000";
    memset(errstr_, 0, sizeof(errstr_));
    consumer_conf_ = rd_kafka_conf_new(); // Kafka configuration
    topic_conf_ = rd_kafka_topic_conf_new(); // Topic configuration
    // judge current status
    if(consumer_run_status_ != NO_INIT){
        nsemq_write_error("Don't initialize consumer multiple times.");
        return ERR_C_INIT_MULTIPLE_INIT;
    }
    // set random group id(reference timer).
    conf_res = rd_kafka_conf_set(consumer_conf_,"group.id", group_id, errstr_, sizeof(errstr_));
    if(conf_res != RD_KAFKA_CONF_OK){
        nsemq_write_error("No vaild group id specified.");
        return ERR_C_INIT_GROUP_ID;
    }
    // Create consumer using accumulated global configuration.
    if (!(consumer_ = rd_kafka_new(RD_KAFKA_CONSUMER, consumer_conf_,
                                   errstr_, sizeof(errstr_)))) {
        strcpy_s(errtemp_, sizeof(errtemp_), "Failed to create new consumer:");
        strcat_s(errtemp_, sizeof(errtemp_), errstr_);
        nsemq_write_error(errtemp_);
        return ERR_C_CREATE_CONSUMER;
    }
    // set bootstrap broker address and port.
    if (rd_kafka_brokers_add(consumer_, broker_addr) == 0) {
        nsemq_write_error("No valid brokers specified.");
        return ERR_C_INIT_BROKER_ADDRESS;
    }
    // judge connection with broker.
    if(TRUE != nsemq_judge_connect(consumer_)){
        nsemq_write_error("Failed to connect broker.");
        return ERR_FAIL_CONNECT_BROKER;
    }
    // initialize the consumer queue.
    topic_queue_ =  rd_kafka_queue_new(consumer_);

    consumer_run_status_ = INIT_STATUS;
    return ERR_NO_ERROR;
}

ErrorCode _nsemq_consumer_subscribe(const char *topic_name,
                                   deserialize_func d_fun,
                                   void (*consume_callback)(void *, char *, char *)) {
    TopicList findNode;
    int start_res = 0;
    rd_kafka_topic_t *topic_object;
    // judge the run status.
    if(INIT_STATUS != consumer_run_status_){
        nsemq_write_error("Failed to subscribe: only allow to subscribe topic in init_status.");
        return ERR_C_RUN_STATUS;
    }
    // determine whether topic_name exists.
    findNode = find_item(topic_list_, topic_name);
    if (findNode != NULL) {
        return ERR_NO_ERROR; // have subscribe topic, reback no error.
    }
    // start to consume message from broker, and save the message to local queue.
    topic_object = rd_kafka_topic_new(consumer_, topic_name, NULL);
    start_res = rd_kafka_consume_start_queue(topic_object, NSEMQ_DEFAULT_PARTITION, NSEMQ_DEFAULT_OFFSET, topic_queue_);
    // start_res = rd_kafka_consume_start(topic_object, NSEMQ_DEFAULT_PARTITION, NSEMQ_DEFAULT_OFFSET);
    if (start_res == -1) {
        err_ = rd_kafka_last_error();
        strcpy_s(errtemp_, sizeof(errtemp_), "Failed to start consuming:");
        strcat_s(errtemp_, sizeof(errtemp_), rd_kafka_err2str(err_));
        nsemq_write_error(errtemp_);
        if (err_ == RD_KAFKA_RESP_ERR__INVALID_ARG){
            nsemq_write_error("Broker based offset storage requires a group.id");
        }
        return ERR_C_SUBS_CREATE_TOPIC;
    }
    // save the mapping by topic name、topic object、msg_consume
    pthread_mutex_lock(&topic_mutex);
    insert_list(&topic_list_, topic_name, topic_object,
                d_fun, consume_callback);
    pthread_mutex_unlock(&topic_mutex);
    return ERR_NO_ERROR;
}

ErrorCode nsemq_consumer_unSubscribe(const char *topic_name){
    TopicList curNode = topic_list_;
    rd_kafka_topic_t *rkt;
    int stop_res;
    // judge the run status.
    if(INIT_STATUS != consumer_run_status_){
        nsemq_write_error("Failed to unsubscribe: only allow to unsubscribe topic before called start().");
        return ERR_C_RUN_STATUS;
    }
    // when it exists, call consume_stop(), cancle subscribe the topic.
    while(curNode){
        if(strcmp(topic_name, curNode->topic_name) == 0){
            break; // pointer to target node.
        }
        curNode = curNode->next;
    }
    // verify that the topic callback list is empty or not includes topic_name.
    if(!curNode){
        strcpy_s(errtemp_, sizeof(errtemp_), "topic subscription is empty or havn't subscribed the topic:");
        strcat_s(errtemp_, sizeof(errtemp_), topic_name);
        nsemq_write_error(errtemp_);
        return ERR_C_UNSUBS_TOPIC_NO_FIND;
    }
    // create topic, call consumer->stop(), cancle subscribe the topic.
    stop_res = rd_kafka_consume_stop(curNode->topic_object, NSEMQ_DEFAULT_PARTITION);
    if(stop_res != 0){
        err_ = rd_kafka_last_error();
        nsemq_write_error((char *)rd_kafka_err2str(err_));
        return ERR_C_UNSUNS_BROKER_TOPIC;
    }
    // delete topic from topic callback list.
    pthread_mutex_lock(&topic_mutex);
    delete_item(&topic_list_, topic_name);
    pthread_mutex_unlock(&topic_mutex);
    return ERR_NO_ERROR;
}

ErrorCode nsemq_consumer_subscription(TopicList *topic_array){
    TopicList curNode = topic_list_;
    while(curNode){
        TopicList newNode = malloc(sizeof(struct TopicNode));
        newNode->topic_name = curNode->topic_name;
        newNode->next = NULL;
        if(!*topic_array){
            *topic_array = newNode;
        }else{
            (*topic_array)->next = newNode;
        }
        curNode = curNode->next;
    }
    return ERR_NO_ERROR;
}

void *thread_function(void *agr){
    int consume_res, i = 0;
    while(consumer_run_status_ == START_STATUS) {
        // consume multiple messages from queue with callback.
        printf("enter poll()\n");
        consume_res = rd_kafka_consume_callback_queue(topic_queue_, 1000,/* timeout_ms */
                                                      nsemq_consume_callback, NULL);
        if(consume_res == -1){ // error
            nsemq_write_error("Failed to start consume callback.");
            break;
        }
        rd_kafka_poll(consumer_, 0);
    }
    pthread_exit(NULL);
    return NULL;
}
ErrorCode nsemq_consumer_start(){
    int create_res, i = 0;
    TopicList curNode = topic_list_;
    // judge the run status.
    if(consumer_run_status_ != INIT_STATUS){
        nsemq_write_error("Failed to start: only allow to start consumer in init_status.");
        return ERR_C_RUN_STATUS;
    }
    // set start status
    pthread_mutex_lock(&status_mutex);
    consumer_run_status_ = START_STATUS;
    pthread_mutex_unlock(&status_mutex);
    create_res = pthread_create(&consume_thread_,NULL,thread_function, curNode);
    pthread_detach(consume_thread_);
    return ERR_NO_ERROR;
}

// close the consumer.
ErrorCode nsemq_consumer_close() {
    TopicList curNode = topic_list_;
    // NO.0 judge the run status.
    if(consumer_run_status_ == CLOSE_STATUS){
        nsemq_write_error("Failed to close: can't multiple called close() function.");
        return ERR_C_RUN_STATUS;
    }
    printf("NO.0 finished.\n");
    // NO.1 stop the consume from broker.
    while (curNode) {
        err_ = rd_kafka_consume_stop(curNode->topic_object, NSEMQ_DEFAULT_PARTITION);
        if(err_ != ERR_NO_ERROR){
            strcpy_s(errtemp_, sizeof(errtemp_), "Failed to stop consuming topic:");
            strcat_s(errtemp_, sizeof(errtemp_), curNode->topic_name);
            nsemq_write_error(errtemp_);
            err_ = rd_kafka_last_error();
            nsemq_write_error((char *)rd_kafka_err2str(err_));
        }
        curNode = curNode->next;
    }
    // NO.2 flush the local queue.
    while (rd_kafka_outq_len(consumer_) > 0) {
        rd_kafka_poll(consumer_, 10);
    }
    // NO.3 ending the consumer thread.
    pthread_mutex_lock(&status_mutex);
    consumer_run_status_ = CLOSE_STATUS;
    pthread_mutex_unlock(&status_mutex);
    pthread_cancel(consume_thread_);

    // NO.4 destroy topic and clear topic list.
    curNode = topic_list_;
    while (curNode) {
        rd_kafka_topic_destroy(curNode->topic_object);
        curNode = curNode->next;
    }
    pthread_mutex_lock(&topic_mutex);
    clear_list(&topic_list_);
    pthread_mutex_unlock(&topic_mutex);
    // NO.5 destroy handle
    rd_kafka_destroy(consumer_);
    return ERR_NO_ERROR;
}
