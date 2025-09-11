// #include "sham.h"


// // LLM GENERATDE CODE STARTS ////



// // Global variables
// FILE *log_file = NULL;
// int logging_enabled = 0;
// enum connection_state state = CLOSED;
// uint32_t server_seq = 5000;
// uint32_t expected_seq = 0;
// uint32_t receiver_window = RECEIVER_BUFFER_SIZE;
// double loss_rate = 0.0;
// int chat_mode = 0;

// // Packet buffer for out-of-order packets
// struct packet_buffer packet_buffers[MAX_WINDOW_SIZE * 2];

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

// void calculate_md5(const char *filename) {
//     FILE *file = fopen(filename, "rb");
//     if (!file) {
//         perror("Error opening received file for MD5 calculation");
//         return;
//     }

//     MD5_CTX md5_ctx;
//     MD5_Init(&md5_ctx);
//     unsigned char buffer[MAX_BUFFER_SIZE];
//     size_t bytes_read;

//     while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
//         MD5_Update(&md5_ctx, buffer, bytes_read);
//     }

//     unsigned char digest[MD5_DIGEST_LENGTH];
//     MD5_Final(digest, &md5_ctx);
//     fclose(file);

//     printf("MD5: ");
//     for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
//         printf("%02x", digest[i]);
//     }
//     printf("\n");
// }

// void send_packet_server(int sockfd, struct sockaddr_in *client_addr, struct sham_header *header, char *data, int data_len) {
//     char buffer[sizeof(struct sham_header) + MAX_BUFFER_SIZE];
//     struct sham_header net_header = hton_header(*header);
//     memcpy(buffer, &net_header, sizeof(struct sham_header));

//     if (data && data_len > 0) {
//         memcpy(buffer + sizeof(struct sham_header), data, data_len);
//     }

//     sendto(sockfd, buffer, sizeof(struct sham_header) + data_len, 0,
//            (struct sockaddr*)client_addr, sizeof(*client_addr));
// }

// // --- Protocol Logic Handlers ---

// void handle_syn(int sockfd, struct sockaddr_in *client_addr, struct sham_header *received_header) {
//     log_event("RCV SYN SEQ=%u", received_header->seq_num);

//     // Set initial expected sequence number from client's initial sequence number
//     expected_seq = received_header->seq_num + 1;
//     state = SYN_RECEIVED;

//     // Send SYN-ACK
//     struct sham_header response;
//     response.seq_num = server_seq;
//     response.ack_num = expected_seq;
//     response.flags = SYN_FLAG | ACK_FLAG;
//     response.window_size = receiver_window;

//     send_packet_server(sockfd, client_addr, &response, NULL, 0);
//     log_event("SND SYN-ACK SEQ=%u ACK=%u", response.seq_num, response.ack_num);
//     server_seq++; // Increment sequence number after sending SYN-ACK
// }

// void handle_ack_for_syn(struct sham_header *received_header) {
//     if (received_header->ack_num == server_seq && state == SYN_RECEIVED) {
//         log_event("RCV ACK FOR SYN");
//         state = ESTABLISHED;
//     } else {
//         log_event("RCV DUP ACK FOR SYN (ignored)");
//     }
// }

// void buffer_packet(struct sham_header *header, char *data, int data_len) {
//     for (int i = 0; i < MAX_WINDOW_SIZE * 2; i++) {
//         if (!packet_buffers[i].valid) {
//             packet_buffers[i].header = *header;
//             memcpy(packet_buffers[i].data, data, data_len);
//             packet_buffers[i].data_len = data_len;
//             packet_buffers[i].valid = 1;
//             log_event("BUFFERED PKT SEQ=%u", header->seq_num);
//             return;
//         }
//     }
//     log_event("BUFFER FULL: Dropped pkt SEQ=%u", header->seq_num);
// }

// /**
//  * @brief Processes buffered packets in sequence after an in-order packet arrives.
//  * @param output_file File handle to write to in file transfer mode.
//  * @param chat_mode_flag Flag to indicate if we should print to stdout in chat mode.
//  */
// void process_buffered_packets(FILE *output_file, int chat_mode_flag) {
//     int found_in_order_packet = 1;
//     while (found_in_order_packet) {
//         found_in_order_packet = 0;
//         for (int i = 0; i < MAX_WINDOW_SIZE * 2; i++) {
//             if (packet_buffers[i].valid && packet_buffers[i].header.seq_num == expected_seq) {
//                 log_event("PROCESS BUFFERED PKT SEQ=%u", packet_buffers[i].header.seq_num);

//                 if (output_file) {
//                     fwrite(packet_buffers[i].data, 1, packet_buffers[i].data_len, output_file);
//                     fflush(output_file);
//                 } else if (chat_mode_flag) {
//                     printf("\rClient: %.*s\n> ", packet_buffers[i].data_len, packet_buffers[i].data);
//                     fflush(stdout);
//                 }

//                 expected_seq += packet_buffers[i].data_len;
//                 packet_buffers[i].valid = 0;
//                 found_in_order_packet = 1;
//                 break; // Restart search from beginning for next sequence number
//             }
//         }
//     }
// }

