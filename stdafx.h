
#pragma once
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1  
#define _CRT_SECURE_NO_WARNINGS 1  

#include "targetver.h"

#define PERROR(...) printf( "[ERROR] " );printf( __VA_ARGS__ );printf("\n" );_gettch();
#define PINFO(...) printf(  __VA_ARGS__);printf( "\n" );


#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件: 
#include <windows.h>


// TODO: 在此处引用程序需要的其他头文件
#pragma comment(lib, "ws2_32.lib") 
#include <winsock2.h>
#include <stdio.h>
#include <tchar.h>
#include <thread>
#include <time.h>
