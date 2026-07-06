// #include "sham.h"

// // Global variables
// FILE *log_file = NULL;
// int logging_enabled = 0;
// enum connection_state state = CLOSED;
// uint32_t client_seq = 100;
// uint32_t expected_ack = 0;
// uint32_t server_window = RECEIVER_BUFFER_SIZE; // Placeholder, actual value from server ACKs
// double loss_rate = 0.0;
// int chat_mode = 0;

// // Sliding window
// struct window_packet send_window[MAX_WINDOW_SIZE];
// int window_base = 0; // Index of the first packet in the window
// int window_next = 0; // Index to insert the next packet

// // Forward declarations
// void init_logging(const char *filename);
// void log_event(const char *format, ...);
// int should_drop_packet(double loss_rate);
// long long get_time_diff_us(struct timeval *start, struct timeval *end);
// struct sham_header hton_header(struct sham_header h);
// struct sham_header ntoh_header(struct sham_header h);
// void send_packet_client(int sockfd, struct sockaddr_in *server_addr, struct sham_header *header, char *data, int data_len);
// int establish_connection(int sockfd, struct sockaddr_in *server_addr);
// void handle_ack(uint32_t ack_num);
// void send_file(int sockfd, struct sockaddr_in *server_addr, const char *input_filename, const char *output_filename);
// void complete_termination_handshake(int sockfd, struct sockaddr_in *server_addr);

// // --- Logging and Utilities ---

// void init_logging(const char *filename) {
//     char *log_env = getenv("RUDP_LOG");
//     if (log_env && strcmp(log_env, "1") == 0) {
//         logging_enabled = 1;
//         log_file = fopen(filename, "w");
//         if (!log_file) {
//             perror("Failed to create log file");
//         }
//     }
// }

// void log_event(const char *format, ...) {
//     if (!logging_enabled || !log_file) return;

//     char time_buffer[30];
//     struct timeval tv;
//     time_t curtime;

//     gettimeofday(&tv, NULL);
//     curtime = tv.tv_sec;

//     strftime(time_buffer, 30, "%Y-%m-%d %H:%M:%S", localtime(&curtime));
//     fprintf(log_file, "[%s.%06ld] [LOG] ", time_buffer, (long)tv.tv_usec);

//     va_list args;
//     va_start(args, format);
//     vfprintf(log_file, format, args);
//     va_end(args);

//     fprintf(log_file, "\n");
//     fflush(log_file);
// }

// int should_drop_packet(double rate) {
//     if (rate > 0.0) {
//         return ((double)rand() / RAND_MAX) < rate;
//     }
//     return 0;
// }

// long long get_time_diff_us(struct timeval *start, struct timeval *end) {
//     return (end->tv_sec - start->tv_sec) * 1000000LL + (end->tv_usec - start->tv_usec);
// }

// struct sham_header hton_header(struct sham_header h) {
//     struct sham_header nh;
//     nh.seq_num = htonl(h.seq_num);
//     nh.ack_num = htonl(h.ack_num);
//     nh.flags = htons(h.flags);
//     nh.window_size = htons(h.window_size);
//     return nh;
// }

// struct sham_header ntoh_header(struct sham_header h) {
//     struct sham_header nh;
//     nh.seq_num = ntohl(h.seq_num);
//     nh.ack_num = ntohl(h.ack_num);
//     nh.flags = ntohs(h.flags);
//     nh.window_size = ntohs(h.window_size);
//     return nh;
// }

// void send_packet_client(int sockfd, struct sockaddr_in *server_addr, struct sham_header *header, char *data, int data_len) {
//     char buffer[sizeof(struct sham_header) + MAX_BUFFER_SIZE];

//     struct sham_header net_header = hton_header(*header);
//     memcpy(buffer, &net_header, sizeof(struct sham_header));

//     if (data && data_len > 0) {
//         memcpy(buffer + sizeof(struct sham_header), data, data_len);
//     }

//     sendto(sockfd, buffer, sizeof(struct sham_header) + data_len, 0,
//            (struct sockaddr*)server_addr, sizeof(*server_addr));
// }

// // --- Sliding Window and Protocol Logic ---

// void add_to_window(struct sham_header *header, char *data, int data_len) {
//     if (window_next < MAX_WINDOW_SIZE) {
//         send_window[window_next].header = *header;
//         memcpy(send_window[window_next].data, data, data_len);
//         send_window[window_next].data_len = data_len;
//         gettimeofday(&send_window[window_next].send_time, NULL);
//         send_window[window_next].acked = 0;
//         send_window[window_next].valid = 1;
//         window_next++;
//     } else {
//         log_event("SND_WINDOW FULL: Cannot add packet SEQ=%u", header->seq_num);
//     }
// }

// void handle_ack(uint32_t ack_num) {
//     log_event("RCV ACK=%u", ack_num);
//     int packets_cleared = 0;