// /**
//  * @brief Handles incoming data packets.
//  * Manages dynamic file opening for file transfer mode and data display for chat mode.
//  * Handles out-of-order buffering and sends cumulative ACKs.
//  */
// void handle_data(int sockfd, struct sockaddr_in *client_addr, struct sham_header *received_header,
//                 char *data, int data_len, FILE **output_file_ptr, int *file_opened) {

//     if (should_drop_packet(loss_rate)) {
//         log_event("DROP DATA SEQ=%u", received_header->seq_num);
//         return;
//     }

//     log_event("RCV DATA SEQ=%u LEN=%d", received_header->seq_num, data_len);

//     if (received_header->seq_num == expected_seq) {
//         // In-order packet processing
//         if (!chat_mode && !(*file_opened)) {
//             // First packet in file mode: Treat payload as filename.
//             char safe_filename[MAX_BUFFER_SIZE + 1];
//             memcpy(safe_filename, data, data_len < MAX_BUFFER_SIZE ? data_len : MAX_BUFFER_SIZE);
//             safe_filename[data_len] = '\0'; // Ensure null termination

//             *output_file_ptr = fopen(safe_filename, "wb");
//             if (*output_file_ptr) {
//                 log_event("CREATED file: %s", safe_filename);
//                 *file_opened = 1;
//             } else {
//                 perror("Failed to create output file from client request");
//                 // Continue without file open, ACKs will still be sent, but data discarded.
//             }
//         } else if (!chat_mode && *output_file_ptr) {
//             // Subsequent packets in file mode: Write data to file.
//             fwrite(data, 1, data_len, *output_file_ptr);
//             fflush(*output_file_ptr);
//         } else if (chat_mode) {
//             // Chat mode: print to screen.
//             printf("\rClient: %.*s\n> ", data_len, data);
//             fflush(stdout);
//         }

//         expected_seq += data_len;

//         // Process buffered packets that are now in sequence
//         process_buffered_packets(*output_file_ptr, chat_mode);

//     } else if (received_header->seq_num > expected_seq) {
//         // Out-of-order packet: buffer it if space available.
//         buffer_packet(received_header, data, data_len);
//     } else {
//         // Duplicate packet (seq_num < expected_seq), ignore data but resend ACK.
//         log_event("RCV DUP DATA SEQ=%u", received_header->seq_num);
//     }

//     // Send cumulative ACK for the highest in-order sequence number received so far.
//     struct sham_header ack_response;
//     ack_response.seq_num = server_seq;
//     ack_response.ack_num = expected_seq;
//     ack_response.flags = ACK_FLAG;
//     ack_response.window_size = receiver_window;

//     send_packet_server(sockfd, client_addr, &ack_response, NULL, 0);
//     log_event("SND ACK=%u WIN=%u", ack_response.ack_num, ack_response.window_size);
// }

// void handle_fin(int sockfd, struct sockaddr_in *client_addr, struct sham_header *received_header) {
//     log_event("RCV FIN SEQ=%u", received_header->seq_num);

//     // Send ACK for client's FIN
//     struct sham_header ack_response;
//     ack_response.seq_num = server_seq;
//     ack_response.ack_num = received_header->seq_num + 1;
//     ack_response.flags = ACK_FLAG;
//     ack_response.window_size = receiver_window;
//     send_packet_server(sockfd, client_addr, &ack_response, NULL, 0);
//     log_event("SND ACK FOR FIN");

//     state = CLOSE_WAIT;

//     // Send server's FIN packet to close connection from this side.
//     struct sham_header fin_response;
//     fin_response.seq_num = server_seq;
//     fin_response.ack_num = 0; // No ACK field in FIN packet
//     fin_response.flags = FIN_FLAG;
//     fin_response.window_size = receiver_window;
//     send_packet_server(sockfd, client_addr, &fin_response, NULL, 0);
//     log_event("SND FIN SEQ=%u", fin_response.seq_num);

//     server_seq++;
//     state = LAST_ACK;
// }

// // --- Main Function ---

// int main(int argc, char *argv[]) {
//     setvbuf(stdout, NULL, _IONBF, 0); // Disable stdout buffering

//     if (argc < 2) {
//         fprintf(stderr, "Usage: %s <port> [--chat] [loss_rate]\n", argv[0]);
//         exit(1);
//     }

//     int port = atoi(argv[1]);
//     for (int i = 2; i < argc; i++) {
//         if (strcmp(argv[i], "--chat") == 0) {
//             chat_mode = 1;
//         } else {
//             loss_rate = atof(argv[i]);
//         }
//     }

//     init_logging("server_log.txt");
//     srand(time(NULL));
//     memset(packet_buffers, 0, sizeof(packet_buffers));

//     int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
//     if (sockfd < 0) {
//         perror("socket creation failed");
//         exit(1);
//     }

