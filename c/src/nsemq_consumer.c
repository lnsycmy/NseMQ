#include "nsemq_consumer.h"
static rd_kafka_t *consumer_;               // consumer instance handle.
static rd_kafka_conf_t *consumer_conf_;     // consumer configuration object.
static rd_kafka_topic_conf_t *topic_conf_;  // topic configuration object.
static rd_kafka_resp_err_t err_;            // kafka error code.
static rd_kafka_queue_t *topic_queue_;      // topic queue.
static char group_id[UUID4_LEN];            // consumer group id.

static RunStatus consumer_run_status_ = NO_INIT;     // consumer current status.
static char errstr_[512];                   // librdkafka API error reporting buffer.
static char strtemp_[512];                  // inner function error.
static pthread_t consume_thread_;           // consumer thread.

pthread_mutex_t topic_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t status_mutex = PTHREAD_MUTEX_INITIALIZER;
extern topic_map_t g_topic_map_;        // defined in nsemq_base.c

/* consumer function */
ErrorCode nsemq_consumer_init(const char *broker_addr) {
    rd_kafka_conf_res_t conf_res;
    memset(errstr_, 0, sizeof(errstr_));
    consumer_conf_ = rd_kafka_conf_new(); // Kafka configuration
    topic_conf_ = rd_kafka_topic_conf_new(); // Topic configuration
    // judge current status
    if(consumer_run_status_ != NO_INIT){
        nsemq_write_error(NULL,"Don't initialize consumer multiple times.");
        return ERR_C_INIT_MULTIPLE_INIT;
    }

    // create group_id by uuid.
    uuid4_init();
    uuid4_generate(group_id);
    // set bootstrap broker address and port.
    conf_res = rd_kafka_conf_set(consumer_conf_,"bootstrap.servers", broker_addr, errstr_, sizeof(errstr_));
    if(conf_res != RD_KAFKA_CONF_OK){
        nsemq_write_error(NULL, "No vaild bootstrap servers specified.");
        return ERR_C_INIT_BROKER_ADDRESS;
    }
    // conf_res = rd_kafka_conf_set(consumer_conf_,"group.id", group_id, errstr_, sizeof(errstr_));
    conf_res = rd_kafka_conf_set(consumer_conf_,"group.instance.id", group_id, errstr_, sizeof(errstr_));
    if(conf_res != RD_KAFKA_CONF_OK){
        nsemq_write_error(NULL,"No vaild group id specified.");
        return ERR_C_INIT_GROUP_ID;
    }
    // Create consumer using accumulated global configuration.
    if (!(consumer_ = rd_kafka_new(RD_KAFKA_CONSUMER, consumer_conf_,
                                   errstr_, sizeof(errstr_)))) {
        sprintf(strtemp_, "Failed to create new consumer:%s", errstr_);
        nsemq_write_error(NULL, strtemp_);
        return ERR_C_CREATE_CONSUMER;
    }
    // judge connection with broker.
    if(TRUE != nsemq_judge_connect(consumer_)){
        nsemq_write_error(NULL,"Failed to connect broker.");
        return ERR_FAIL_CONNECT_BROKER;
    }
    // initialize the consumer queue.
    topic_queue_ =  rd_kafka_queue_new(consumer_);
    consumer_run_status_ = INIT_STATUS;
    return ERR_NO_ERROR;
}

ErrorCode nsemq_consumer_subscribe(const char *topic_name,
                                   deserialize_func d_fun,
                                   void (*consume_callback)(void *, char *, char *)) {
    int start_res = 0;
    rd_kafka_topic_t *topic_object;
    TopicItem topic_item;
    // judge the run status.
    if(INIT_STATUS != consumer_run_status_){
        nsemq_write_error(consumer_, "Failed to subscribe: only allow to subscribe topic in init_status.");
        return ERR_C_RUN_STATUS;
    }
    // determine whether topic_name exists.
    if (map_get(&g_topic_map_, topic_name) != NULL) {
        return ERR_NO_ERROR; // have subscribe topic, reback no error.
    }
    // start to consume message from broker, and save the message to local queue.
    topic_object = rd_kafka_topic_new(consumer_, topic_name, NULL);
    start_res = rd_kafka_consume_start_queue(topic_object, NSEMQ_DEFAULT_PARTITION, NSEMQ_DEFAULT_OFFSET, topic_queue_);
    // start_res = rd_kafka_consume_start(topic_object, NSEMQ_DEFAULT_PARTITION, NSEMQ_DEFAULT_OFFSET);
    if (start_res == -1) {
        err_ = rd_kafka_last_error();
        sprintf(strtemp_, "Failed to start consuming:%s", rd_kafka_err2str(err_));
        nsemq_write_error(consumer_, strtemp_);
        if (err_ == RD_KAFKA_RESP_ERR__INVALID_ARG){
            nsemq_write_error(consumer_, "Broker based offset storage requires a group.id");
        }
        return ERR_C_SUBS_CREATE_TOPIC;
    }
    // save the mapping by topic name¡¢topic object¡¢msg_consume
    pthread_mutex_lock(&topic_mutex);
    topic_item.topic_object = topic_object;
    topic_item.deserialize_func = d_fun;
    topic_item.consume_callback = consume_callback;
    map_set(&g_topic_map_, topic_name, topic_item);
    pthread_mutex_unlock(&topic_mutex);
    return ERR_NO_ERROR;
}

