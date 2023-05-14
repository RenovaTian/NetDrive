#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/md5.h>

#define SERVER_ADDR "127.0.0.1"     //Ԫ���ݷ�������ַ�������д��)
#define PORT 8080

#define BLOCK_SIZE 4194304          //�ļ����С

int file_list() {                   //�г��ļ�Ŀ¼
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // �����׽���
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;     //����IPv4
    serv_addr.sin_port = htons(PORT);

    // ��IP��ַת��Ϊ�����ֽ���
    if(inet_pton(AF_INET, SERVER_ADDR, &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // ���ӵ�������
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // ����������������Ի�ȡ�ļ��б�
    strcpy(buffer, "LIST");
    send(sock, buffer, strlen(buffer), 0);

    // ���׽����ж�ȡ�ļ��б�
    valread = read(sock, buffer, 1024);
    printf("%s\n", buffer);                 //������ʱ�ô�ӡbuffer������ʾ�ļ�Ŀ¼

    // �ر��׽���
    close(sock);

    return 0;
}

int file_upload(const char* filepath){      //�ļ��ϴ�����

    int sock = 0, valread;                  //��ʼ���׽���������
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    FILE* fp = fopen(filepath,"r");         //���ļ�

    // �����׽���
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // ���ӷ�����
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    int file_size = get_filesize(filepath);                 //����ļ��Ĵ�С��Ԫ���ݷ�����
    long num_network = htonl(file_size);                    //ת��Ϊ�����ֽ��򣨴����

    strcpy(buffer,"upload");                                //������������ϴ���Ϣ
    send(sock, buffer, strlen(buffer), 0);
    send(sock, &num_network, sizeof(num_network), 0);       //������������ļ���С��Ϣ

    valread = read(sock, buffer, 1024);                     //�յ�Ԫ���ݷ����������Ŀ��÷�������ַ

    {
        //�����յ��ķ�������ַ���ӷ�����
    }

    int num_blocks = (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;     //�������
    int last_block_size = file_size % BLOCK_SIZE;                   //���һ�������
    if (last_block_size == 0) {
        last_block_size = BLOCK_SIZE;
    }

    //���Ϳ��������һ��Ĵ�С
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

    for (int i = 0; i < num_blocks; i++) {              //��������ļ��飬�������ϣֵ
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

        // ���Ϳ����ݵ�������
        if (send(sock, block_data, block_size, 0) == -1) {
            perror("send");
            return -1;
        }

         
        MD5_Update(&md5_ctx, block_data, block_size);

        free(block_data);
    }
    // �����ϣֵ
    unsigned char md5_value[MD5_DIGEST_LENGTH];
    MD5_Final(md5_value, &md5_ctx);

    // ���͹�ϣֵ��������
    if (send(sock, md5_value, sizeof(md5_value), 0) == -1) {
        perror("send");
        return -1;
    }

    {
        //����Ԫ���ݷ���������Ϣ�������ʾ�ļ���������·���
    }

    {
        //����ɹ�����Ԫ���ݷ�����send��Ϣ�Ը���Ԫ������Ϣ
    }
    
    close(sock);
    fclose(fp);
    return 0;
}

int file_download(){                                    //�ļ�����
    int sock = 0, valread;  
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    const char* filename; // Ҫ���ص��ļ���

    // �����׽���
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return -1;
    }

    // ����Ԫ���ݷ�����������
    struct sockaddr_in server_sockaddr;
    memset(&server_sockaddr, 0, sizeof(server_sockaddr));
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr.s_addr = inet_addr(server_addr);
    server_sockaddr.sin_port = htons(server_port);
    if (connect(sockfd, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr)) == -1) {
        perror("connect");
        return -1;
    }

    strcpy(buffer,"download");                          //��Ԫ���ݷ��������������ļ�ָ��
    if (send(sockfd, buffer, strlen(filename), 0) == -1) {
        perror("send");
        return -1;
    }

    printf("��ָ�������ļ�����:");
    sacnf(%s,&filename);
    // �����ļ�����������
    if (send(sockfd, filename, strlen(filename), 0) == -1) {
        perror("send");
        return -1;
    }

    if (recv(sockfd, buffer, sizeof(buffer), 0) == -1) {        //���տ������ص��ķ�������ַ
        perror("recv");
        return -1;
    }

    {
        //���ݽ��յ��ĵ�ַ���Ӷ�Ӧ�����������Ӳ��Ͼͻ�
    }

    // �����ļ���С�Ϳ���
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

    // ���ļ�����д��
    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }

    MD5_CTX ctx;
    MD5_Init(&ctx);

    // �����ļ��鲢д���ļ��ͼ���MD5
    for (int i = 0; i < num_blocks; i++) {
        long offset = i * BLOCK_SIZE;
        int block_size = (i == num_blocks - 1) ? last_block_size : BLOCK_SIZE;

        char* block_data = (char*)malloc(block_size);
        if (block_data == NULL) {
            perror("malloc");
            return -1;
        }

        // ���տ�����
        if (recv(sockfd, block_data, block_size, 0) == -1) {
            perror("recv");
            return -1;
        }

        // ��������д���ļ�
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

    // �� MD5 ֵת��Ϊ�ַ�����ʽ
    char md5_str[MD5_DIGEST_LENGTH * 2 + 1];

    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(md5_str + i * 2, "%02x", md5[i]);
    }
    md5_str[MD5_DIGEST_LENGTH * 2] = '\0';

    // �Ƚ� MD5 ֵ
    char server_md5_str[MD5_DIGEST_LENGTH * 2 + 1];
    if (recv(sockfd, server_md5_str, MD5_DIGEST_LENGTH * 2 + 1, 0) == -1) {
        perror("recv");
        return -1;
    }
    if (strcmp(md5_str, server_md5_str) != 0) {
        printf("�ļ�����ʧ�ܣ�MD5 ֵ��ƥ�䣡\n");
        // �ر��׽��ֺ��ļ���ɾ��ʧ�ܵ��ļ�
        remove(filename)
        close(sockfd);
        fclose(fp);
        return -1;
    }2

    // �ر��׽��ֺ��ļ�
    close(sockfd);
    fclose(fp);

    printf("�ļ�������ɣ�\n");

    return 0;
}



long get_filesize(const char* filepath){
    struct stat file_stat;
    if (stat(filepath, &file_stat) == -1) { // ��ȡ�ļ���Ϣ
        perror("stat");
        return -1;
    }
    if (S_ISREG(file_stat.st_mode)) { // �������ͨ�ļ�
        long file_size;
        FILE* fp = fopen(filepath, "r");
        if (fp == NULL) { // ���ļ�ʧ��
            perror("fopen");
            return -1;
        }
        fseek(fp, 0, SEEK_END);
        file_size = ftell(fp); // ��ȡ�ļ���С
        fclose(fp);
        return file_size
    }
    return -1;
}


#endif