//     struct sockaddr_in server_addr, client_addr;
//     socklen_t client_len = sizeof(client_addr);
//     memset(&server_addr, 0, sizeof(server_addr));
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_addr.s_addr = INADDR_ANY;
//     server_addr.sin_port = htons(port);

//     if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
//         perror("bind failed");
//         close(sockfd);
//         exit(1);
//     }

//     printf("Server listening on port %d\n", port);
//     state = LISTEN;

//     char buffer[sizeof(struct sham_header) + MAX_BUFFER_SIZE];
//     FILE *output_file = NULL;
//     int file_opened = 0; // Flag to track if dynamic file has been opened

//     fd_set read_fds;
//     int max_fd = sockfd;

//     while (state != CLOSED) {
//         FD_ZERO(&read_fds);
//         FD_SET(sockfd, &read_fds);
//         max_fd = sockfd;

//         if (chat_mode && state == ESTABLISHED) {
//             FD_SET(STDIN_FILENO, &read_fds);
//             max_fd = (STDIN_FILENO > max_fd) ? STDIN_FILENO : max_fd;
//         }

//         struct timeval timeout;
//         timeout.tv_sec = 1; // Main loop timeout for periodic checks/tasks
//         timeout.tv_usec = 0;

//         int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

//         if (activity < 0) {
//             if (errno == EINTR) continue;
//             perror("select failed");
//             break;
//         }

//         if (activity > 0) {
//             if (FD_ISSET(sockfd, &read_fds)) {
//                 ssize_t bytes_received = recvfrom(sockfd, buffer, sizeof(buffer), 0,
//                                                  (struct sockaddr*)&client_addr, &client_len);

//                 if (bytes_received >= (ssize_t)sizeof(struct sham_header)) {
//                     struct sham_header received_header = ntoh_header(*(struct sham_header*)buffer);
//                     char *data = buffer + sizeof(struct sham_header);
//                     int data_len = bytes_received - sizeof(struct sham_header);

//                     if (received_header.flags & SYN_FLAG) {
//                         handle_syn(sockfd, &client_addr, &received_header);
//                     } else if (data_len > 0 && state == ESTABLISHED) {
//                         handle_data(sockfd, &client_addr, &received_header, data, data_len, &output_file, &file_opened);
//                     } else if (received_header.flags & ACK_FLAG && data_len == 0) {
//                         if(state == SYN_RECEIVED) handle_ack_for_syn(&received_header);
//                         if(state == LAST_ACK) {
//                             log_event("RCV FINAL ACK. Connection fully closed.");
//                             state = CLOSED;
//                         }
//                     } else if (received_header.flags & FIN_FLAG) {
//                         handle_fin(sockfd, &client_addr, &received_header);
//                     }
//                 }
//             }

//             if (chat_mode && state == ESTABLISHED && FD_ISSET(STDIN_FILENO, &read_fds)) {
//                 char input[MAX_BUFFER_SIZE];
//                 if (fgets(input, sizeof(input), stdin)) {
//                     if (strncmp(input, "/quit", 5) == 0) {
//                         // Server initiated close (via keyboard input) - less common scenario
//                         struct sham_header fin_header;
//                         fin_header.seq_num = server_seq;
//                         fin_header.flags = FIN_FLAG;
//                         fin_header.window_size = receiver_window;
//                         send_packet_server(sockfd, &client_addr, &fin_header, NULL, 0);
//                         log_event("SND FIN SEQ=%u (server initiated)", fin_header.seq_num);
//                         server_seq++;
//                         state = FIN_WAIT_1; // Server enters FIN_WAIT_1 state
//                     } else {
//                         // Send chat message (Note: server send reliability not implemented)
//                         struct sham_header data_header;
//                         data_header.seq_num = server_seq;
//                         data_header.flags = 0;
//                         data_header.window_size = receiver_window;
//                         int input_len = strlen(input);
//                         send_packet_server(sockfd, &client_addr, &data_header, input, input_len);
//                         printf("> ");
//                         log_event("SND DATA SEQ=%u LEN=%d", server_seq, input_len);
//                         server_seq += input_len;
//                     }
//                 }
//             }
//         }
//     }

//     if (output_file) {
//         fclose(output_file);
//         // MD5 calculation logic here assumes filename was stored separately.
//         // For simplicity, we assume we need to re-open based on a known name if we wanted to MD5.
//         // Since we don't store the dynamic filename globally, skipping MD5 for now.
//     }

//     close(sockfd);
//     if (log_file) fclose(log_file);
//     return 0;
// }


// // LLM GENREATED CODE ENDS///



// #include "sham.h"


// // LLM GENERATDE CODE STARTS ////



// // Global variables
// FILE *log_file = NULL;
// int logging_enabled = 0;
// enum connection_state state = CLOSED;
// uint32_t server_seq = 5000;
// uint32_t expected_seq = 0;
// uint32_t receiver_window = RECEIVER_BUFFER_SIZE;
// double loss_rate = 0.0;
// int chat_mode = 0;

// // Packet buffer for out-of-order packets
// struct packet_buffer packet_buffers[MAX_WINDOW_SIZE * 2];

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