//     // Mark packets as acknowledged (cumulative ACK)
//     for (int i = 0; i < window_next; i++) {
//         if (send_window[i].valid && !send_window[i].acked && send_window[i].header.seq_num < ack_num) {
//             send_window[i].acked = 1;
//             packets_cleared++;
//         }
//     }

//     // Slide the window: remove acknowledged packets from the front
//     if (packets_cleared > 0) {
//         while (window_base < window_next && send_window[window_base].acked) {
//             window_base++;
//         }
//         // Compact the window buffer
//         if (window_base > 0) {
//             memmove(send_window, &send_window[window_base], (window_next - window_base) * sizeof(struct window_packet));
//             window_next -= window_base;
//             window_base = 0;
//         }
//     }
// }

// void check_timeouts_and_retransmit(int sockfd, struct sockaddr_in *server_addr) {
//     struct timeval current_time;
//     gettimeofday(&current_time, NULL);

//     for (int i = 0; i < window_next; i++) {
//         if (send_window[i].valid && !send_window[i].acked) {
//             long long time_diff = get_time_diff_us(&send_window[i].send_time, &current_time);
//             if (time_diff >= TIMEOUT_MS) {
//                 log_event("TIMEOUT SEQ=%u", send_window[i].header.seq_num);
//                 send_packet_client(sockfd, server_addr, &send_window[i].header,
//                                  send_window[i].data, send_window[i].data_len);
//                 log_event("RETX DATA SEQ=%u LEN=%d", send_window[i].header.seq_num, send_window[i].data_len);
//                 gettimeofday(&send_window[i].send_time, NULL); // Reset timer after retransmission
//             }
//         }
//     }
// }

// // --- Connection Management ---

// int establish_connection(int sockfd, struct sockaddr_in *server_addr) {
//     // 1. Send SYN
//     struct sham_header syn_header;
//     syn_header.seq_num = client_seq;
//     syn_header.ack_num = 0;
//     syn_header.flags = SYN_FLAG;
//     syn_header.window_size = RECEIVER_BUFFER_SIZE; // Client's initial receive window

//     send_packet_client(sockfd, server_addr, &syn_header, NULL, 0);
//     log_event("SND SYN SEQ=%u", syn_header.seq_num);

//     state = SYN_SENT;
//     expected_ack = client_seq + 1; // Expect ACK for SYN acks a sequence number + 1 byte

//     // 2. Wait for SYN-ACK
//     char buffer[sizeof(struct sham_header) + MAX_BUFFER_SIZE];
//     struct sockaddr_in from_addr;
//     socklen_t from_len = sizeof(from_addr);
//     fd_set read_fds;
//     struct timeval timeout;
//     int retries = 0;

//     while (retries < 5) {
//         FD_ZERO(&read_fds);
//         FD_SET(sockfd, &read_fds);
//         timeout.tv_sec = 1;
//         timeout.tv_usec = 0;

//         int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);

//         if (activity > 0) {
//             ssize_t bytes_received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&from_addr, &from_len);
//             if (bytes_received >= (ssize_t)sizeof(struct sham_header)) {
//                 struct sham_header received_header = ntoh_header(*(struct sham_header*)buffer);

//                 if (state == SYN_SENT && (received_header.flags & SYN_FLAG) && (received_header.flags & ACK_FLAG) && received_header.ack_num == expected_ack) {
//                     log_event("RCV SYN-ACK SEQ=%u ACK=%u", received_header.seq_num, received_header.ack_num);
//                     server_window = received_header.window_size;

//                     // 3. Send ACK
//                     struct sham_header ack_header;
//                     client_seq = expected_ack; // Update sequence number
//                     ack_header.seq_num = client_seq;
//                     ack_header.ack_num = received_header.seq_num + 1; // ACK server's SYN sequence number
//                     ack_header.flags = ACK_FLAG;
//                     ack_header.window_size = RECEIVER_BUFFER_SIZE;

//                     send_packet_client(sockfd, server_addr, &ack_header, NULL, 0);
//                     log_event("SND ACK=%u WIN=%u", ack_header.ack_num, ack_header.window_size);

//                     state = ESTABLISHED;
//                     return 1; // Success
//                 }
//             }
//         } else if (activity == 0) {
//             // Timeout: retransmit SYN
//             log_event("TIMEOUT: Resending SYN");
//             send_packet_client(sockfd, server_addr, &syn_header, NULL, 0);
//             retries++;
//         } else {
//             perror("select error during handshake");
//             return 0;
//         }
//     }
//     fprintf(stderr, "Connection timed out during handshake.\n");
//     return 0; // Failure
// }

// /**
//  * @brief Handles the final part of the four-way termination handshake.
//  * Assumes FIN has already been sent by this client (state == FIN_WAIT_1).
//  * Waits for ACK from server, then waits for FIN from server, then sends final ACK.
//  */
// void complete_termination_handshake(int sockfd, struct sockaddr_in *server_addr) {
//     char recv_buffer[sizeof(struct sham_header) + MAX_BUFFER_SIZE];
//     struct sockaddr_in from_addr;
//     socklen_t from_len = sizeof(from_addr);
//     fd_set read_fds;
//     struct timeval timeout;
//     int close_retry = 0;

