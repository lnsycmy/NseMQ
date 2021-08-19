#include "nsemq_consumer.h"

static rd_kafka_t *consumer_;               // consumer instance handle.
static rd_kafka_conf_t *consumer_conf_;     // consumer configuration object.
static rd_kafka_resp_err_t err_;            // kafka error code.
static rd_kafka_queue_t *topic_queue_;      // topic queue.
static char group_id[UUID4_LEN];            // consumer group id.

static RunStatus consumer_run_status_ = NO_INIT; // consumer current status.
static char errstr_[512];                   // librdkafka API error reporting buffer.
static char strtemp_[512];                  // inner function str.
static pthread_t consume_thread_;           // consumer thread.
static pthread_mutex_t topic_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t status_mutex = PTHREAD_MUTEX_INITIALIZER;

extern topic_map_t g_topic_map_;            // defined in nsemq_base.c
extern int         log_level;               // defined in nsemq_base.c

/* Initialize the consumer by broker address, the consumer handle created internally. */
ErrorCode nsemq_consumer_init(const char *broker_addr) {
    rd_kafka_conf_res_t conf_res;
    memset(errstr_, 0, sizeof(errstr_));
    consumer_conf_ = rd_kafka_conf_new();   // Kafka configuration
    // 0.judge current status
    if (consumer_run_status_ != NO_INIT && consumer_run_status_ != CLOSE_STATUS) {
        nsemq_write_error(NULL, "Don't initialize consumer multiple times.");
        return ERR_C_INIT_MULTIPLE_INIT;
    }
    // 1.set bootstrap broker address and port.
    conf_res = rd_kafka_conf_set(consumer_conf_, "bootstrap.servers", broker_addr, errstr_, sizeof(errstr_));
    if (conf_res != RD_KAFKA_CONF_OK) {
        nsemq_write_error(NULL, "No vaild bootstrap servers specified.");
        return ERR_C_INIT_BROKER_ADDRESS;
    }
    // 2.create group_id by uuid.
    uuid4_init();
    uuid4_generate(group_id);
    conf_res = rd_kafka_conf_set(consumer_conf_, "group.instance.id", group_id, errstr_, sizeof(errstr_));
    if (conf_res != RD_KAFKA_CONF_OK) {
        nsemq_write_error(NULL, "No vaild group id specified.");
        return ERR_C_INIT_GROUP_ID;
    }
    // 3.create consumer using accumulated global configuration.
    if (!(consumer_ = rd_kafka_new(RD_KAFKA_CONSUMER, consumer_conf_,
                                   errstr_, sizeof(errstr_)))) {
        sprintf(strtemp_, "Failed to create new consumer:%s", errstr_);
        nsemq_write_error(NULL, strtemp_);
        return ERR_C_CREATE_CONSUMER;
    }
    // 4.judge connection with broker.
    if (TRUE != nsemq_judge_connect(consumer_)) {
        nsemq_write_error(NULL, "Failed to connect broker.");
        rd_kafka_destroy(consumer_);
        return ERR_FAIL_CONNECT_BROKER;
    }
    // 5.initialize the consumer queue and topic map.
    topic_queue_ = rd_kafka_queue_new(consumer_);
    map_init(&g_topic_map_);
    // 6.set log level
    rd_kafka_set_log_level(consumer_, LOG_INFO);
    log_level = LOG_INFO;
    consumer_run_status_ = INIT_STATUS;
    return ERR_NO_ERROR;
}