// void calculate_md5(const char *filename) {
//     FILE *file = fopen(filename, "rb");
//     if (!file) {
//         perror("Error opening received file for MD5 calculation");
//         return;
//     }

//     MD5_CTX md5_ctx;
//     MD5_Init(&md5_ctx);
//     unsigned char buffer[MAX_BUFFER_SIZE];
//     size_t bytes_read;

//     while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
//         MD5_Update(&md5_ctx, buffer, bytes_read);
//     }

//     unsigned char digest[MD5_DIGEST_LENGTH];
//     MD5_Final(digest, &md5_ctx);
//     fclose(file);

//     printf("MD5: ");
//     for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
//         printf("%02x", digest[i]);
//     }
//     printf("\n");
// }

// void send_packet_server(int sockfd, struct sockaddr_in *client_addr, struct sham_header *header, char *data, int data_len) {
//     char buffer[sizeof(struct sham_header) + MAX_BUFFER_SIZE];
//     struct sham_header net_header = hton_header(*header);
//     memcpy(buffer, &net_header, sizeof(struct sham_header));

//     if (data && data_len > 0) {
//         memcpy(buffer + sizeof(struct sham_header), data, data_len);
//     }

//     sendto(sockfd, buffer, sizeof(struct sham_header) + data_len, 0,
//            (struct sockaddr*)client_addr, sizeof(*client_addr));
// }

// // --- Protocol Logic Handlers ---

// void handle_syn(int sockfd, struct sockaddr_in *client_addr, struct sham_header *received_header) {
//     log_event("RCV SYN SEQ=%u", received_header->seq_num);

//     // Set initial expected sequence number from client's initial sequence number
//     expected_seq = received_header->seq_num + 1;
//     state = SYN_RECEIVED;

//     // Send SYN-ACK
//     struct sham_header response;
//     response.seq_num = server_seq;
//     response.ack_num = expected_seq;
//     response.flags = SYN_FLAG | ACK_FLAG;
//     response.window_size = receiver_window;

//     send_packet_server(sockfd, client_addr, &response, NULL, 0);
//     log_event("SND SYN-ACK SEQ=%u ACK=%u", response.seq_num, response.ack_num);
//     server_seq++; // Increment sequence number after sending SYN-ACK
// }

// void handle_ack_for_syn(struct sham_header *received_header) {
//     if (received_header->ack_num == server_seq && state == SYN_RECEIVED) {
//         log_event("RCV ACK FOR SYN");
//         state = ESTABLISHED;
//     } else {
//         log_event("RCV DUP ACK FOR SYN (ignored)");
//     }
// }

// void buffer_packet(struct sham_header *header, char *data, int data_len) {
//     for (int i = 0; i < MAX_WINDOW_SIZE * 2; i++) {
//         if (!packet_buffers[i].valid) {
//             packet_buffers[i].header = *header;
//             memcpy(packet_buffers[i].data, data, data_len);
//             packet_buffers[i].data_len = data_len;
//             packet_buffers[i].valid = 1;
//             log_event("BUFFERED PKT SEQ=%u", header->seq_num);
//             return;
//         }
//     }
//     log_event("BUFFER FULL: Dropped pkt SEQ=%u", header->seq_num);
// }

// /**
//  * @brief Processes buffered packets in sequence after an in-order packet arrives.
//  * @param output_file File handle to write to in file transfer mode.
//  * @param chat_mode_flag Flag to indicate if we should print to stdout in chat mode.
//  */
// void process_buffered_packets(FILE *output_file, int chat_mode_flag) {
//     int found_in_order_packet = 1;
//     while (found_in_order_packet) {
//         found_in_order_packet = 0;
//         for (int i = 0; i < MAX_WINDOW_SIZE * 2; i++) {
//             if (packet_buffers[i].valid && packet_buffers[i].header.seq_num == expected_seq) {
//                 log_event("PROCESS BUFFERED PKT SEQ=%u", packet_buffers[i].header.seq_num);

//                 if (output_file) {
//                     fwrite(packet_buffers[i].data, 1, packet_buffers[i].data_len, output_file);
//                     fflush(output_file);
//                 } else if (chat_mode_flag) {
//                     printf("\rClient: %.*s\n> ", packet_buffers[i].data_len, packet_buffers[i].data);
//                     fflush(stdout);
//                 }

//                 expected_seq += packet_buffers[i].data_len;
//                 packet_buffers[i].valid = 0;
//                 found_in_order_packet = 1;
//                 break; // Restart search from beginning for next sequence number
//             }
//         }
//     }
// }

// /**
//  * @brief Handles incoming data packets.
//  * Manages dynamic file opening for file transfer mode and data display for chat mode.
//  * Handles out-of-order buffering and sends cumulative ACKs.
//  */
// // *** MODIFIED: Added 'char *filename_buffer' as the last argument
// void handle_data(int sockfd, struct sockaddr_in *client_addr, struct sham_header *received_header,
//                 char *data, int data_len, FILE **output_file_ptr, int *file_opened, char *filename_buffer) {

