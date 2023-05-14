#ifndef LOGIN_MODULE_H
#define LOGIN_MODULE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_ADDR "127.0.0.1" //服务器地址
#define PORT 8080               //端口号

int usr_login(void){                //在login函数返回-1表示出错，返回大于等于0的数表示权限等级
    int sock = 0, valread;          //套接字描述符
    struct sockaddr_in serv_addr;   //服务器地址信息
    char buffer[1024] = {0};        //
    char name_buffer[1024] = {0};   //临时保存此次登陆者的id，用于查询权限等级
    int access_level = 0;           //存储用户权限等级的临时变量

    // 创建套接字
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); //服务器地址内存信息清零

    serv_addr.sin_family = AF_INET;     //设置地址族为IPv4
    serv_addr.sin_port = htons(PORT);   //设置端口号

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

    while(1){
        // 从标准输入读取用户名和密码
        printf("Enter username: ");
        scanf("%s", buffer);
        strcpy(name_buffer,buffer);     //临时保存此次登陆用户名
        send(sock, buffer, strlen(buffer), 0);

        printf("Enter password: ");
        scanf("%s", buffer);
        send(sock, buffer, strlen(buffer), 0);

        // 从套接字读取服务器发送的验证结果
        valread = read(sock, buffer, 1024);
        //printf("%s\n", buffer);

        if(strcmp(buffer,"incorrect") == 0){    //账号密码不正确
            printf("账号或者密码不正确，请重新输入。");
            continue;
        }

        break;                  //账号密码正确，退出循环
    }

    strcpy(buffer,"get access_level");          //向服务器发送查询权限信息的请求
    send(sock, buffer, strlen(buffer), 0);
    valread = read(sock, buffer, 1024);
    access_level = buffer[0];                   //假设元数据服务器发来的是一个一位十进制数

    close(sock);
    return access_level;        //返回权限等级
}

#endif