/* subscribe topic function, called by nsemq_consumer_subscribe() internally */
ErrorCode nsemq_consumer_subscribe_internal(const char *topic_name,
                                            const char *data_type,
                                            ds_msg_func deserialize_func,
                                            msg_cb_func consume_callback) {
    int start_res = 0;
    rd_kafka_topic_t *topic_object;
    TopicItem topic_item;
    // 0.judge the run status, if subscription is not allowed, stop subscription.
    if (consumer_run_status_ == NO_INIT || consumer_run_status_ == CLOSE_STATUS) {
        nsemq_write_error(consumer_,
                          "Failed to subscribe: not allowed to subscribe to topics in uninitialized or closed status.");
        return ERR_C_RUN_STATUS;
    }
    // 1.determine whether topic_name exists.
    if (map_get(&g_topic_map_, topic_name) != NULL) {
        sprintf(strtemp_, "Repeat subscription topic:%s", topic_name);
        nsemq_write_info(consumer_, strtemp_);
        return ERR_NO_ERROR;
    }
    // 2.create the topic object, and determine whether the creation is successful.
    topic_object = rd_kafka_topic_new(consumer_, topic_name, NULL);
    if (NULL == topic_object) {
        sprintf(strtemp_, "Failed to generate topic(%s) for this consumer", topic_name);
        nsemq_write_error(consumer_, strtemp_);
        return ERR_C_RUN_STATUS;
    }
    // 3. if START_STATUS, start to consume message from broker.
    if (consumer_run_status_ == START_STATUS) {
        // save the message to local queue, and start to consume.
        start_res = rd_kafka_consume_start_queue(topic_object, NSEMQ_DEFAULT_PARTITION, NSEMQ_DEFAULT_OFFSET,
                                                 topic_queue_);
        if (start_res == -1) {
            err_ = rd_kafka_last_error();
            sprintf(strtemp_, "Failed to start consuming:%s", rd_kafka_err2str(err_));
            nsemq_write_error(consumer_, strtemp_);
            return ERR_C_SUBS_CREATE_TOPIC;
        }
    }
    // 4.add topic to topic_map
    pthread_mutex_lock(&topic_mutex);
    topic_item.bind_data_type = (char *)data_type;
    topic_item.topic_object = topic_object;
    topic_item.deserialize_func = deserialize_func;
    topic_item.consume_callback = consume_callback;
    topic_item.subs_status = TRUE;
    map_set(&g_topic_map_, topic_name, topic_item);
    pthread_mutex_unlock(&topic_mutex);
    return ERR_NO_ERROR;
}

/* unsubscribe one topic, stop consuming this topic if start() have been called */
ErrorCode nsemq_consumer_unsubscribe(const char *topic_name) {
    int stop_res = 0;
    rd_kafka_topic_t *topic_object;
    TopicItem *topic_item;
    // 0.judge the run status, if un-subscription is not allowed, stop un-subscription.
    if (consumer_run_status_ == NO_INIT || consumer_run_status_ == CLOSE_STATUS) {
        nsemq_write_error(consumer_,
                          "Failed to subscribe: not allowed to unsubscribe the topics in uninitialized or closed status.");
        return ERR_C_RUN_STATUS;
    }
    // 1.determine whether topic_name exists.
    topic_item = map_get(&g_topic_map_, topic_name);
    if (topic_item == NULL) {
        sprintf(strtemp_, "topic subscription is empty or not subscribed the topic:%s", topic_name);
        nsemq_write_error(consumer_, strtemp_);
        return ERR_C_UNSUBS_TOPIC_NO_FIND;
    }
    // 2.if START_STATUS, stop to consume message from broker.
    if (consumer_run_status_ == START_STATUS) {
        // stop to consume.
        stop_res = rd_kafka_consume_stop(topic_item->topic_object, NSEMQ_DEFAULT_PARTITION);
        if (stop_res != 0) {
            err_ = rd_kafka_last_error();
            nsemq_write_error(consumer_, (char *) rd_kafka_err2str(err_));
            return ERR_C_UNSUNS_BROKER_TOPIC;
        }

    }
    // 3.destroy the topic object.
    rd_kafka_topic_destroy(topic_item->topic_object);
    // 4.remove topic from topic_map
    pthread_mutex_lock(&topic_mutex);
    map_remove(&g_topic_map_, topic_name);
    pthread_mutex_unlock(&topic_mutex);
    return ERR_NO_ERROR;
}

