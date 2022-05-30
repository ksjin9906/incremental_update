#pragma once
#include<iostream>
#include<string.h>
#include<cstring>
#include"func.h"
#pragma warning(disable:4996)
using namespace std;
void get_file_info(string file_name,string filenames[], int &num_clocks, int &num_blocks,int cycles[],int start[],string mod[], int& max_cycle, int& max_start);