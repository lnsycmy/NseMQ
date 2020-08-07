#include "nsemq_consumer.h"

static rd_kafka_t *consumer_;         // Consumer instance handle
static rd_kafka_conf_t *consumer_conf_;  // Temporary configuration object
static rd_kafka_topic_conf_t *topic_conf_;
static char errstr_[512];       // librdkafka API error reporting buffer
static TopicList topic_list_;          // topic list.
static rd_kafka_queue_t *topic_queue_;
static rd_kafka_resp_err_t err_;
static int run_flag_;
// queue
static rd_kafka_queue_t *rkqu = NULL;

/* list function */
void insert_list(TopicList *topic_list, char *topic_name, void (*consume_callback)(rd_kafka_message_t *, void *)) {
    TopicList new = malloc(sizeof(struct TopicNode));
	TopicList cur = *topic_list;
    if (!new || consumer_ == NULL) {
        return;
    }
    // rd_kafka_topic_t *topic_object = rd_kafka_topic_new(consumer_, topic_name, NULL);
    new->topic_name = topic_name;
    // new->topic_object = topic_object;
    new->consume_callback = consume_callback;
    new->next = NULL;
    // 如果链表为空，直接插入新节点
    if (!*topic_list) {
        *topic_list = new;
        return;
    }
    // 遍历到最后一个后，在下一个插入新节点
   
    while (cur->next) {
        cur = cur->next;
    }
    cur->next = new;
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
    if (p1 == topic_list_) {
        *topic_list = p1->next;
        free(p1);
    } else if (p1 != NULL) {
        p2->next = p1->next;
        free(p1);
    } else {
        printf("不存在\n");
    }
}

void clear_list(TopicList *topic_list) {
    TopicList curNode = NULL;
    while (*topic_list) {
        curNode = *topic_list;
        topic_list_ = (*topic_list)->next;
        free(curNode);
    }
}

/* consumer function */
ErrorCode init(const char *broker_addr) {
    // Kafka configuration
    consumer_conf_ = rd_kafka_conf_new();
    // Topic configuration
    topic_conf_ = rd_kafka_topic_conf_new();
    // Create Kafka handle
    if (!(consumer_ = rd_kafka_new(RD_KAFKA_CONSUMER, consumer_conf_,
                                   errstr_, sizeof(errstr_)))) {
        fprintf(stderr,
                "%% Failed to create new consumer: %s\n",
                errstr_);
        return ERR_C_CREATE_CONSUMER;
    }
    // add brokers
    if (rd_kafka_brokers_add(consumer_, broker_addr) == 0) {
        fprintf(stderr, "%% No valid brokers specified\n");
        return ERR_C_INIT_BROKER_ADDRESS;
    }
    run_flag_ = 1;
    printf("finished init();\n");
    return ERR_NO_ERROR;
}

ErrorCode subscribe(char *topic_name, void (*consume_callback)(rd_kafka_message_t *, void *)) {
    // save the mapping between topic and msg_consume
    TopicList findNode = find_item(topic_list_, topic_name);
	rd_kafka_topic_t *new_topic_object = rd_kafka_topic_new(consumer_, topic_name, NULL);
	int res = 0;
	printf("enter subscribe();\n");
    if (findNode != NULL) {
        return ERR_NO_ERROR;  // have subscribed this topic
    }
    insert_list(&topic_list_, topic_name, consume_callback);
    findNode = find_item(topic_list_, topic_name);
    // start to consume message from broker, and save the message to local queue.
    res = rd_kafka_consume_start(new_topic_object, NSEMQ_DEFAULT_PARTITION, NSEMQ_DEFAULT_OFFSET);
    if (res == -1) {
        err_ = rd_kafka_last_error();
        fprintf(stderr, "%% Failed to start consuming: %s\n",
                rd_kafka_err2str(err_));
        if (err_ == RD_KAFKA_RESP_ERR__INVALID_ARG)
            fprintf(stderr,
                    "%% Broker based offset storage "
                    "requires a group.id, "
                    "add: -X group.id=yourGroup\n");
        delete_item(topic_list_, topic_name);
        return ERR_C_SUBS_CREATE_TOPIC;
    }
    return ERR_NO_ERROR;
}

ErrorCode subscription(TopicList *topic_array){
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
}

ErrorCode start() {
    // 线程可以使用pthread，更轻便一些
	rd_kafka_topic_t *new_topic_object = NULL;
    while (1) {
        TopicList curNode = topic_list_;
        while (curNode) {
            new_topic_object = rd_kafka_topic_new(consumer_, curNode->topic_name, NULL);
			printf("enter %s poll()\n", curNode->topic_name);
            rd_kafka_consume_callback(new_topic_object,
                                      NSEMQ_DEFAULT_PARTITION,
                                      1000,/* timeout_ms */
                                      curNode->consume_callback,/* consumer callback function */
                                      NULL/*	void* opaque */);
            curNode = curNode->next;
        }
        rd_kafka_poll(consumer_, 0);
    }
    return ERR_NO_ERROR;
}

// close the consumer.
ErrorCode close() {
    run_flag_ = 0;
    // Stop consuming
    while (topic_list_) {
        rd_kafka_consume_stop(topic_list_->topic_object, NSEMQ_DEFAULT_PARTITION);
        topic_list_ = topic_list_->next;
    }
    while (rd_kafka_outq_len(consumer_) > 0) {
        rd_kafka_poll(consumer_, 10);
    }
    /* Destroy topic */
    while (topic_list_) {
        rd_kafka_topic_destroy(topic_list_->topic_object);
        topic_list_ = topic_list_->next;
    }
    clear_list(topic_list_);
    /* Destroy handle */
    rd_kafka_destroy(consumer_);
}

ErrorCode subscribe2(const char *topic_name){

}

ErrorCode start2(){

}