/* get all subscribed topic names and save it to topic_list. */
ErrorCode nsemq_consumer_subscriptions(list_t *topic_list) {
    const char *key;
    map_iter_t iter;
    if (NULL == topic_list) {
        nsemq_write_error(consumer_, "Not enough memory allocated when acquiring subscription.");
        return ERR_C_GET_SUBS_MEMORY;
    }
    iter = map_iter(&g_topic_map_);
    while ((key = map_next(&g_topic_map_, &iter))) {
        list_rpush(topic_list, list_node_new((void *)key));
    }
    return ERR_NO_ERROR;
}

/* used to create consumer thread */
void *consume_thread_func(void *agr) {
    int consume_res;
    while (1) {
        pthread_mutex_lock(&status_mutex);
        if(consumer_run_status_ != START_STATUS) {
            pthread_mutex_unlock(&status_mutex);
            break;
        }
        nsemq_write_debug(consumer_, "Enter poll(): refresh the received message");
        // consume multiple messages from queue with callback.
        consume_res = rd_kafka_consume_callback_queue(topic_queue_, 1000,/* timeout_ms */
                                                      nsemq_consume_callback, NULL);
        // add unlock after callback, ensure topic_queue_ can't be destroyed between called callback_queue.
        pthread_mutex_unlock(&status_mutex);
        if (consume_res == -1) { // error
            nsemq_write_error(consumer_, "Failed to start consume callback.");
            break;
        }
        rd_kafka_consumer_poll(consumer_,0);
    }
    pthread_detach(pthread_self());
    return NULL;
}

/* start to consume message from broker */
ErrorCode nsemq_consumer_start(int async_flag) {
    int start_res, create_res = -1;
    const char *key;
    map_iter_t iter;
    TopicItem *topic_item;
    pthread_attr_t attr;    // set attr PTHREAD_CREATE_DETACHED.
    // 0.judge the run status.
    if (consumer_run_status_ != INIT_STATUS && consumer_run_status_ != STOP_STATUS) {
        nsemq_write_error(consumer_, "Failed to start: only allow to start consume after called init() or stop().");
        return ERR_C_RUN_STATUS;
    }
    // 1.traverse the topic map and start consumption.
    iter = map_iter(&g_topic_map_);
    while ((key = map_next(&g_topic_map_, &iter))) {
        // get the topic item.
        topic_item = map_get(&g_topic_map_, key);
        // save the message to local queue, and start to consume.
        start_res = rd_kafka_consume_start_queue(topic_item->topic_object, NSEMQ_DEFAULT_PARTITION,
                                                 NSEMQ_DEFAULT_OFFSET, topic_queue_);
        if (start_res == -1) {
            err_ = rd_kafka_last_error();
            sprintf(strtemp_, "Failed to start consuming from topic(%s):%s", key, rd_kafka_err2str(err_));
            nsemq_write_error(consumer_, strtemp_);
            topic_item->subs_status = FALSE;
            continue;
        }
    }
    // 2.set start status, ensure that thread function can run.
    pthread_mutex_lock(&status_mutex);
    consumer_run_status_ = START_STATUS;
    pthread_mutex_unlock(&status_mutex);
    // 3.create consume callback thread.
    create_res = pthread_create(&consume_thread_, NULL, consume_thread_func, NULL);
    if(create_res != 0){
        sprintf(strtemp_, "Failed to create consume thread, errno is %d", errno);
        return ERR_C_START_CREATE_THREAD;
    }
    // 4.achieve async or sync through pthread_detach()/pthread_join()
    if (async_flag) {
        pthread_detach(consume_thread_);
    } else {
        pthread_join(consume_thread_, NULL);
    }
    return ERR_NO_ERROR;
}

