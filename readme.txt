使用到的標頭檔
#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<sstream>
#include<fstream>
#include<vector>
#include<cstring>
#include<ctime>
#include<iomanip>  //設定小數點後4位輸出
#include<pthread.h>  //multithread
#include<mutex>  //multithread對於同時修改的記憶體空間的保護

g++ -pthread -std=c++11 -o main main.cpp