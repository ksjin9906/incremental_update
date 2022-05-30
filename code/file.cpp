#include "file.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#pragma warning(disable:4996)
using namespace std;

/// read file, and get clock's information(numbers of clocks, each clock's name, numbers of modules)
/// total clock cycle has name of clocks
void get_file_info(string file_name, string filenames[],int &num_clocks, int &num_blocks, int cycles[], int start[],string mod[],int &max_cycle, int &max_start) {
	ifstream fin;
	fin.open(file_name);
	if (fin.fail()) {
		cout << "there is no " << file_name << " file " << endl;
		exit(100);
	}//file load fail

	char line[200];
	int i = 0;// line
	num_clocks=0; //number of clocks
	num_blocks = 0;// number of module blocks

	while (!fin.eof()) {
		i++;
		fin.getline(line, sizeof(line));
		if (line == NULL) { //line is NULL continue
			continue;
		}
		else if (line[1] == '*') { // #**** continue
			continue;
		}
		char* ptr = strtok(line, " ");
		if (ptr== NULL) {
			continue;
		}
		else if (!strcmp(ptr, "Clock")) { //to get clock file names
			fin.getline(line, sizeof(line));
			for (int j = 0; j < num_clocks; j++) {
				fin.getline(line, sizeof(line));
				char* ptr;
				ptr = strtok(line, " ");
				filenames[j] = ptr;
				filenames[j] = filenames[j] + ".comb.ps"; //because file name is like wb_clk.comb.ps
				cout << filenames[j] << endl;
			}
		}
		else if (!strcmp(ptr, "Block")) {	//to get module_names 
			fin.getline(line, sizeof(line));
			for (int j = 0; j < num_blocks; j++) {
				fin.getline(line, sizeof(line));
				if (line[0] == '_' || line[0] == '-'||line[0]==' ') {
					j--;
					continue;
				}
				char* ptr;
				ptr = strtok(line, " ");
				mod[j] = ptr;
			}
			break;
		}
		ptr = strtok(NULL, " ");
		if (ptr == NULL) {
			continue;
		}
		if (!strcmp(ptr,"NUM.")) {
			ptr = strtok(NULL, " "); //ptr is clocks or blocks
			if (!strcmp(ptr, "CLOCKS")) {  //READ NUM_CLOCKS
				ptr = strtok(NULL, " ");
				ptr = strtok(NULL, " ");
				num_clocks = atoi(ptr);
			}
			else { //READ NUM_BLOCKS
				ptr = strtok(NULL, " ");
				ptr = strtok(NULL, " ");
				num_blocks = atoi(ptr);
			}
		}
	}

	fin.close();



	///for get cycles and start about each clocks
	for (int k = 0; k < num_clocks; k++) {
		ifstream fclk;
		fclk.open(filenames[k]);
		if (fclk.fail()) {
			cout << filenames[k] << " read failed" << endl;
			exit(100);
		}//file load fail
		while (!fclk.eof()) {
			fclk.getline(line, sizeof(line));
			if (line[0] == '#') {
				continue;
			}
			else {
				fclk.getline(line, sizeof(line));
				char* ptr = strtok(line, "\t");
				if (ptr == NULL) { //no data
					start[k] = 0;
					cycles[k] = 0;
					fclk.close();
					continue;
				}
				start[k] = atoi(ptr);
				ptr = strtok(NULL, "\t");
				int num = atoi(ptr);
				cycles[k] = num - start[k];
				fclk.close();
				break;
			}
		}
	}
	
	max_cycle = getmax(num_clocks, cycles); //get max cycle 
	max_start = getmax(num_clocks, start); //get max start location

}