/* stop the consumer asynchronously by one thread */
void* stop_thread_func(void *agr) {
    const char *key;
    map_iter_t iter;
    TopicItem *topic_item;
    // traverse the topic map and stop consumption.
    iter = map_iter(&g_topic_map_);
    while ((key = map_next(&g_topic_map_, &iter))) {
        // get the topic item, and stop using this topic's message.
        topic_item = map_get(&g_topic_map_, key);
        err_ = rd_kafka_consume_stop(topic_item->topic_object, NSEMQ_DEFAULT_PARTITION);
        if (err_ != ERR_NO_ERROR) {
            err_ = rd_kafka_last_error();
            sprintf(strtemp_, "Failed to stop consuming topic(%s):%s", key, (char *) rd_kafka_err2str(err_));
            nsemq_write_error(consumer_, strtemp_);
            continue;
        }
    }
    pthread_detach(pthread_self());
    return NULL;
}

/* stop to consume message from broker */
ErrorCode nsemq_consumer_stop() {
    pthread_t   stop_thread;
    int         stop_res = -1;
    // 0.judge the run status.
    if (consumer_run_status_ != START_STATUS) {
        nsemq_write_error(consumer_, "Failed to stop: only allow to stop consume after called start().");
        return ERR_C_RUN_STATUS;
    }
    // 1.create new thread, and traverse the topic map and stop consumption.
    stop_res = pthread_create(&stop_thread, NULL, stop_thread_func, NULL);
    if(stop_res != 0){
        sprintf(strtemp_, "Failed to create stop thread.");
        nsemq_write_error(consumer_, strtemp_);
        return ERR_C_START_CREATE_THREAD;
    }
    pthread_detach(stop_thread);
    // 2.set stop status, ensure that thread can be cancelled.
    pthread_mutex_lock(&status_mutex);
    consumer_run_status_ = STOP_STATUS;
    pthread_mutex_unlock(&status_mutex);
    nsemq_write_info(consumer_, "Stopped consuming messages.");
    return ERR_NO_ERROR;
}

void* close_thread_func(void *agr) {
    const char *key;
    TopicItem *topic_item;
    map_iter_t iter_destroy;
    int        max_wait_time = 5;
    // NO.0 flush the local queue.
    while (rd_kafka_outq_len(consumer_) > 0) {
        rd_kafka_poll(consumer_, 10);
    }
    // NO.1 destroy topic object, and clear topic map.
    pthread_mutex_lock(&topic_mutex);
    iter_destroy = map_iter(&g_topic_map_);
    while ((key = map_next(&g_topic_map_, &iter_destroy))) {
        topic_item = map_get(&g_topic_map_, key);
        rd_kafka_topic_destroy(topic_item->topic_object);
    }
    map_deinit(&g_topic_map_);
    pthread_mutex_unlock(&topic_mutex);
    // NO.2 destroy topic queue.
    rd_kafka_queue_destroy(topic_queue_);
    // NO.3 destroy handle.
    rd_kafka_destroy(consumer_);
    // NO.4 let background threads clean up and terminate cleanly.
    while(max_wait_time-- > 0 &&  rd_kafka_wait_destroyed(1000) == -1);
    nsemq_write_info(NULL, "Successfully close the consumer.");
    pthread_detach(pthread_self());
    return NULL;
}

/* stop the consume message from broker and close consumer handle. */
ErrorCode nsemq_consumer_close() {
    pthread_t   close_thread;
    int         close_res = -1;
    // NO.0 judge the run status.
    if (consumer_run_status_ == CLOSE_STATUS) {
        nsemq_write_error(consumer_, "Failed to close: can't multiple called close() function.");
        return ERR_C_RUN_STATUS;
    } else if (consumer_run_status_ == START_STATUS) {
        // if in START_STATUS, to called stop() automatically.
        nsemq_consumer_stop();
    }
    // NO.1 create a new close thread.
    close_res = pthread_create(&close_thread, NULL, close_thread_func, NULL);
    if(close_res != 0){
        sprintf(strtemp_, "Failed to create close thread.");
        nsemq_write_error(consumer_, strtemp_);
        return ERR_C_START_CREATE_THREAD;
    }
    pthread_detach(close_thread);
    // NO.2 set CLOSE_STATUS.
    pthread_mutex_lock(&status_mutex);
    consumer_run_status_ = CLOSE_STATUS;
    pthread_mutex_unlock(&status_mutex);
    return ERR_NO_ERROR;
}