//     if (should_drop_packet(loss_rate)) {
//         log_event("DROP DATA SEQ=%u", received_header->seq_num);
//         return;
//     }

//     log_event("RCV DATA SEQ=%u LEN=%d", received_header->seq_num, data_len);

//     if (received_header->seq_num == expected_seq) {
//         // In-order packet processing
//         if (!chat_mode && !(*file_opened)) {
//             // *** MODIFIED: Instead of a local variable, store the filename in the buffer from main()
//             strncpy(filename_buffer, data, data_len);
//             filename_buffer[data_len] = '\0'; // Ensure null termination

//             *output_file_ptr = fopen(filename_buffer, "wb");
//             if (*output_file_ptr) {
//                 log_event("CREATED file: %s", filename_buffer);
//                 *file_opened = 1;
//             } else {
//                 perror("Failed to create output file from client request");
//                 // Continue without file open, ACKs will still be sent, but data discarded.
//             }
//         } else if (!chat_mode && *output_file_ptr) {
//             // Subsequent packets in file mode: Write data to file.
//             fwrite(data, 1, data_len, *output_file_ptr);
//             fflush(*output_file_ptr);
//         } else if (chat_mode) {
//             // Chat mode: print to screen.
//             printf("\rClient: %.*s\n> ", data_len, data);
//             fflush(stdout);
//         }

//         expected_seq += data_len;

//         // Process buffered packets that are now in sequence
//         process_buffered_packets(*output_file_ptr, chat_mode);

//     } else if (received_header->seq_num > expected_seq) {
//         // Out-of-order packet: buffer it if space available.
//         buffer_packet(received_header, data, data_len);
//     } else {
//         // Duplicate packet (seq_num < expected_seq), ignore data but resend ACK.
//         log_event("RCV DUP DATA SEQ=%u", received_header->seq_num);
//     }

//     // Send cumulative ACK for the highest in-order sequence number received so far.
//     struct sham_header ack_response;
//     ack_response.seq_num = server_seq;
//     ack_response.ack_num = expected_seq;
//     ack_response.flags = ACK_FLAG;
//     ack_response.window_size = receiver_window;

//     send_packet_server(sockfd, client_addr, &ack_response, NULL, 0);
//     log_event("SND ACK=%u WIN=%u", ack_response.ack_num, ack_response.window_size);
// }

// void handle_fin(int sockfd, struct sockaddr_in *client_addr, struct sham_header *received_header) {
//     log_event("RCV FIN SEQ=%u", received_header->seq_num);

//     // Send ACK for client's FIN
//     struct sham_header ack_response;
//     ack_response.seq_num = server_seq;
//     ack_response.ack_num = received_header->seq_num + 1;
//     ack_response.flags = ACK_FLAG;
//     ack_response.window_size = receiver_window;
//     send_packet_server(sockfd, client_addr, &ack_response, NULL, 0);
//     log_event("SND ACK FOR FIN");

//     state = CLOSE_WAIT;

//     // Send server's FIN packet to close connection from this side.
//     struct sham_header fin_response;
//     fin_response.seq_num = server_seq;
//     fin_response.ack_num = 0; // No ACK field in FIN packet
//     fin_response.flags = FIN_FLAG;
//     fin_response.window_size = receiver_window;
//     send_packet_server(sockfd, client_addr, &fin_response, NULL, 0);
//     log_event("SND FIN SEQ=%u", fin_response.seq_num);

//     server_seq++;
//     state = LAST_ACK;
// }

// // --- Main Function ---

// int main(int argc, char *argv[]) {
//     setvbuf(stdout, NULL, _IONBF, 0); // Disable stdout buffering

//     if (argc < 2) {
//         fprintf(stderr, "Usage: %s <port> [--chat] [loss_rate]\n", argv[0]);
//         exit(1);
//     }

//     int port = atoi(argv[1]);
//     for (int i = 2; i < argc; i++) {
//         if (strcmp(argv[i], "--chat") == 0) {
//             chat_mode = 1;
//         } else {
//             loss_rate = atof(argv[i]);
//         }
//     }

//     init_logging("server_log.txt");
//     srand(time(NULL));
//     memset(packet_buffers, 0, sizeof(packet_buffers));

//     int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
//     if (sockfd < 0) {
//         perror("socket creation failed");
//         exit(1);
//     }

//     struct sockaddr_in server_addr, client_addr;
//     socklen_t client_len = sizeof(client_addr);
//     memset(&server_addr, 0, sizeof(server_addr));
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_addr.s_addr = INADDR_ANY;
//     server_addr.sin_port = htons(port);

//     if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
//         perror("bind failed");
//         close(sockfd);
//         exit(1);
//     }

//     printf("Server listening on port %d\n", port);
//     state = LISTEN;

//     char buffer[sizeof(struct sham_header) + MAX_BUFFER_SIZE];
//     FILE *output_file = NULL;
//     int file_opened = 0;
//     // *** ADDED: Buffer to store the filename received from the client
//     char received_filename[MAX_BUFFER_SIZE + 1] = {0};

//     fd_set read_fds;
//     int max_fd = sockfd;

