#pragma once
#include <string>
using namespace std;
void get_new_ups_table(vector<pair<int, int>> ups_table[], string filenames,int cyc,int clk_start,string file_name, int num_blocks,string mod[],int i, int num_clocks);
//filenames is clk's name, file_name is model name like ethmac