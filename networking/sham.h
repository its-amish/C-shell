// #ifndef SHAM_H
// #define SHAM_H

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <sys/time.h>
// #include <time.h>
// #include <sys/select.h>
// #include <errno.h>
// #include <stdarg.h>
// #include <openssl/md5.h>

// // Protocol constants
// #define MAX_BUFFER_SIZE 1024
// #define MAX_WINDOW_SIZE 10
// #define TIMEOUT_MS 500000 // 500ms in microseconds
// #define RECEIVER_BUFFER_SIZE 8192

// // S.H.A.M. Header Structure - packed to ensure consistent layout
// #pragma pack(push, 1)
// struct sham_header {
//     uint32_t seq_num;      // Sequence Number
//     uint32_t ack_num;      // Acknowledgment Number
//     uint16_t flags;        // Control flags (SYN, ACK, FIN)
//     uint16_t window_size;  // Flow control window size
// };
// #pragma pack(pop)

// // Protocol flags
// #define SYN_FLAG 0x1  // Synchronise - Used to initiate a connection
// #define ACK_FLAG 0x2  // Acknowledge - Indicates the ack_num field is significant
// #define FIN_FLAG 0x4  // Finish - Used to terminate a connection

// // Connection states
// enum connection_state {
//     CLOSED,
//     LISTEN,
//     SYN_SENT,
//     SYN_RECEIVED,
//     ESTABLISHED,
//     FIN_WAIT_1,
//     FIN_WAIT_2,
//     CLOSE_WAIT,
//     CLOSING,
//     LAST_ACK,
//     TIME_WAIT
// };

// // Sliding window packet structure for client sender window
// struct window_packet {
//     struct sham_header header;
//     char data[MAX_BUFFER_SIZE];
//     int data_len;
//     struct timeval send_time;
//     int acked;
//     int valid;
// };

// // Packet buffer for server receiver buffer (out-of-order packets)
// struct packet_buffer {
//     struct sham_header header;
//     char data[MAX_BUFFER_SIZE];
//     int data_len;
//     int valid;
// };

// // Function prototypes
// void init_logging(const char *filename);
// void log_event(const char *format, ...);
// int should_drop_packet(double loss_rate);
// long long get_time_diff_us(struct timeval *start, struct timeval *end);
// void calculate_md5(const char *filename);

// // Network byte order conversion functions
// struct sham_header hton_header(struct sham_header h);
// struct sham_header ntoh_header(struct sham_header h);

// #endif // SHAM_H

#ifndef SHAM_H
#define SHAM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <sys/select.h>
#include <errno.h>
#include <stdarg.h>
#include <openssl/md5.h>

// Protocol constants
#define MAX_BUFFER_SIZE 1024
#define MAX_WINDOW_SIZE 10
#define TIMEOUT_MS 500000 // 500ms in microseconds
#define RECEIVER_BUFFER_SIZE 8192

// S.H.A.M. Header Structure - packed to ensure consistent layout
#pragma pack(push, 1)
struct sham_header {
    uint32_t seq_num;      // Sequence Number
    uint32_t ack_num;      // Acknowledgment Number
    uint16_t flags;        // Control flags (SYN, ACK, FIN)
    uint16_t window_size;  // Flow control window size
};
#pragma pack(pop)

// Protocol flags
#define SYN_FLAG 0x1  // Synchronise - Used to initiate a connection
#define ACK_FLAG 0x2  // Acknowledge - Indicates the ack_num field is significant
#define FIN_FLAG 0x4  // Finish - Used to terminate a connection

// Connection states
enum connection_state {
    CLOSED,
    LISTEN,
    SYN_SENT,
    SYN_RECEIVED,
    ESTABLISHED,
    FIN_WAIT_1,
    FIN_WAIT_2,
    CLOSE_WAIT,
    CLOSING,
    LAST_ACK,
    TIME_WAIT
};

// Sliding window packet structure for client sender window
struct window_packet {
    struct sham_header header;
    char data[MAX_BUFFER_SIZE];
    int data_len;
    struct timeval send_time;
    int acked;
    int valid;
};

// Packet buffer for server receiver buffer (out-of-order packets)
struct packet_buffer {
    struct sham_header header;
    char data[MAX_BUFFER_SIZE];
    int data_len;
    int valid;
};

// Function prototypes
void init_logging(const char *filename);
void log_event(const char *format, ...);
int should_drop_packet(double loss_rate);
long long get_time_diff_us(struct timeval *start, struct timeval *end);
void calculate_md5(const char *filename);

// Network byte order conversion functions
struct sham_header hton_header(struct sham_header h);
struct sham_header ntoh_header(struct sham_header h);

#endif // SHAM_H