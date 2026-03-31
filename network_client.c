/*
 * 纯C语言实现的网络连接示例
 * 功能：TCP客户端，连接到指定服务器并发送/接收数据
 * 
 * 编译方法:
 *   gcc -o network_client network_client.c
 * 
 * 使用方法:
 *   ./network_client <服务器IP> <端口号>
 *   例如: ./network_client 127.0.0.1 8080
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 4096
#define MAX_HOSTNAME 256

/**
 * 创建并连接到服务器
 * @param host 服务器主机名或IP地址
 * @param port 服务器端口号
 * @return 成功返回socket文件描述符，失败返回-1
 */
int connect_to_server(const char *host, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *he;
    
    // 创建socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket创建失败");
        return -1;
    }
    
    // 解析主机名
    he = gethostbyname(host);
    if (he == NULL) {
        fprintf(stderr, "无法解析主机: %s\n", host);
        close(sockfd);
        return -1;
    }
    
    // 设置服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr, he->h_addr_list[0], he->h_length);
    
    // 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("连接失败");
        close(sockfd);
        return -1;
    }
    
    printf("成功连接到服务器 %s:%d\n", host, port);
    return sockfd;
}

/**
 * 发送数据到服务器
 * @param sockfd socket文件描述符
 * @param data 要发送的数据
 * @return 成功返回发送的字节数，失败返回-1
 */
int send_data(int sockfd, const char *data) {
    int bytes_sent;
    
    bytes_sent = send(sockfd, data, strlen(data), 0);
    if (bytes_sent == -1) {
        perror("发送失败");
        return -1;
    }
    
    printf("已发送 %d 字节\n", bytes_sent);
    return bytes_sent;
}

/**
 * 从服务器接收数据
 * @param sockfd socket文件描述符
 * @param buffer 接收缓冲区
 * @param buffer_size 缓冲区大小
 * @return 成功返回接收的字节数，失败返回-1
 */
int receive_data(int sockfd, char *buffer, int buffer_size) {
    int bytes_received;
    
    memset(buffer, 0, buffer_size);
    bytes_received = recv(sockfd, buffer, buffer_size - 1, 0);
    
    if (bytes_received == -1) {
        perror("接收失败");
        return -1;
    } else if (bytes_received == 0) {
        printf("服务器已断开连接\n");
        return 0;
    }
    
    buffer[bytes_received] = '\0';
    printf("收到 %d 字节: %s\n", bytes_received, buffer);
    return bytes_received;
}

/**
 * 主函数 - 演示网络连接功能
 */
int main(int argc, char *argv[]) {
    int sockfd;
    char buffer[BUFFER_SIZE];
    const char *message = "Hello, Server!";
    
    // 检查命令行参数
    if (argc != 3) {
        fprintf(stderr, "用法: %s <服务器IP> <端口号>\n", argv[0]);
        fprintf(stderr, "示例: %s 127.0.0.1 8080\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "端口号必须在 1-65535 之间\n");
        exit(EXIT_FAILURE);
    }
    
    printf("正在连接到 %s:%d ...\n", server_ip, port);
    
    // 连接到服务器
    sockfd = connect_to_server(server_ip, port);
    if (sockfd == -1) {
        exit(EXIT_FAILURE);
    }
    
    // 发送数据
    if (send_data(sockfd, message) == -1) {
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    // 接收响应
    if (receive_data(sockfd, buffer, BUFFER_SIZE) <= 0) {
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    // 关闭连接
    printf("关闭连接...\n");
    close(sockfd);
    
    printf("操作完成!\n");
    return 0;
}