//     while (state != CLOSED) {
//         FD_ZERO(&read_fds);
//         FD_SET(sockfd, &read_fds);
//         max_fd = sockfd;

//         if (chat_mode && state == ESTABLISHED) {
//             FD_SET(STDIN_FILENO, &read_fds);
//             max_fd = (STDIN_FILENO > max_fd) ? STDIN_FILENO : max_fd;
//         }

//         struct timeval timeout;
//         timeout.tv_sec = 1;
//         timeout.tv_usec = 0;

//         int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

//         if (activity < 0) {
//             if (errno == EINTR) continue;
//             perror("select failed");
//             break;
//         }

//         if (activity > 0) {
//             if (FD_ISSET(sockfd, &read_fds)) {
//                 ssize_t bytes_received = recvfrom(sockfd, buffer, sizeof(buffer), 0,
//                                                  (struct sockaddr*)&client_addr, &client_len);

//                 if (bytes_received >= (ssize_t)sizeof(struct sham_header)) {
//                     struct sham_header received_header = ntoh_header(*(struct sham_header*)buffer);
//                     char *data = buffer + sizeof(struct sham_header);
//                     int data_len = bytes_received - sizeof(struct sham_header);

//                     if (received_header.flags & SYN_FLAG) {
//                         handle_syn(sockfd, &client_addr, &received_header);
//                     } else if (data_len > 0 && state == ESTABLISHED) {
//                         // *** MODIFIED: Pass received_filename buffer to the handler
//                         handle_data(sockfd, &client_addr, &received_header, data, data_len, &output_file, &file_opened, received_filename);
//                     } else if (received_header.flags & ACK_FLAG && data_len == 0) {
//                         if(state == SYN_RECEIVED) handle_ack_for_syn(&received_header);
//                         if(state == LAST_ACK) {
//                             log_event("RCV FINAL ACK. Connection fully closed.");
//                             state = CLOSED;
//                         }
//                     } else if (received_header.flags & FIN_FLAG) {
//                         handle_fin(sockfd, &client_addr, &received_header);
//                     }
//                 }
//             }

//             if (chat_mode && state == ESTABLISHED && FD_ISSET(STDIN_FILENO, &read_fds)) {
//                 char input[MAX_BUFFER_SIZE];
//                 if (fgets(input, sizeof(input), stdin)) {
//                     if (strncmp(input, "/quit", 5) == 0) {
//                         // Server initiated close (via keyboard input) - less common scenario
//                         struct sham_header fin_header;
//                         fin_header.seq_num = server_seq;
//                         fin_header.flags = FIN_FLAG;
//                         fin_header.window_size = receiver_window;
//                         send_packet_server(sockfd, &client_addr, &fin_header, NULL, 0);
//                         log_event("SND FIN SEQ=%u (server initiated)", fin_header.seq_num);
//                         server_seq++;
//                         state = FIN_WAIT_1; // Server enters FIN_WAIT_1 state
//                     } else {
//                         // Send chat message (Note: server send reliability not implemented)
//                         struct sham_header data_header;
//                         data_header.seq_num = server_seq;
//                         data_header.flags = 0;
//                         data_header.window_size = receiver_window;
//                         int input_len = strlen(input);
//                         send_packet_server(sockfd, &client_addr, &data_header, input, input_len);
//                         printf("> ");
//                         log_event("SND DATA SEQ=%u LEN=%d", server_seq, input_len);
//                         server_seq += input_len;
//                     }
//                 }
//             }
//         }
//     }

//     if (output_file) {
//         fclose(output_file);
//         // *** MODIFIED: Check if a file was actually created and then calculate its MD5 sum
//         if (file_opened && !chat_mode) {
//             printf("File transfer complete.\n");
//             calculate_md5(received_filename);
//         }
//     }

//     close(sockfd);
//     if (log_file) fclose(log_file);
//     return 0;
// }


// // LLM GENREATED CODE ENDS///


#include "sham.h"

// Global variables
FILE *log_file = NULL;
int logging_enabled = 0;
enum connection_state state = CLOSED;
uint32_t server_seq = 5000;
uint32_t expected_seq = 0;
uint32_t receiver_window = RECEIVER_BUFFER_SIZE;
double loss_rate = 0.0;
int chat_mode = 0;

// Packet buffer for out-of-order packets
struct packet_buffer packet_buffers[MAX_WINDOW_SIZE * 2];

// --- Logging and Utilities ---

void init_logging(const char *filename) {
    char *log_env = getenv("RUDP_LOG");
    if (log_env && strcmp(log_env, "1") == 0) {
        logging_enabled = 1;
        log_file = fopen(filename, "w");
        if (!log_file) {
            perror("Failed to create log file");
        }
    }
}

void log_event(const char *format, ...) {
    if (!logging_enabled || !log_file) return;

    char time_buffer[30];
    struct timeval tv;
    time_t curtime;

    gettimeofday(&tv, NULL);
    curtime = tv.tv_sec;

    strftime(time_buffer, 30, "%Y-%m-%d %H:%M:%S", localtime(&curtime));
    fprintf(log_file, "[%s.%06ld] [LOG] ", time_buffer, (long)tv.tv_usec);

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);

    fprintf(log_file, "\n");
    fflush(log_file);
}

