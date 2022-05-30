#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <list>
#include <cstring>
#include <stdlib.h>
#include <vector>
#include "Node.h"
#include "ups.h"
#include <utility>
#pragma warning(disable:4996)
using namespace std;
#define MAX_MOD 500
void get_new_ups_table(vector<pair<int, int>> ups_table[], string filenames, int cyc, int clk_start, string file_name, int num_blocks, string mod[],int i, int num_clocks) {

	ifstream fin;
	ifstream et;//model file like ethermac

	vector<int> list_mod[MAX_MOD]; //0~끝까지의 ups리스트
	vector<int> time_list_mod[MAX_MOD]; //ups인 cycle의 번지수를 저장한 배열
	if (cyc == 0) return; //cycle is 0, no data

	cout << filenames << " Read Start (" << i + 1 << " / " << num_clocks << " )" << endl;
	fin.open(filenames); //clock file open which file is like MRX_CLK.comb.ps
	if (fin.fail()) {
		cout << "There is no file that name is" << filenames << endl;
		exit(100);
	}//file load fail	
	char line[20000];
	int mod_num = 0;
		while (!fin.eof()) {
			fin.getline(line, sizeof(line));
			if (line[0] == '#') {
				continue;
			}
			else {
				char* ptr = strtok(line, "\t");
				int state;
				if (ptr == NULL) {
					continue;
				}//if line is none
				else {
					ptr = strtok(NULL, "\t");
					for (int i = 0; i < num_blocks; i++) {
						ptr = strtok(NULL, "\t");
						if (ptr[0] != '-') {
							state = atoi(ptr);
						}
						list_mod[i].push_back(state);
					}
				}

			}
		}	//Get list_mode[MAX_MOD]

		for (mod_num = 0; mod_num < num_blocks; mod_num++) {
			et.open(file_name);
			if (et.fail()) {
				cout << "there is no file that name is " << file_name << endl;
				exit(100);
			}
			while (!et.eof()) {
				et.getline(line, sizeof(line));
				if (line[0] == '*') {
					char* ptr = strtok(line, " ");
					ptr = strtok(NULL, "-");
					char* mod_name = ptr;
					ptr = strtok(NULL, "\n");
					char* clk_name = ptr;
					strcat(clk_name, ".comb.ps");
					if (clk_name == filenames) {
						if (mod_name == mod[mod_num]) { //module 까지 일치하면 해당 정보vector에 저장
							et.getline(line, sizeof(line));
							et.getline(line, sizeof(line)); //두줄은 건너뛰기
							while (!et.eof()) {
								et.getline(line, sizeof(line));
								if (line[0] == '-') {
									break; // end line
								}
								else {
									////////////////////// 시구간을 0,1,2 ... ,n으로 표시해서 UPS가 존재하는 시구간을 time_list_mod에 추가///////////////
									char* token = strtok(line, "-");
									int start = atoi(token);
									token = strtok(NULL, ":");
									int end = atoi(token);
									token = strtok(NULL, " ");
									int cycle = atoi(token);
									int u_time = (start - clk_start) / cyc;
									cycle = cycle / 10;

									for (int i = 0; i < cycle; i++) {
										time_list_mod[mod_num].push_back(u_time + i);
									}
									////////////////////// 시구간을 0,1,2 ... ,n으로 표시해서 UPS가 존재하는 시구간을 time_list_mod에 추가///////////////
								}
							}
							break;
						}
					}
					continue;
				}
				else {
					continue;
				}
			}


			ups* up = new ups();

			if (time_list_mod[mod_num].size() == 0) {
				cout << filenames << " clock's " << mod_num + 1 << " table read (" << mod_num + 1 << "/" << num_blocks << ")" << endl;
				et.close();
				continue;
			}

			////////////////////////// root node 생성 ///////////////////////////////
			Node* rootnode = new Node(list_mod[mod_num][time_list_mod[mod_num][0]]);
			rootnode->setcount(1);
			up->setRoot(rootnode); 
			////////////////////////// root node 생성 ///////////////////////////////

			for (int i = 1; i < time_list_mod[mod_num].size(); i++) {
				int data = list_mod[mod_num][time_list_mod[mod_num][i]];
				Node* node = up->getRoot();
				node = up->find(data); //find node that data same 'this data'
				if (node) { //count +1
					up->update(node);
				}
				else {//no data, that data(ups) first time emerge
					up->make(data);
				}
			}

			up->Inorder(up->getRoot(), &ups_table[mod_num]); // ups번호 오름차순으로 ups_table 생성 (ups_table에는 해당 모듈에서의 (ups 번호, 해당 ups의 개수)정보가 담겨 있다.
			et.close();
			cout << filenames << " clock's " << mod_num + 1 << " table read (" << mod_num + 1 << "/" << num_blocks << ")" << endl;
		}


		cout << filenames << " Read End (" << i + 1 << " / " << num_clocks << " )" << endl;
	fin.close();
}