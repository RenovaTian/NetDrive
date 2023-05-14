#ifndef LOGIN_MODULE_H
#define LOGIN_MODULE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_ADDR "127.0.0.1" //��������ַ
#define PORT 8080               //�˿ں�

int usr_login(void){                //��login��������-1��ʾ�������ش��ڵ���0������ʾȨ�޵ȼ�
    int sock = 0, valread;          //�׽���������
    struct sockaddr_in serv_addr;   //��������ַ��Ϣ
    char buffer[1024] = {0};        //
    char name_buffer[1024] = {0};   //��ʱ����˴ε�½�ߵ�id�����ڲ�ѯȨ�޵ȼ�
    int access_level = 0;           //�洢�û�Ȩ�޵ȼ�����ʱ����

    // �����׽���
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); //��������ַ�ڴ���Ϣ����

    serv_addr.sin_family = AF_INET;     //���õ�ַ��ΪIPv4
    serv_addr.sin_port = htons(PORT);   //���ö˿ں�

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

    while(1){
        // �ӱ�׼�����ȡ�û���������
        printf("Enter username: ");
        scanf("%s", buffer);
        strcpy(name_buffer,buffer);     //��ʱ����˴ε�½�û���
        send(sock, buffer, strlen(buffer), 0);

        printf("Enter password: ");
        scanf("%s", buffer);
        send(sock, buffer, strlen(buffer), 0);

        // ���׽��ֶ�ȡ���������͵���֤���
        valread = read(sock, buffer, 1024);
        //printf("%s\n", buffer);

        if(strcmp(buffer,"incorrect") == 0){    //�˺����벻��ȷ
            printf("�˺Ż������벻��ȷ�����������롣");
            continue;
        }

        break;                  //�˺�������ȷ���˳�ѭ��
    }

    strcpy(buffer,"get access_level");          //����������Ͳ�ѯȨ����Ϣ������
    send(sock, buffer, strlen(buffer), 0);
    valread = read(sock, buffer, 1024);
    access_level = buffer[0];                   //����Ԫ���ݷ�������������һ��һλʮ������

    close(sock);
    return access_level;        //����Ȩ�޵ȼ�
}

#endif