ErrorCode nsemq_consumer_unsubscribe(const char *topic_name){
    int stop_res = 0;
    rd_kafka_topic_t *rkt;
    TopicItem *topic_item = NULL;
    // judge the run status.
    if(INIT_STATUS != consumer_run_status_){
        nsemq_write_error(consumer_, "Failed to unsubscribe: only allow to unsubscribe topic before called start().");
        return ERR_C_RUN_STATUS;
    }
    // verify that the topic callback list is empty or not includes topic_name.
    topic_item = map_get(&g_topic_map_, topic_name);
    if(topic_item == NULL){
        sprintf(strtemp_, "topic subscription is empty or havn't subscribed the topic:%s", topic_name);
        nsemq_write_error(consumer_, strtemp_);
        return ERR_C_UNSUBS_TOPIC_NO_FIND;
    }
    // when it exists, call consume_stop(), cancle subscribe the topic.
    stop_res = rd_kafka_consume_stop(topic_item->topic_object, NSEMQ_DEFAULT_PARTITION);
    if(stop_res != 0){
        err_ = rd_kafka_last_error();
        nsemq_write_error(consumer_, (char *)rd_kafka_err2str(err_));
        return ERR_C_UNSUNS_BROKER_TOPIC;
    }
    // delete topic from topic callback map.
    pthread_mutex_lock(&topic_mutex);
    map_remove(&g_topic_map_, topic_name);
    pthread_mutex_unlock(&topic_mutex);
    return ERR_NO_ERROR;
}

ErrorCode nsemq_consumer_subscription(char **topic_array, int *topic_count){
    const char *key;
    int count = 0;
    map_iter_t iter = map_iter(&g_topic_map_);
    while ((key = map_next(&g_topic_map_, &iter))) {
        topic_array[count] = (char *)key;
        printf("inner topic:%s\n", topic_array[count]);
        count ++;
    }
    *topic_count = count;
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
            nsemq_write_error(consumer_, "Failed to start consume callback.");
            break;
        }
        rd_kafka_poll(consumer_, 0);
    }
    pthread_exit(NULL);
    return NULL;
}
ErrorCode nsemq_consumer_start(){
    int create_res, i = 0;
    // judge the run status.
    if(consumer_run_status_ != INIT_STATUS){
        nsemq_write_error(consumer_, "Failed to start: only allow to start consumer in init_status.");
        return ERR_C_RUN_STATUS;
    }
    // set start status
    pthread_mutex_lock(&status_mutex);
    consumer_run_status_ = START_STATUS;
    pthread_mutex_unlock(&status_mutex);
    // create pthread, and detach.
    create_res = pthread_create(&consume_thread_, NULL, thread_function, NULL);
    pthread_detach(consume_thread_);
    return ERR_NO_ERROR;
}

// close the consumer.
ErrorCode nsemq_consumer_close() {
    const char *key;
    TopicItem *topicItem;
	map_iter_t iter_stop;
	map_iter_t iter_destroy;
    // NO.0 judge the run status.
    if(consumer_run_status_ == CLOSE_STATUS){
        nsemq_write_error(consumer_, "Failed to close: can't multiple called close() function.");
        return ERR_C_RUN_STATUS;
    }
    // NO.1 stop the consume from broker.
    iter_stop = map_iter(&g_topic_map_);
    while ((key = map_next(&g_topic_map_, &iter_stop))) {
        topicItem = map_get(&g_topic_map_, key);
        err_ = rd_kafka_consume_stop(topicItem->topic_object, NSEMQ_DEFAULT_PARTITION);
        if(err_ != ERR_NO_ERROR){
            err_ = rd_kafka_last_error();
            sprintf(strtemp_, "Failed to stop consuming topic(%s):%s", key, (char *)rd_kafka_err2str(err_));
            nsemq_write_error(consumer_, strtemp_);
        }
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
    // NO.4 destroy topic object.
    iter_destroy = map_iter(&g_topic_map_);
    while ((key = map_next(&g_topic_map_, &iter_destroy))) {
        topicItem = map_get(&g_topic_map_, key);
        rd_kafka_topic_destroy(topicItem->topic_object);
    }
    // NO.5 clear topic map.
    pthread_mutex_lock(&topic_mutex);
    map_deinit(&g_topic_map_);
    pthread_mutex_unlock(&topic_mutex);
    // NO.6 destroy topic queue.
    rd_kafka_queue_destroy(topic_queue_);
    // NO.7 destroy handle
    rd_kafka_destroy(consumer_);
    return ERR_NO_ERROR;
}