//     while (state != CLOSED && close_retry < 10) {
//         FD_ZERO(&read_fds);
//         FD_SET(sockfd, &read_fds);
//         timeout.tv_sec = 1;
//         timeout.tv_usec = 0;

//         int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);

//         if (activity > 0) {
//             ssize_t bytes_received = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr*)&from_addr, &from_len);
//             if (bytes_received >= (ssize_t)sizeof(struct sham_header)) {
//                 struct sham_header received_header = ntoh_header(*(struct sham_header*)recv_buffer);

//                 if (state == FIN_WAIT_1 && (received_header.flags & ACK_FLAG)) {
//                     // Server acknowledged our FIN. Move to FIN_WAIT_2.
//                     log_event("RCV ACK FOR FIN");
//                     state = FIN_WAIT_2;
//                 }

//                 if (state == FIN_WAIT_2 && (received_header.flags & FIN_FLAG)) {
//                     // Server sent its FIN. Send final ACK and close.
//                     log_event("RCV FIN SEQ=%u", received_header.seq_num);
//                     struct sham_header final_ack;
//                     final_ack.seq_num = client_seq;
//                     final_ack.ack_num = received_header.seq_num + 1;
//                     final_ack.flags = ACK_FLAG;
//                     final_ack.window_size = RECEIVER_BUFFER_SIZE;

//                     send_packet_client(sockfd, server_addr, &final_ack, NULL, 0);
//                     log_event("SND ACK=%u", final_ack.ack_num);
//                     state = CLOSED;
//                 }
//             }
//         } else if (activity == 0) {
//             log_event("TIMEOUT waiting for server FIN/ACK");
//             close_retry++;
//         } else {
//             perror("select error during termination");
//             break;
//         }
//     }
//     if (state != CLOSED) {
//         log_event("Termination handshake incomplete. Forcing close.");
//     }
// }

// // --- Application Modes ---

// void send_file(int sockfd, struct sockaddr_in *server_addr, const char *input_filename, const char *output_filename) {
//     FILE *file = fopen(input_filename, "rb");
//     if (!file) {
//         perror("Failed to open input file");
//         return;
//     }

//     char file_buffer[MAX_BUFFER_SIZE];
//     char recv_buffer[sizeof(struct sham_header) + MAX_BUFFER_SIZE];
//     struct sockaddr_in from_addr;
//     socklen_t from_len = sizeof(from_addr);
//     fd_set read_fds;
//     struct timeval timeout;
//     int file_complete = 0;

//     // --- Step 1: Send filename packet first ---
//     // Note: A more robust implementation would wait for this ACK specifically
//     // before starting file transfer. Here we add it to the window and proceed.
//     struct sham_header name_header;
//     name_header.seq_num = client_seq;
//     name_header.ack_num = 0;
//     name_header.flags = 0; // Data packet
//     name_header.window_size = RECEIVER_BUFFER_SIZE;
//     int output_filename_len = strlen(output_filename) + 1; // Include null terminator

//     send_packet_client(sockfd, server_addr, &name_header, (char*)output_filename, output_filename_len);
//     log_event("SND DATA (filename) SEQ=%u LEN=%d FILENAME=%s", name_header.seq_num, output_filename_len, output_filename);
//     add_to_window(&name_header, (char*)output_filename, output_filename_len);
//     client_seq += output_filename_len;

//     // --- Step 2: Main data transfer loop ---
//     while (!file_complete || window_next > 0) {
//         // Send new packets if window allows and file not complete
//         while (!file_complete && window_next < MAX_WINDOW_SIZE) {
//             size_t bytes_read = fread(file_buffer, 1, MAX_BUFFER_SIZE, file);

//             if (bytes_read > 0) {
//                 struct sham_header data_header;
//                 data_header.seq_num = client_seq;
//                 data_header.ack_num = 0;
//                 data_header.flags = 0;
//                 data_header.window_size = RECEIVER_BUFFER_SIZE;

//                 send_packet_client(sockfd, server_addr, &data_header, file_buffer, bytes_read);
//                 log_event("SND DATA SEQ=%u LEN=%d", data_header.seq_num, bytes_read);
//                 add_to_window(&data_header, file_buffer, bytes_read);
//                 client_seq += bytes_read;
//             } else if (feof(file)) {
//                 file_complete = 1;
//                 break;
//             } else {
//                 perror("File read error");
//                 file_complete = 1; // Exit loop on error
//                 break;
//             }
//         }

//         // Wait for ACKs or timeout
//         FD_ZERO(&read_fds);
//         FD_SET(sockfd, &read_fds);
//         timeout.tv_sec = 0;
//         timeout.tv_usec = 100000; // 100ms select timeout