int should_drop_packet(double rate) {
    if (rate > 0.0) {
        return ((double)rand() / RAND_MAX) < rate;
    }
    return 0;
}

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

void calculate_md5(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening received file for MD5 calculation");
        return;
    }

    MD5_CTX md5_ctx;
    MD5_Init(&md5_ctx);
    unsigned char buffer[MAX_BUFFER_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        MD5_Update(&md5_ctx, buffer, bytes_read);
    }

    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5_Final(digest, &md5_ctx);
    fclose(file);

    printf("MD5: ");
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        printf("%02x", digest[i]);
    }
    printf("\n");
}

void send_packet_server(int sockfd, struct sockaddr_in *client_addr, struct sham_header *header, char *data, int data_len) {
    char buffer[sizeof(struct sham_header) + MAX_BUFFER_SIZE];
    struct sham_header net_header = hton_header(*header);
    memcpy(buffer, &net_header, sizeof(struct sham_header));

    if (data && data_len > 0) {
        memcpy(buffer + sizeof(struct sham_header), data, data_len);
    }

    sendto(sockfd, buffer, sizeof(struct sham_header) + data_len, 0,
           (struct sockaddr*)client_addr, sizeof(*client_addr));
}

// --- Protocol Logic Handlers ---

void handle_syn(int sockfd, struct sockaddr_in *client_addr, struct sham_header *received_header) {
    log_event("RCV SYN SEQ=%u", received_header->seq_num);
    expected_seq = received_header->seq_num + 1;
    state = SYN_RECEIVED;

    struct sham_header response;
    response.seq_num = server_seq;
    response.ack_num = expected_seq;
    response.flags = SYN_FLAG | ACK_FLAG;
    response.window_size = receiver_window;

    send_packet_server(sockfd, client_addr, &response, NULL, 0);
    log_event("SND SYN-ACK SEQ=%u ACK=%u", response.seq_num, response.ack_num);
    server_seq++;
}

void handle_ack_for_syn(struct sham_header *received_header) {
    if (received_header->ack_num == server_seq && state == SYN_RECEIVED) {
        log_event("RCV ACK FOR SYN");
        state = ESTABLISHED;
    } else {
        log_event("RCV DUP ACK FOR SYN (ignored)");
    }
}

void buffer_packet(struct sham_header *header, char *data, int data_len) {
    for (int i = 0; i < MAX_WINDOW_SIZE * 2; i++) {
        if (!packet_buffers[i].valid) {
            packet_buffers[i].header = *header;
            memcpy(packet_buffers[i].data, data, data_len);
            packet_buffers[i].data_len = data_len;
            packet_buffers[i].valid = 1;
            log_event("BUFFERED PKT SEQ=%u", header->seq_num);
            return;
        }
    }
    log_event("BUFFER FULL: Dropped pkt SEQ=%u", header->seq_num);
}

void process_buffered_packets(FILE *output_file) {
    int processed_one = 1;
    while (processed_one) {
        processed_one = 0;
        for (int i = 0; i < MAX_WINDOW_SIZE * 2; i++) {
            if (packet_buffers[i].valid && packet_buffers[i].header.seq_num == expected_seq) {
                log_event("PROCESS BUFFERED PKT SEQ=%u", packet_buffers[i].header.seq_num);

                if (chat_mode) {
                    printf("\rClient: %.*s\n> ", packet_buffers[i].data_len, packet_buffers[i].data);
                    fflush(stdout);
                } else if (output_file) {
                    fwrite(packet_buffers[i].data, 1, packet_buffers[i].data_len, output_file);
                }

                expected_seq += packet_buffers[i].data_len;
                packet_buffers[i].valid = 0;
                processed_one = 1;
                break;
            }
        }
    }
}

void handle_data(int sockfd, struct sockaddr_in *client_addr, struct sham_header *received_header,
                char *data, int data_len, FILE **output_file_ptr, int *file_opened, char *filename_buffer) {

    if (should_drop_packet(loss_rate)) {
        log_event("DROP DATA SEQ=%u", received_header->seq_num);
        return;
    }

    log_event("RCV DATA SEQ=%u LEN=%d", received_header->seq_num, data_len);

    if (received_header->seq_num >= expected_seq) {
        if (received_header->seq_num == expected_seq) {
            if (!chat_mode && !(*file_opened)) {
                strncpy(filename_buffer, data, data_len);
                filename_buffer[data_len] = '\0';
                *output_file_ptr = fopen(filename_buffer, "wb");
                if (*output_file_ptr) {
                    log_event("CREATED file: %s", filename_buffer);
                    *file_opened = 1;
                } else {
                    perror("Failed to create output file");
                }
            } else if (chat_mode) {
                printf("\rClient: %.*s\n> ", data_len, data);
                fflush(stdout);
            } else if (*output_file_ptr) {
                fwrite(data, 1, data_len, *output_file_ptr);
            }
            expected_seq += data_len;
            process_buffered_packets(*output_file_ptr);
        } else {
            buffer_packet(received_header, data, data_len);
        }
    }

    struct sham_header ack_response;
    ack_response.seq_num = server_seq;
    ack_response.ack_num = expected_seq;
    ack_response.flags = ACK_FLAG;
    ack_response.window_size = receiver_window;
    send_packet_server(sockfd, client_addr, &ack_response, NULL, 0);
    log_event("SND ACK=%u WIN=%u", ack_response.ack_num, ack_response.window_size);
}


