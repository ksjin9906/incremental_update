#pragma once

#include "UPSlist.h"

#define MAX_CLK 20 

using namespace std;

typedef struct window_info {
	pair<int, int> max_pos;
	int sum_window; // sum(Window) : Sum of UPS's count which are in window
	UPSList ups_in_window[MAX_CLK];
}Window_Info;

typedef struct extract_results {
	int blocks_needed;
	double hit_ratio;
}Extract_Results;