//         int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);

//         if (activity > 0) {
//             ssize_t bytes_received = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr*)&from_addr, &from_len);
//             if (bytes_received >= (ssize_t)sizeof(struct sham_header)) {
//                 struct sham_header received_header = ntoh_header(*(struct sham_header*)recv_buffer);
//                 if (received_header.flags & ACK_FLAG) {
//                     server_window = received_header.window_size;
//                     handle_ack(received_header.ack_num);
//                 }
//             }
//         } else if (activity < 0 && errno != EINTR) {
//             perror("select failed during file transfer");
//             break;
//         }

//         // Check for timeouts and retransmit unacked packets in window
//         check_timeouts_and_retransmit(sockfd, server_addr);
//     }

//     fclose(file);

//     // --- Step 3: Initiate termination ---
//     struct sham_header fin_header;
//     fin_header.seq_num = client_seq;
//     fin_header.ack_num = 0;
//     fin_header.flags = FIN_FLAG;
//     fin_header.window_size = RECEIVER_BUFFER_SIZE;

//     send_packet_client(sockfd, server_addr, &fin_header, NULL, 0);
//     log_event("SND FIN SEQ=%u", fin_header.seq_num);
//     state = FIN_WAIT_1;

//     complete_termination_handshake(sockfd, server_addr);
// }

// void chat_mode_loop(int sockfd, struct sockaddr_in *server_addr) {
//     char input[MAX_BUFFER_SIZE];
//     char recv_buffer[sizeof(struct sham_header) + MAX_BUFFER_SIZE];
//     struct sockaddr_in from_addr;
//     socklen_t from_len = sizeof(from_addr);
//     fd_set read_fds;
//     int max_fd = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;

//     printf("Chat mode started. Type /quit to exit.\n> ");
//     fflush(stdout);

//     while (state == ESTABLISHED) {
//         FD_ZERO(&read_fds);
//         FD_SET(sockfd, &read_fds);
//         FD_SET(STDIN_FILENO, &read_fds);

//         // Check timeouts for retransmission periodically
//         check_timeouts_and_retransmit(sockfd, server_addr);

//         struct timeval select_timeout;
//         select_timeout.tv_sec = 0;
//         select_timeout.tv_usec = 500000; // 500ms timeout for retransmission check loop

//         int activity = select(max_fd + 1, &read_fds, NULL, NULL, &select_timeout);

//         if (activity < 0) {
//             if (errno == EINTR) continue;
//             perror("select failed in chat mode");
//             break;
//         }

//         // Network input processing
//         if (activity > 0 && FD_ISSET(sockfd, &read_fds)) {
//             ssize_t bytes_received = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr*)&from_addr, &from_len);

//             if (bytes_received >= (ssize_t)sizeof(struct sham_header)) {
//                 struct sham_header received_header = ntoh_header(*(struct sham_header*)recv_buffer);
//                 char *data = recv_buffer + sizeof(struct sham_header);
//                 int data_len = bytes_received - sizeof(struct sham_header);

//                 // --- FIX: Process ACKs for sent chat messages ---
//                 if (received_header.flags & ACK_FLAG) {
//                     handle_ack(received_header.ack_num);
//                 }

//                 // Process incoming data payload
//                 if (data_len > 0) {
//                     if (!should_drop_packet(loss_rate)) {
//                         log_event("RCV DATA SEQ=%u LEN=%d", received_header.seq_num, data_len);
//                         printf("\rServer: %.*s\n> ", data_len, data);
//                         fflush(stdout);

//                         // Send ACK for received data
//                         struct sham_header ack_header;
//                         ack_header.seq_num = client_seq;
//                         ack_header.ack_num = received_header.seq_num + data_len;
//                         ack_header.flags = ACK_FLAG;
//                         ack_header.window_size = RECEIVER_BUFFER_SIZE;
//                         send_packet_client(sockfd, server_addr, &ack_header, NULL, 0);
//                         log_event("SND ACK=%u", ack_header.ack_num);

//                     } else {
//                         log_event("DROP DATA SEQ=%u", received_header.seq_num);
//                     }
//                 }

//                 // Process FIN from server initiated close
//                 if (received_header.flags & FIN_FLAG) {
//                     log_event("RCV FIN SEQ=%u", received_header.seq_num);
//                     state = CLOSE_WAIT; // Server initiated close

//                     // Send ACK for server FIN
//                     struct sham_header ack_header;
//                     ack_header.seq_num = client_seq;
//                     ack_header.ack_num = received_header.seq_num + 1;
//                     ack_header.flags = ACK_FLAG;
//                     ack_header.window_size = RECEIVER_BUFFER_SIZE;
//                     send_packet_client(sockfd, server_addr, &ack_header, NULL, 0);

