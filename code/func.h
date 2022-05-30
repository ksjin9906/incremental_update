#pragma once
#pragma warning(disable:4996)
#include "ups.h"
#include "Node.h"
#include "upstable.h"
#include "UPSlist.h"
#include "types.h"
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <stdlib.h>
using namespace std;
// ��� ��ȣ�� power state ��ȣ�� �����ϸ�
// UPS���� return
bool isUPS(vector<pair<int, int>> ups_table[], int module_num, int ups_num);


// UPSList�� �־��� section�� cnt(UPS)���� ��, sum(section)���
int get_count_sum(vector<pair<int, int>> ups_table[], UPSList section);


void reflect_update(vector<pair<int, int>>** ups_table, UPSList section[]);

// section�� ���Ե� ups���� count�� 0���� ó����.
void reflect_update_clk(vector<pair<int, int>> ups_table[], UPSList section);


// �ñ����� �� return
int count_sections(string filename);


// .comb ������ �а� �� ��° �ñ����� � UPS�� ���ԵǴ��� matching
// 
// ** parameter **
// <clk> 0 : WB_CLK , 1 : MRX_CLK, 2: MTX_CLK
// <ups_table> UPS�� power state�� ����
// <section_cnt> �ñ����� ���� ������ ����
// <total_sum> CPS�� UPS�� ����
// <cps_sum> CPS�� ��
void match_ups(string filename, UPSList* time_section, vector<pair<int, int>> ups_table[], int* total_sum, int* cps_sum, int num_blocks);

// caculate penalty 
// consider update size, blocks_needed, section location
double caculate_penalty(int window_size, int blocks_needed, vector<pair<int, int>> extracted);

// write dumpfile for get vcd file

void remove_overlap(vector<pair<int, int>>&best);

void write_dumpfile(vector<pair<int, int>>& best,string file_name, double hit_ratio); //write dump file

//get max about k[]
int getmax(int num_clocks,int k[]);

// copy ups table
void copy_ups_table(vector<pair<int, int>>** origin, vector<pair<int, int>>** target);


// 22-02-12(Lyy)
void find_best_window(Window_Info* best_window, int window_size, vector<pair<int, int>>** ups_table, double minimum_ups_for_hitratio, int sig,int sel);



// 22-02-12(Lyy)
Extract_Results extract_sections(vector<pair<int, int>> *extracted, vector<int> *num_update, vector<pair<int, int>>** ups_table, int window_size, int original_cps_sum, int total_sum,int sel);

/// <summary>
/// dffefe
/// </summary>
/// <param name="new_ups_table"></param>
/// <param name="original_cps_sum"></param>
/// <param name="total_sum"></param>
/// <param name="sel_alg"></param>
void get_time_section(vector<pair<int, int>>** new_ups_table, int original_cps_sum, int total_sum,int sel_alg);

double get_maxloc(); //get  max clock location

void print_best_time_section(); //print best time section and write dumpfile

void input_filename(); //func input filename

void input_target_hit_ratio(double &target_hit_ratio, int cps_sum, int total_sum); //func input target hit ratio

void write_result(int window_size, int blocks_needed, vector<int> num_update, vector<pair<int, int>> extracted, double hit_ratio,int sel);

bool decide_size(int block, int &down, int &up,double &tmp,double &last,double init); //decide window_size

bool check_int(double a, double initial);
