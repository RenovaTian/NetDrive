#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/md5.h>

#define SERVER_ADDR "127.0.0.1"     //元数据服务器地址（先随便写的)
#define PORT 8080

#define BLOCK_SIZE 4194304          //文件块大小

int file_list() {                   //列出文件目录
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // 创建套接字
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;     //设置IPv4
    serv_addr.sin_port = htons(PORT);

    // 将IP地址转换为网络字节序
    if(inet_pton(AF_INET, SERVER_ADDR, &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // 连接到服务器
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // 向服务器发送请求以获取文件列表
    strcpy(buffer, "LIST");
    send(sock, buffer, strlen(buffer), 0);

    // 从套接字中读取文件列表
    valread = read(sock, buffer, 1024);
    printf("%s\n", buffer);                 //这里暂时用打印buffer代替显示文件目录

    // 关闭套接字
    close(sock);

    return 0;
}

int file_upload(const char* filepath){      //文件上传函数

    int sock = 0, valread;                  //初始化套接字描述符
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    FILE* fp = fopen(filepath,"r");         //打开文件

    // 创建套接字
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 连接服务器
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    int file_size = get_filesize(filepath);                 //获得文件的大小给元数据服务器
    long num_network = htonl(file_size);                    //转换为网络字节序（大端序）

    strcpy(buffer,"upload");                                //向服务器发送上传信息
    send(sock, buffer, strlen(buffer), 0);
    send(sock, &num_network, sizeof(num_network), 0);       //向服务器发送文件大小信息

    valread = read(sock, buffer, 1024);                     //收到元数据服务器发来的可用服务器地址

    {
        //根据收到的服务器地址连接服务器
    }

    int num_blocks = (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;     //计算块数
    int last_block_size = file_size % BLOCK_SIZE;                   //最后一块的数量
    if (last_block_size == 0) {
        last_block_size = BLOCK_SIZE;
    }

    //发送块数和最后一块的大小
    if (send(sock, &num_blocks, sizeof(num_blocks), 0) == -1) {          
        perror("send");
        return -1;
    }
    if (send(sock, &last_block_size, sizeof(last_block_size), 0) == -1) {
        perror("send");
        return -1;
    }

    MD5_CTX md5_ctx;
    MD5_Init(&md5_ctx);

    for (int i = 0; i < num_blocks; i++) {              //逐个发送文件块，并计算哈希值
        long offset = i * BLOCK_SIZE;
        int block_size = (i == num_blocks - 1) ? last_block_size : BLOCK_SIZE;

        char* block_data = (char*)malloc(block_size);
        if (block_data == NULL) {
            perror("malloc");
            return -1;
        }

        fseek(fp, offset, SEEK_SET);
        if (fread(block_data, 1, block_size, fp) != block_size) {
            perror("fread");
            return -1;
        }

        // 发送块数据到服务器
        if (send(sock, block_data, block_size, 0) == -1) {
            perror("send");
            return -1;
        }

         
        MD5_Update(&md5_ctx, block_data, block_size);

        free(block_data);
    }
    // 计算哈希值
    unsigned char md5_value[MD5_DIGEST_LENGTH];
    MD5_Final(md5_value, &md5_ctx);

    // 发送哈希值到服务器
    if (send(sock, md5_value, sizeof(md5_value), 0) == -1) {
        perror("send");
        return -1;
    }

    {
        //接收元数据服务器的信息，如果显示文件出错就重新发送
    }

    {
        //如果成功就向元数据服务器send信息以更新元数据信息
    }
    
    close(sock);
    fclose(fp);
    return 0;
}

int file_download(){                                    //文件下载
    int sock = 0, valread;  
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    const char* filename; // 要下载的文件名

    // 创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return -1;
    }

    // 连接元数据服务器服务器
    struct sockaddr_in server_sockaddr;
    memset(&server_sockaddr, 0, sizeof(server_sockaddr));
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr.s_addr = inet_addr(server_addr);
    server_sockaddr.sin_port = htons(server_port);
    if (connect(sockfd, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr)) == -1) {
        perror("connect");
        return -1;
    }

    strcpy(buffer,"download");                          //向元数据服务器发送下载文件指令
    if (send(sockfd, buffer, strlen(filename), 0) == -1) {
        perror("send");
        return -1;
    }

    printf("请指定下载文件名称:");
    sacnf(%s,&filename);
    // 发送文件名到服务器
    if (send(sockfd, filename, strlen(filename), 0) == -1) {
        perror("send");
        return -1;
    }

    if (recv(sockfd, buffer, sizeof(buffer), 0) == -1) {        //接收可以下载到的服务器地址
        perror("recv");
        return -1;
    }

    {
        //根据接收到的地址连接对应服务器，连接不上就换
    }

    // 接收文件大小和块数
    long file_size;
    int num_blocks;
    int last_block_size;
    if (recv(sockfd, &file_size, sizeof(file_size), 0) == -1) {
        perror("recv");
        return -1;
    }
    if (recv(sockfd, &num_blocks, sizeof(num_blocks), 0) == -1) {
        perror("recv");
        return -1;
    }
    if (recv(sockfd, &last_block_size, sizeof(last_block_size), 0) == -1) {
        perror("recv");
        return -1;
    }

    // 打开文件用于写入
    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }

    MD5_CTX ctx;
    MD5_Init(&ctx);

    // 接收文件块并写入文件和计算MD5
    for (int i = 0; i < num_blocks; i++) {
        long offset = i * BLOCK_SIZE;
        int block_size = (i == num_blocks - 1) ? last_block_size : BLOCK_SIZE;

        char* block_data = (char*)malloc(block_size);
        if (block_data == NULL) {
            perror("malloc");
            return -1;
        }

        // 接收块数据
        if (recv(sockfd, block_data, block_size, 0) == -1) {
            perror("recv");
            return -1;
        }

        // 将块数据写入文件
        fseek(fp, offset, SEEK_SET);
        if (fwrite(block_data, 1, block_size, fp) != block_size) {
            perror("fwrite");
            return -1;
        }

        MD5_Update(&ctx, block_data, block_size);           

        free(block_data);
    }

    unsigned char md5[MD5_DIGEST_LENGTH];
    MD5_Final(md5, &ctx);

    // 将 MD5 值转换为字符串形式
    char md5_str[MD5_DIGEST_LENGTH * 2 + 1];

    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(md5_str + i * 2, "%02x", md5[i]);
    }
    md5_str[MD5_DIGEST_LENGTH * 2] = '\0';

    // 比较 MD5 值
    char server_md5_str[MD5_DIGEST_LENGTH * 2 + 1];
    if (recv(sockfd, server_md5_str, MD5_DIGEST_LENGTH * 2 + 1, 0) == -1) {
        perror("recv");
        return -1;
    }
    if (strcmp(md5_str, server_md5_str) != 0) {
        printf("文件下载失败，MD5 值不匹配！\n");
        // 关闭套接字和文件，删除失败的文件
        remove(filename)
        close(sockfd);
        fclose(fp);
        return -1;
    }2

    // 关闭套接字和文件
    close(sockfd);
    fclose(fp);

    printf("文件下载完成！\n");

    return 0;
}



long get_filesize(const char* filepath){
    struct stat file_stat;
    if (stat(filepath, &file_stat) == -1) { // 获取文件信息
        perror("stat");
        return -1;
    }
    if (S_ISREG(file_stat.st_mode)) { // 如果是普通文件
        long file_size;
        FILE* fp = fopen(filepath, "r");
        if (fp == NULL) { // 打开文件失败
            perror("fopen");
            return -1;
        }
        fseek(fp, 0, SEEK_END);
        file_size = ftell(fp); // 获取文件大小
        fclose(fp);
        return file_size
    }
    return -1;
}


#endif