//                     // Send our own FIN
//                     struct sham_header fin_header;
//                     fin_header.seq_num = client_seq;
//                     fin_header.ack_num = 0;
//                     fin_header.flags = FIN_FLAG;
//                     fin_header.window_size = RECEIVER_BUFFER_SIZE;
//                     send_packet_client(sockfd, server_addr, &fin_header, NULL, 0);
//                     log_event("SND FIN SEQ=%u", fin_header.seq_num);
//                     state = LAST_ACK; // Transition state, main loop will exit
//                     break; // Exit chat loop immediately
//                 }
//             }
//         }

//         // Keyboard input processing
//         if (activity > 0 && FD_ISSET(STDIN_FILENO, &read_fds)) {
//             if (fgets(input, sizeof(input), stdin)) {
//                 if (strncmp(input, "/quit", 5) == 0) {
//                     // Initiate FIN handshake
//                     struct sham_header fin_header;
//                     fin_header.seq_num = client_seq;
//                     fin_header.ack_num = 0;
//                     fin_header.flags = FIN_FLAG;
//                     fin_header.window_size = RECEIVER_BUFFER_SIZE;

//                     send_packet_client(sockfd, server_addr, &fin_header, NULL, 0);
//                     log_event("SND FIN SEQ=%u", fin_header.seq_num);
//                     state = FIN_WAIT_1;
//                     break; // Exit chat loop to proceed to termination handshake
//                 } else {
//                     // Send chat message as data packet
//                     struct sham_header data_header;
//                     data_header.seq_num = client_seq;
//                     data_header.ack_num = 0;
//                     data_header.flags = 0;
//                     data_header.window_size = RECEIVER_BUFFER_SIZE;

//                     int input_len = strlen(input);
//                     send_packet_client(sockfd, server_addr, &data_header, input, input_len);
//                     log_event("SND DATA SEQ=%u LEN=%d", client_seq, input_len);

//                     // Add to retransmission window
//                     add_to_window(&data_header, input, input_len);
//                     client_seq += input_len;
//                     printf("> ");
//                     fflush(stdout);
//                 }
//             }
//         }
//     }
// }

// // --- Main Function ---

// int main(int argc, char *argv[]) {
//     setvbuf(stdout, NULL, _IONBF, 0); // Disable stdout buffering

//     if (argc < 3) {
//         fprintf(stderr, "Usage:\n");
//         fprintf(stderr, "  File Transfer: %s <server_ip> <server_port> <input_file> <output_file_name> [loss_rate]\n", argv[0]);
//         fprintf(stderr, "  Chat Mode:     %s <server_ip> <server_port> --chat [loss_rate]\n", argv[0]);
//         exit(1);
//     }

//     char *server_ip = argv[1];
//     int server_port = atoi(argv[2]);
//     char *input_file = NULL;
//     char *output_file = NULL;

//     // Parse arguments
//     if (argc > 3 && strcmp(argv[3], "--chat") == 0) {
//         chat_mode = 1;
//         if (argc > 4) loss_rate = atof(argv[4]);
//     } else {
//         if (argc < 5) {
//             fprintf(stderr, "File transfer mode requires input and output file names.\n");
//             exit(1);
//         }
//         input_file = argv[3];
//         output_file = argv[4];
//         if (argc > 5) loss_rate = atof(argv[5]);
//     }

//     init_logging("client_log.txt");
//     srand(time(NULL));
//     memset(send_window, 0, sizeof(send_window));

//     int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
//     if (sockfd < 0) {
//         perror("socket creation failed");
//         exit(1);
//     }

//     struct sockaddr_in server_addr;
//     memset(&server_addr, 0, sizeof(server_addr));
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(server_port);
//     inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

//     // Establish connection
//     if (!establish_connection(sockfd, &server_addr)) {
//         fprintf(stderr, "Failed to establish connection with server.\n");
//         close(sockfd);
//         exit(1);
//     }

//     printf("Connection established.\n");

//     // Start application mode logic
//     if (chat_mode) {
//         chat_mode_loop(sockfd, &server_addr);
//         if (state == FIN_WAIT_1) {
//             complete_termination_handshake(sockfd, &server_addr);
//         }
//     } else {
//         send_file(sockfd, &server_addr, input_file, output_file);
//     }

//     printf("Connection closed.\n");
//     close(sockfd);
//     if (log_file) fclose(log_file);
//     return 0;
// }


#include "sham.h"

// Global variables
FILE *log_file = NULL;
int logging_enabled = 0;
enum connection_state state = CLOSED;
uint32_t client_seq = 100;
uint32_t last_ack_received = 101; // For flow control
uint32_t server_window = RECEIVER_BUFFER_SIZE;
double loss_rate = 0.0;
int chat_mode = 0;

// Sliding window
struct window_packet send_window[MAX_WINDOW_SIZE];
int window_next_idx = 0;

// --- Logging and Utilities ---
void init_logging(const char *filename) {
    char *log_env = getenv("RUDP_LOG");
    if (log_env && strcmp(log_env, "1") == 0) {
        logging_enabled = 1;
        log_file = fopen(filename, "w");
        if (!log_file) perror("Failed to create log file");
    }
}

