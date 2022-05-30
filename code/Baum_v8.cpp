#include <iostream>
#include <fstream>
#include "ups.h"
#include "Node.h"
#include "upstable.h"
#include "file.h"
#include "UPSlist.h"
#include "func.h"
#include <stdlib.h>
#include <time.h>
#include <iomanip>
#pragma warning(disable:4996)
// ver 22/ 01/ 10

// 추가로 필요한 것 :
// (1) 시간에 따른 패널티 계산 실험
// (2) 추출된 시구간 위치에 대한 패널티 계산 실험
// (1)과 (2)의 결과를 통해 알파, 배타, 감마 값 설정 해주기 
// func 파일 정리, 주석 달기

// 최종적으로 caculate_penalty함수 내용 수정과 코드 정리 


// ver 22/ 01 / 17
// : 시구간을 통합하는게 가장 좋은 방법 같음
// (0) cycle 등도 file 읽어서 결정하는 것으로 바꾸기
// (1) 시구간 통합할 때 크기 결정하는 법 잘 생각해보기

// ver 22 /01/ 22
// 일반화 파일 이름 입력 받고 실행 (현재 txt파일 형식으로 저장되는데(WB_CLK -> WB_CLK.txt) 
// 먼저 파일 정보 출력하고 hit ratio 입력받도록 하기.
// 일반화 작업 수행 시 clk개수는 최대 20개 로 생각 만약 clk가 더늘어날경우 해당 숫자 바꿔주어야함. MAX_CLK 숫자 바꿔주면 됨. module 도마찬가지 MAX_MODULE은 500으로 설정

// ver 05 22 /01/ 26
// ups table 생성이 오래걸리는 파일이 존재하여 (ex. NV_NVDLA) 해당 파일을 읽는 진행과정을 시각화 추가
// NV_NVDLA 등의 길이가 긴 파일도 읽도록 수정

//ver 06 22 /01 /26
// 기존 txt파일로 읽던 방식은 텍스트파일로 변환 해줘야하기 때문에 .rpt, .comb 파일을 읽도록 수행
// 기존 upstable 생성 시간이 너무 길던 문제 해결

//ver 07 22 /02 /04
// design model이 있는데 해당 모델에서는 기존 다른 모델들과 파일 양식이 달라 line 수말고 , 특정단어를 읽어 블록과 클럭수 읽기
// 클럭 사이클이 없을 경우 예외처리
// design 모델의 경우 start end 형식이 아니여서 그런지 에러남 design 모델에 대해서는 추후에 수정
// 소모시간 출력 1. 프로그램 전체 실행 시간 2. 프로그램 입력을 제외한 돌아가는 시간
// 마크다운 형식으로  결과 출력
// 최종 시구간 오름차순 정렬
// dump file생성

//ver 07.1 22 /02 /07
//dump file이름 구체화, dumpfile 내용 수정
//코드 정리

// ver 08 22 /02 /11
// 이른 시간대 추출을 위한 함수 get_earlty_time_section()함수 추가, 특정 hit ratio이하에선 이른 시간대의 추출 기대

// ver 08.02 22 /02 /14


//ver 09. 22/02/16
// 코드 정리(진행중)
// block_needed가 하나가 되는 윈도우 사이즈 최소 탐색

//ver 10 22/02/25
// dump file 내용 수정 -> (겹치는 구간 하나로 통일, 마지막에 $finish; 추가)


//ver 10 22.02.28
// dumpfile 내용 수정 -> 업데이트는 ps단위로 진행되므로 ns -> ps 단위 수정
// 업데이트시 앞의 2사이클은 버려진다. (powerbaum tool내 기능으로 인한) -> 시구간 추출 시 앞 부분에 2cycle 추가
#define MAX_CLK 20
#define MAX_MOD 500
#define alg 1 // signal that use for parameter of function get time section, it means function (get_time_section) perform basic algorithm
#define early_alg 2 // signal that use for parameter of function get time section, it means function (get_time_section) perform get early time algorithm


UPSList* time_section[MAX_CLK];
string file_name;//file_name is like ethmac.txt
string filenames[MAX_CLK]; //filenames like WB_CLK.txt
string mod_names[MAX_MOD]; //module names
string fn; //file nmae for write dump file
double target_hit_ratio; // 사용자로부터 입력 받을 target hit ratio
double max_loc; // 모든 클럭에 대해 가장 마지막 시구간의 위치
int num_clocks, num_blocks; //num clocks : 클럭 수, num_blocks : 모듈블럭의 수
int cycles[MAX_CLK] = {0,}; //각 클럭별 클럭 사이클 주기 정보
int start[MAX_CLK] = { 0, }; //각 클럭별 클럭의 시작 위치 
int section_cnt[MAX_CLK] = { 0 , }; //각 클럭별 시구간의 수 
int max_cycle = 0;//여러 클럭 사이클 중 가장 최대값을 갖는 사이클 주기
int max_start = 0;//여러 클럭 시작 위치 중 가장 최대값을 갖는 시작위치

int best_window_size; //get time section() 에서 가장 best time section에 대한 window size
double lowest_penalty = 99999999; //get_time section()  의 패널티
double hit_ratio_after_updating_best; //get time section() 에서 가장 best time section에 대한 expected hit ratio
vector<pair<int, int>> best_periods; //get_time section()의 시구간


int main()
{
	int total_sum = 0;   // CPS + UPS의 합
	int cps_sum = 0;     // CPS의 합

	input_filename(); 

	get_file_info(file_name,filenames, num_clocks, num_blocks,cycles,start,mod_names,max_cycle,max_start); //get file's information (ex. num_clocks, num_blocks, module names)

	vector<pair<int, int>> **new_ups_table=new vector<pair<int, int>> *[num_clocks];// first = UPS number(int), second = count(int)
	
	
	for (int i = 0; i < num_clocks; i++) {
		new_ups_table[i] = new vector<pair<int, int>>[num_blocks];
	}//dynamic allocation


	for (int i =0; i < num_clocks; i++) {
		get_new_ups_table(new_ups_table[i], filenames[i], cycles[i], start[i], file_name, num_blocks, mod_names, i, num_clocks);
	}//get ups_table 
	
	for (int i = 0; i < num_clocks; i++) {
		section_cnt[i] = count_sections(filenames[i]);
	}//count time section about each clock

	for (int i = 0; i < num_clocks; i++) {
		time_section[i] = new UPSList[section_cnt[i]];
		match_ups(filenames[i], time_section[i], new_ups_table[i], &total_sum, &cps_sum,num_blocks);
	} //get 'what ups in the time_section'
	
	system("cls");//cmd clear
	cout << file_name << " read success" << endl << endl;

	input_target_hit_ratio(target_hit_ratio, cps_sum, total_sum); //function that input target hit ratio from user

	max_loc = get_maxloc(); //function that get max_loc (max_loc is max end location about all clocks) 
	
	get_time_section(new_ups_table,cps_sum,total_sum,alg); //function can get best time section for incremental update
	//get_time_section(new_ups_table, cps_sum, total_sum, early_alg); //function can get early time section for incremental update


	print_best_time_section(); // print best_time_section and write dump file

	return 0;
}