void handle_fin(int sockfd, struct sockaddr_in *client_addr, struct sham_header *received_header) {
    log_event("RCV FIN SEQ=%u", received_header->seq_num);

    struct sham_header ack_response;
    ack_response.seq_num = server_seq;
    ack_response.ack_num = received_header->seq_num + 1;
    ack_response.flags = ACK_FLAG;
    ack_response.window_size = receiver_window;
    send_packet_server(sockfd, client_addr, &ack_response, NULL, 0);
    log_event("SND ACK FOR FIN");
    state = CLOSE_WAIT;

    struct sham_header fin_response;
    fin_response.seq_num = server_seq;
    fin_response.ack_num = 0;
    fin_response.flags = FIN_FLAG;
    fin_response.window_size = receiver_window;
    send_packet_server(sockfd, client_addr, &fin_response, NULL, 0);
    log_event("SND FIN SEQ=%u", fin_response.seq_num);
    server_seq++;
    state = LAST_ACK;
}

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port> [--chat] [loss_rate]\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--chat") == 0) chat_mode = 1;
        else loss_rate = atof(argv[i]);
    }

    init_logging("server_log.txt");
    srand(time(NULL));
    memset(packet_buffers, 0, sizeof(packet_buffers));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(1);
    }

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(1);
    }

    printf("Server listening on port %d\n", port);
    state = LISTEN;

    char buffer[sizeof(struct sham_header) + MAX_BUFFER_SIZE];
    FILE *output_file = NULL;
    int file_opened = 0;
    char received_filename[MAX_BUFFER_SIZE + 1] = {0};

    fd_set read_fds;
    int max_fd;

    while (state != CLOSED) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        max_fd = sockfd;

        if (chat_mode && state == ESTABLISHED) {
            FD_SET(STDIN_FILENO, &read_fds);
            if (STDIN_FILENO > max_fd) max_fd = STDIN_FILENO;
        }

        struct timeval timeout = {1, 0};
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

        if (activity < 0 && errno != EINTR) {
            perror("select failed");
            break;
        }

        if (activity > 0) {
            if (FD_ISSET(sockfd, &read_fds)) {
                ssize_t bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_len);
                if (bytes >= (ssize_t)sizeof(struct sham_header)) {
                    struct sham_header header = ntoh_header(*(struct sham_header*)buffer);
                    char *data = buffer + sizeof(struct sham_header);
                    int data_len = bytes - sizeof(struct sham_header);

                    if (header.flags & SYN_FLAG) handle_syn(sockfd, &client_addr, &header);
                    else if (header.flags & ACK_FLAG && data_len == 0) {
                        if (state == SYN_RECEIVED) handle_ack_for_syn(&header);
                        if (state == LAST_ACK) {
                            log_event("RCV FINAL ACK. Connection fully closed.");
                            state = CLOSED;
                        }
                    } else if (header.flags & FIN_FLAG) handle_fin(sockfd, &client_addr, &header);
                    else if (state == ESTABLISHED) handle_data(sockfd, &client_addr, &header, data, data_len, &output_file, &file_opened, received_filename);
                }
            }

            if (chat_mode && state == ESTABLISHED && FD_ISSET(STDIN_FILENO, &read_fds)) {
                char input[MAX_BUFFER_SIZE];
                if (fgets(input, sizeof(input), stdin)) {
                    if (strncmp(input, "/quit", 5) == 0) {
                        struct sham_header fin_header = { .seq_num = server_seq, .flags = FIN_FLAG, .window_size = receiver_window };
                        send_packet_server(sockfd, &client_addr, &fin_header, NULL, 0);
                        log_event("SND FIN SEQ=%u (server initiated)", fin_header.seq_num);
                        server_seq++;
                        state = FIN_WAIT_1;
                    } else {
                        struct sham_header data_header = { .seq_num = server_seq, .window_size = receiver_window };
                        int input_len = strlen(input);
                        send_packet_server(sockfd, &client_addr, &data_header, input, input_len);
                        log_event("SND DATA SEQ=%u LEN=%d", server_seq, input_len);
                        server_seq += input_len;
                        printf("> ");
                    }
                }
            }
        }
    }

    if (output_file) {
        fclose(output_file);
        if (file_opened && !chat_mode) {
            printf("File transfer complete.\n");
            calculate_md5(received_filename);
        }
    }

    close(sockfd);
    if (log_file) fclose(log_file);
    return 0;
}