void log_event(const char *format, ...) {
    if (!logging_enabled || !log_file) return;
    char time_buffer[30];
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t curtime = tv.tv_sec;
    strftime(time_buffer, 30, "%Y-%m-%d %H:%M:%S", localtime(&curtime));
    fprintf(log_file, "[%s.%06ld] [LOG] ", time_buffer, (long)tv.tv_usec);
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    fprintf(log_file, "\n");
    fflush(log_file);
}

long long get_time_diff_us(struct timeval *start, struct timeval *end) {
    return (end->tv_sec - start->tv_sec) * 1000000LL + (end->tv_usec - start->tv_usec);
}

// --- Network and Header Utilities ---
struct sham_header hton_header(struct sham_header h) {
    struct sham_header nh;
    nh.seq_num = htonl(h.seq_num);
    nh.ack_num = htonl(h.ack_num);
    nh.flags = htons(h.flags);
    nh.window_size = htons(h.window_size);
    return nh;
}

struct sham_header ntoh_header(struct sham_header h) {
    struct sham_header nh;
    nh.seq_num = ntohl(h.seq_num);
    nh.ack_num = ntohl(h.ack_num);
    nh.flags = ntohs(h.flags);
    nh.window_size = ntohs(h.window_size);
    return nh;
}

void send_packet_client(int sockfd, struct sockaddr_in *addr, struct sham_header *h, char *data, int len) {
    char buffer[sizeof(struct sham_header) + MAX_BUFFER_SIZE];
    struct sham_header net_header = hton_header(*h);
    memcpy(buffer, &net_header, sizeof(struct sham_header));
    if (data && len > 0) memcpy(buffer + sizeof(struct sham_header), data, len);
    sendto(sockfd, buffer, sizeof(struct sham_header) + len, 0, (struct sockaddr*)addr, sizeof(*addr));
}

// --- Sliding Window and Protocol Logic ---

void add_to_window(struct sham_header *header, char *data, int data_len) {
    if (window_next_idx < MAX_WINDOW_SIZE) {
        send_window[window_next_idx].header = *header;
        memcpy(send_window[window_next_idx].data, data, data_len);
        send_window[window_next_idx].data_len = data_len;
        gettimeofday(&send_window[window_next_idx].send_time, NULL);
        send_window[window_next_idx].acked = 0;
        send_window[window_next_idx].valid = 1;
        window_next_idx++;
    }
}

void handle_ack(uint32_t ack_num) {
    log_event("RCV ACK=%u", ack_num);
    last_ack_received = ack_num;
    int slide_count = 0;
    for (int i = 0; i < window_next_idx; i++) {
        if (send_window[i].valid && !send_window[i].acked && (send_window[i].header.seq_num + send_window[i].data_len) <= ack_num) {
            send_window[i].acked = 1;
        }
    }
    while (slide_count < window_next_idx && send_window[slide_count].acked) {
        slide_count++;
    }
    if (slide_count > 0) {
        memmove(send_window, &send_window[slide_count], (window_next_idx - slide_count) * sizeof(struct window_packet));
        window_next_idx -= slide_count;
    }
}

void check_timeouts_and_retransmit(int sockfd, struct sockaddr_in *server_addr) {
    struct timeval now;
    gettimeofday(&now, NULL);
    for (int i = 0; i < window_next_idx; i++) {
        if (send_window[i].valid && !send_window[i].acked) {
            if (get_time_diff_us(&send_window[i].send_time, &now) >= TIMEOUT_MS) {
                log_event("TIMEOUT SEQ=%u", send_window[i].header.seq_num);
                send_packet_client(sockfd, server_addr, &send_window[i].header, send_window[i].data, send_window[i].data_len);
                log_event("RETX DATA SEQ=%u LEN=%d", send_window[i].header.seq_num, send_window[i].data_len);
                gettimeofday(&send_window[i].send_time, NULL);
            }
        }
    }
}

// --- Connection Management ---

int establish_connection(int sockfd, struct sockaddr_in *server_addr) {
    struct sham_header syn = { .seq_num = client_seq, .flags = SYN_FLAG, .window_size = RECEIVER_BUFFER_SIZE };
    send_packet_client(sockfd, server_addr, &syn, NULL, 0);
    log_event("SND SYN SEQ=%u", syn.seq_num);
    state = SYN_SENT;

    for (int retries = 0; retries < 5; retries++) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        struct timeval timeout = {1, 0};
        int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);

        if (activity > 0) {
            char buffer[MAX_BUFFER_SIZE];
            ssize_t bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
            if (bytes >= (ssize_t)sizeof(struct sham_header)) {
                struct sham_header rh = ntoh_header(*(struct sham_header*)buffer);
                if ((rh.flags & (SYN_FLAG | ACK_FLAG)) && rh.ack_num == client_seq + 1) {
                    log_event("RCV SYN-ACK SEQ=%u ACK=%u", rh.seq_num, rh.ack_num);
                    server_window = rh.window_size;
                    log_event("FLOW WIN UPDATE=%u", server_window);
                    client_seq++;
                    struct sham_header ack = { .seq_num = client_seq, .ack_num = rh.seq_num + 1, .flags = ACK_FLAG, .window_size = RECEIVER_BUFFER_SIZE };
                    send_packet_client(sockfd, server_addr, &ack, NULL, 0);
                    log_event("SND ACK=%u WIN=%u", ack.ack_num, ack.window_size);
                    state = ESTABLISHED;
                    return 1;
                }
            }
        } else {
            log_event("TIMEOUT: Resending SYN");
            send_packet_client(sockfd, server_addr, &syn, NULL, 0);
        }
    }
    return 0;
}

