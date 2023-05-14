#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "login_module.h"
#include "request_validator.h"
#include "file_manager.h"

int access_level = 0;

// 内部指令类型定义
typedef enum {
    CMD_INVALID, // 无效指令
    CMD_LIST,    // 列出文件列表指令
    CMD_UPLOAD,  // 上传文件指令
    CMD_DOWNLOAD // 下载文件指令
    CMD_HELP,    // 帮助指令
    CMD_LOGIN,   // 登陆指令
    CMD_LOGOUT,  // 注销指令
} command_t;

// 解析用户输入的命令，并返回对应的内部指令类型
command_t parse_command(const char* cmd){
     if (strcmp(cmd, "help") == 0) {
        return CMD_HELP;
    } else if (strcmp(cmd, "login") == 0) {
        return CMD_LOGIN;
    } else if (strcmp(cmd, "logout") == 0) {
        return CMD_LOGOUT;
    } else if (strcmp(cmd, "list") == 0) {
        return CMD_LOGIN;
    } else if (strcmp(cmd, "upload") == 0) {
        return CMD_UPLOAD;
    } else if (strcmp(cmd, "download") == 0) {
        return CMD_DOWNLOAD;
    } else{
        return CMD_INVALID;
    }
};
//获得内部指令类型后转向相关操作；
void process_command(command_t cmd) {
    switch (cmd) {
        case CMD_HELP:
            printf("This is the instructions for use：\n");
            printf("-login: 登陆.\n");
            printf("-logout: 注销.\n");
            printf("-list: 列出文件列表.\n");
            printf("-upload: 上传文件.\n");
            printf("-download: 下载文件.\n");
            break;
        case CMD_LOGIN:
            access_level = usr_login();
            break;
        case CMD_LOGOUT:

            printf("This is the logout command.\n");
            exit(0);                                                    //注销直接将程序退出
            break;
        case CMD_LIST:
            if(request_validator(access_level, (int)CMD_LIST) < 0){     //权限验证，输入为权限等级和操作等级
                break;                                                  //无权
            };

            file_list();                                                //列出目录
            break;
        case CMD_UPLOAD:
            char filepath[256];                                         // 临时变量，用于保存文件路径

            if(request_validator(access_level, (int)CMD_UPLOAD) < 0){   //权限验证，输入为权限等级和操作等级
                break;
            };    

            printf("请输入文件路径：");
            scanf("%s", filepath);
            file_upload(filepath);                                      //上传文件

            break;
        case CMD_DOWNLOAD:
            printf("This is the download command.\n");

            if(request_validator(access_level, (int)CMD_DOWNLOAD) < 0){ //权限验证，输入为权限等级和操作等级
                break;
            }; 

            file_download();                                            //下载文件

            break;
        default:
            printf("Unknown command. 输入help获得帮助\n");
            break;
    }
}

#endif /* COMMAND_PARSER_H */
