//
//  main.cpp
//  scanner
//
//  Created by 贺星宇 on 2018/12/10.
//  Copyright © 2018年 贺星宇. All rights reserved.
//

#include <iostream>
using namespace std;
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/fcntl.h>


//对某一ip进行端口扫描
void scanPort(char *);


int main(int argc, const char * argv[]) {
    
    char ipadd[40] = {"192.168.54.128"};
    
//    下列功能是由ip 显示主机信息，本地要有反向解析的服务，或在/etc/hosts中手动添加对应信息
//    hostent *myh;
//    in_addr inad ;
//    inad.s_addr =  inet_addr(ipadd);
//    myh = gethostbyaddr(&inad, 4, AF_INET);
//    cout << myh->h_name<<endl;
    
    scanPort(ipadd);
    return 0;
}


void scanPort(char * destIp){
    //目标地址
    sockaddr_in dest;
    
    
    //对端口依次进行扫描
    for(int i = 1;i < 1024;i++){
        
       
        
        //初始化地址
        memset(&dest,0,sizeof(dest));
        dest.sin_family = AF_INET;
        dest.sin_addr.s_addr = inet_addr(destIp);
        
        //创建socket
        int sockfd = socket(AF_INET,SOCK_STREAM,0);
        if(sockfd < 0)
        {
            cout << "socket error\n";
            return ;
        }
        //设置目标端口
        dest.sin_port = htons(i);
        
        //将socket设置为非阻塞模式
        int flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags|O_NONBLOCK);
        
        
        int rs = connect(sockfd, (sockaddr*)&dest, sizeof(dest));
        //最好的情况，链接直接成功
        if(rs == 0){
            cout << destIp << " : "<< i<<endl;
        }
        //连接没有立即返回，此时errno若是EINPROGRESS，我们就要调用select继续分析
        else if(errno == EINPROGRESS){
            fd_set rset,wset;
            FD_ZERO(&rset);
            FD_SET(sockfd,&rset);
            wset = rset;
            
            //设置时间限制
            timeval tval;
            tval.tv_sec = 0;
            tval.tv_usec = 5000;
            
            int srs = select(sockfd + 1, &rset, &wset, NULL, &tval);
            
            //此时说明超时
            if(srs == 0)
            {
                errno = ETIMEDOUT;
            }
            //若描述符变为可读或可写
            else if(FD_ISSET(sockfd,&rset) || FD_ISSET(sockfd, &wset)){
                int errorflag = -1;
                int len = sizeof(errorflag);
                getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (void *)&errorflag, (socklen_t*)&len);
                //连接正常
                if(errorflag == 0){
                    cout << destIp << " : "<< i<<endl;
                }
            }
        }
        
        //接下来对情况进行判断
        //fd_set
        
        close(sockfd);
        
    }
}