void complete_termination(int sockfd, struct sockaddr_in *server_addr) {
    while (state != CLOSED) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        struct timeval timeout = {1, 0};
        int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
        if (activity > 0) {
            char buffer[MAX_BUFFER_SIZE];
            ssize_t bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
            if (bytes >= (ssize_t)sizeof(struct sham_header)) {
                struct sham_header rh = ntoh_header(*(struct sham_header*)buffer);
                if (state == FIN_WAIT_1 && (rh.flags & ACK_FLAG)) {
                    log_event("RCV ACK FOR FIN");
                    state = FIN_WAIT_2;
                }
                if (state == FIN_WAIT_2 && (rh.flags & FIN_FLAG)) {
                    log_event("RCV FIN SEQ=%u", rh.seq_num);
                    struct sham_header final_ack = { .seq_num = client_seq, .ack_num = rh.seq_num + 1, .flags = ACK_FLAG, .window_size = RECEIVER_BUFFER_SIZE };
                    send_packet_client(sockfd, server_addr, &final_ack, NULL, 0);
                    log_event("SND ACK=%u", final_ack.ack_num);
                    state = CLOSED;
                }
            }
        } else {
            log_event("TIMEOUT waiting for server FIN/ACK");
        }
    }
}

// --- Application Modes ---

void send_file(int sockfd, struct sockaddr_in *server_addr, const char *in_file, const char *out_file) {
    FILE *file = fopen(in_file, "rb");
    if (!file) {
        perror("Failed to open input file");
        return;
    }

    struct sham_header name_h = { .seq_num = client_seq, .window_size = RECEIVER_BUFFER_SIZE };
    int out_len = strlen(out_file) + 1;
    send_packet_client(sockfd, server_addr, &name_h, (char*)out_file, out_len);
    log_event("SND DATA (filename) SEQ=%u LEN=%d FILENAME=%s", name_h.seq_num, out_len, out_file);
    add_to_window(&name_h, (char*)out_file, out_len);
    client_seq += out_len;

    int file_done = 0;
    while (!file_done || window_next_idx > 0) {
        uint32_t bytes_in_flight = client_seq - last_ack_received;
        while (!file_done && window_next_idx < MAX_WINDOW_SIZE && bytes_in_flight < server_window) {
            char file_buf[MAX_BUFFER_SIZE];
            size_t read_bytes = fread(file_buf, 1, MAX_BUFFER_SIZE, file);
            if (read_bytes > 0) {
                struct sham_header data_h = { .seq_num = client_seq, .window_size = RECEIVER_BUFFER_SIZE };
                send_packet_client(sockfd, server_addr, &data_h, file_buf, read_bytes);
                log_event("SND DATA SEQ=%u LEN=%d", data_h.seq_num, read_bytes);
                add_to_window(&data_h, file_buf, read_bytes);
                client_seq += read_bytes;
                bytes_in_flight += read_bytes;
            } else {
                file_done = 1;
                break;
            }
        }

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        struct timeval timeout = {0, 100000}; // 100ms
        int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);

        if (activity > 0) {
            char recv_buf[MAX_BUFFER_SIZE];
            ssize_t bytes = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, NULL, NULL);
            if (bytes >= (ssize_t)sizeof(struct sham_header)) {
                struct sham_header rh = ntoh_header(*(struct sham_header*)recv_buf);
                if (rh.flags & ACK_FLAG) {
                    if (server_window != rh.window_size) {
                        server_window = rh.window_size;
                        log_event("FLOW WIN UPDATE=%u", server_window);
                    }
                    handle_ack(rh.ack_num);
                }
            }
        }
        check_timeouts_and_retransmit(sockfd, server_addr);
    }
    fclose(file);

    struct sham_header fin = { .seq_num = client_seq, .flags = FIN_FLAG, .window_size = RECEIVER_BUFFER_SIZE };
    send_packet_client(sockfd, server_addr, &fin, NULL, 0);
    log_event("SND FIN SEQ=%u", fin.seq_num);
    state = FIN_WAIT_1;
    complete_termination(sockfd, server_addr);
}

void chat_mode_loop(int sockfd, struct sockaddr_in *server_addr) {
    printf("Chat mode started. Type /quit to exit.\n> ");
    fflush(stdout);

    while (state == ESTABLISHED) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        int max_fd = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;

        struct timeval timeout = {0, 500000}; // 500ms
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

        if (activity < 0 && errno != EINTR) {
            perror("select failed in chat mode");
            break;
        }

        if (activity > 0 && FD_ISSET(sockfd, &read_fds)) {
            char recv_buf[MAX_BUFFER_SIZE];
            ssize_t bytes = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, NULL, NULL);
            if (bytes >= (ssize_t)sizeof(struct sham_header)) {
                struct sham_header rh = ntoh_header(*(struct sham_header*)recv_buf);
                char *data = recv_buf + sizeof(struct sham_header);
                int data_len = bytes - sizeof(struct sham_header);

                if (rh.flags & ACK_FLAG) handle_ack(rh.ack_num);
                if (data_len > 0) {
                    log_event("RCV DATA SEQ=%u LEN=%d", rh.seq_num, data_len);
                    printf("\rServer: %.*s\n> ", data_len, data);
                    fflush(stdout);
                    struct sham_header ack = { .seq_num = client_seq, .ack_num = rh.seq_num + data_len, .flags = ACK_FLAG, .window_size = RECEIVER_BUFFER_SIZE };
                    send_packet_client(sockfd, server_addr, &ack, NULL, 0);
                    log_event("SND ACK=%u", ack.ack_num);
                }
                if (rh.flags & FIN_FLAG) {
                     log_event("RCV FIN SEQ=%u", rh.seq_num);
                    state = CLOSE_WAIT;
                    struct sham_header ack_for_fin = { .seq_num = client_seq, .ack_num = rh.seq_num + 1, .flags = ACK_FLAG, .window_size = RECEIVER_BUFFER_SIZE };
                    send_packet_client(sockfd, server_addr, &ack_for_fin, NULL, 0);

                    struct sham_header fin = { .seq_num = client_seq, .flags = FIN_FLAG, .window_size = RECEIVER_BUFFER_SIZE };
                    send_packet_client(sockfd, server_addr, &fin, NULL, 0);
                    log_event("SND FIN SEQ=%u", fin.seq_num);
                    state = LAST_ACK;
                    break;
                }
            }
        }

        if (activity > 0 && FD_ISSET(STDIN_FILENO, &read_fds)) {
            char input[MAX_BUFFER_SIZE];
            if (fgets(input, sizeof(input), stdin)) {
                if (strncmp(input, "/quit", 5) == 0) {
                    struct sham_header fin = { .seq_num = client_seq, .flags = FIN_FLAG, .window_size = RECEIVER_BUFFER_SIZE };
                    send_packet_client(sockfd, server_addr, &fin, NULL, 0);
                    log_event("SND FIN SEQ=%u", fin.seq_num);
                    state = FIN_WAIT_1;
                    break;
                } else {
                    struct sham_header data_h = { .seq_num = client_seq, .window_size = RECEIVER_BUFFER_SIZE };
                    int input_len = strlen(input);
                    send_packet_client(sockfd, server_addr, &data_h, input, input_len);
                    log_event("SND DATA SEQ=%u LEN=%d", client_seq, input_len);
                    add_to_window(&data_h, input, input_len);
                    client_seq += input_len;
                    printf("> ");
                    fflush(stdout);
                }
            }
        }
        check_timeouts_and_retransmit(sockfd, server_addr);
    }
}

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc < 4) {
        fprintf(stderr, "Usage:\n  File: %s <ip> <port> <in_file> <out_file> [loss]\n  Chat: %s <ip> <port> --chat [loss]\n", argv[0], argv[0]);
        exit(1);
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    char *input_file = NULL, *output_file = NULL;

    if (strcmp(argv[3], "--chat") == 0) {
        chat_mode = 1;
        if (argc > 4) loss_rate = atof(argv[4]);
    } else {
        if (argc < 5) {
            fprintf(stderr, "File mode needs input and output file names.\n");
            exit(1);
        }
        input_file = argv[3];
        output_file = argv[4];
        if (argc > 5) loss_rate = atof(argv[5]);
    }

    init_logging("client_log.txt");
    srand(time(NULL));
    memset(send_window, 0, sizeof(send_window));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    if (!establish_connection(sockfd, &server_addr)) {
        fprintf(stderr, "Failed to establish connection.\n");
        close(sockfd);
        exit(1);
    }

    printf("Connection established.\n");

    if (chat_mode) {
        chat_mode_loop(sockfd, &server_addr);
        if (state == FIN_WAIT_1) complete_termination(sockfd, &server_addr);
    } else {
        send_file(sockfd, &server_addr, input_file, output_file);
    }

    printf("Connection closed.\n");
    close(sockfd);
    if (log_file) fclose(log_file);
    return 0;
}