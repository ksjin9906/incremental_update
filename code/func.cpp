#include "func.h"
#pragma warning(disable:4996)

#define alg 1 // signal that use for parameter of function get time section, it means function (get_time_section) perform basic algorithm
#define early_alg 2 // signal that use for parameter of function get time section, it means function (get_time_section) perform get early time algorithm
#define MAX 10000 //MAX is Maginot Line about window size
#define MAX_MUL 128 //MAX_MUL is Maginot Line about multiple window size (ex. window size is under max_cycle*MAX_MUL)

using namespace std;
extern int num_clocks;
extern int num_blocks;

extern double target_hit_ratio;

extern UPSList* time_section[MAX_CLK];

extern int cycles[MAX_CLK];
extern int start[MAX_CLK];
extern int section_cnt[MAX_CLK]; // �ñ����� ��
extern int max_cycle;
extern int max_start;
extern double max_loc;
extern string file_name;
extern string fn;

extern int best_window_size;
extern double lowest_penalty; //get_time section�� �г�Ƽ
extern double hit_ratio_after_updating_best;
extern vector<pair<int, int>> best_periods; //get_time section�� �ñ���

// 22-02-11(LYY)
// copy from origin -> target
void copy_ups_table(vector<pair<int, int>>** origin, vector<pair<int, int>>** target) {
	for (int i = 0; i < num_clocks; i++) {
		target[i] = new vector<pair<int, int>>[num_blocks];
	}
	for (int clk = 0; clk < num_clocks; clk++) {
		for (int mod = 0; mod < num_blocks; mod++) {
			target[clk][mod].resize(origin[clk][mod].size());
			copy(origin[clk][mod].begin(), origin[clk][mod].end(), target[clk][mod].begin());
		}
	}
}


// ��� ��ȣ�� power state ��ȣ�� �����ϸ�
// UPS���� return
bool isUPS(vector<pair<int, int>> ups_table[], int module_num, int ups_num) {
	for (int i = 0; i < ups_table[module_num].size(); i++) {
		if (ups_num == ups_table[module_num][i].first) {
			if (ups_table[module_num][i].second == 0) // count = 0 (trained) -> treat as CPS (not UPS)
				return false;
			return true;
		}
	}
	return false;
}

// UPSList�� �־��� section�� cnt(UPS)���� ��, sum(section)���
int get_count_sum(vector<pair<int, int>> ups_table[], UPSList section) {
	vector<pair<int, int>>* list = section.getList();
	int sum = 0;

	for (int i = 0; i < list->size(); i++) {
		int module_num = list->at(i).first;
		int UPS_num = list->at(i).second;

		// get count
		for (int j = 0; j < ups_table[module_num].size(); j++) {
			if (UPS_num == ups_table[module_num][j].first) {
				sum += ups_table[module_num][j].second;
				break;
			}
		}
	}

	return sum;
}

void reflect_update(vector<pair<int, int>>** ups_table, UPSList section[]) {
	for (int clk = 0; clk < num_clocks; clk++) {
		reflect_update_clk(ups_table[clk], section[clk]);
	}
}

void reflect_update_clk(vector<pair<int, int>> ups_table[], UPSList section) {
	vector<pair<int, int>>* list = section.getList();

	for (int i = 0; i < list->size(); i++) {
		int module_num = list->at(i).first;
		int UPS_num = list->at(i).second;

		// make trained count 0 
		for (int j = 0; j < ups_table[module_num].size(); j++) {
			if (UPS_num == ups_table[module_num][j].first) {
				ups_table[module_num][j].second = 0; // make trained count zero
				break;
			}
		}
	}
}

// �ñ����� �� return
int count_sections(string filename) {
	ifstream fin;
	fin.open(filename);

	char line[20000];
	int cnt = 0;

	while (1) {
		if (fin.eof())
			break;

		fin.getline(line, sizeof(line));
		if (line[0] == '#')
			continue;
		else {
			char* ptr = strtok(line, "\t");
			if (ptr == NULL)
				continue;
			else
				cnt++;
		}
	}

	fin.close();

	return cnt;
}

// .comb ������ �а� �� ��° �ñ����� � UPS�� ���ԵǴ��� matching
// 
// ** parameter **
// <clk> 0 : WB_CLK , 1 : MRX_CLK, 2: MTX_CLK
// <ups_table> UPS�� power state�� ����
// <section_cnt> �ñ����� ���� ������ ����
// <total_sum> CPS�� UPS�� ����
// <cps_sum> CPS�� ��
void match_ups(string filename, UPSList* time_section, vector<pair<int, int>> ups_table[], int* total_sum, int* cps_sum,int num_blocks) {

	ifstream fin;
	fin.open(filename);

	char line[20000];

	int section_num = 0;

	while (1) {
		if (fin.eof())
			break;

		fin.getline(line, sizeof(line));

		if (line[0] == '#') {
			continue;
		}
		else {
			char* ptr = strtok(line, "\t");
			if (ptr == NULL)
				continue;
			ptr = strtok(NULL, "\t");
			for (int i = 0; i < num_blocks; i++) {
				ptr = strtok(NULL, "\t");
				if (ptr[0] == '-')
					continue;

				(*total_sum)++;

				int ups_num = atoi(ptr);
				if (isUPS(ups_table, i, ups_num)) {
					time_section[section_num].add(i, ups_num);
				}
				else
					(*cps_sum)++;
			}
			section_num++;
		}
	}
	fin.close();
}


double caculate_penalty(int window_size, int blocks_needed, vector<pair<int, int>> extracted) {
	double penalty = 0;
	
	double alpha = 1.0;//total train time 
	double beta = 7500.0; //total block_needed
	double gamma = 0.0; //end of train

	sort(extracted.rbegin(), extracted.rend()); 

	double penalty_training = alpha* window_size * blocks_needed;
	double penalty_setting_vcd = beta * blocks_needed;
	double penalty_max_dump_location= gamma * extracted.begin()->second;

	penalty = penalty_training + penalty_setting_vcd + penalty_max_dump_location;

	return penalty;

}
void remove_overlap(vector<pair<int, int>>&best) {
	double prev = -1.00;
	for (auto it = best.begin(); it != best.end(); it++) {
		if (prev > it->first) {
			prev = it->second;
			double temp_end = it->second;
			(--it)->second = temp_end;
			best.erase(++it);
			continue;
		}
		prev = it->second;
	}
}
void write_dumpfile(vector<pair<int, int>> &best, string file_name,double hit_ratio) {
	ofstream fout;
	string filename = file_name + "_dump.txt";
	double total_dumptime = 0.00;
	fout.open(filename);
	remove_overlap(best);
	fout << fixed;
	fout.precision(2);
	fout << "// Optimized interval information" << endl;
	for (auto it = best.begin(); it != best.end(); it++) {
		fout << "// " << (double)it->first << "ns - "<<(double)it->second<<"ns ("<<it->second- it->first<< "ns)" << endl;
		total_dumptime += (it->second - it->first+(max_cycle/5));
		//it->first *= 1000.00;
		if (it->first != 0.00)it->first -= (double)max_cycle*0.20;
		//it->second *= 1000.00;
	}
	fout << "// number of time window : " << best.size() << endl;
	fout << "// total dump time : " << (double)(best.begin()->second - best.begin()->first) * best.size()<<endl;
	fout << "// Expected hit ratio : " << hit_ratio << endl;
	fout << endl << endl<<"// Testbench insert section start"<<endl;
	fout << "initial begin" << endl;
	if (best.begin()->first != 0.0) {
		fout << "\t$dumpoff;\t\t// start dumpoff" << endl;
	}
	double prev_time = 0.00;
	for (auto it = best.begin(); it != best.end(); it++) {
		fout << "\t#" << (double)(it->first-prev_time) << "\t\t$dumpon;"<<endl;
		fout << "\t#" << (double)it->second-it->first << "\t\t$dumpoff;"<<"\t\t// "<<it->first<< "~"<< it->second <<"ns"<< endl;
		prev_time = it->second;
	}
	fout << "\t$finish;" << endl;
	fout << "end" << endl;
	fout << "// Testbench insert section end" << endl;
	fout.close();
}

int getmax(int num_clocks, int k[]) {
	int max = 0;
	for (int i = 0; i < num_clocks; i++) {
		if (max < k[i]) {
			max = k[i];
		}
	}
	return max;
}

// 22-02-12(Lyy)
// ���� window size���� sum(window)�� ���� ū best window�� ����
// sum(window) : window���� UPS���� �󵵼��� ��
void find_best_window(Window_Info* best_window, int window_size, vector<pair<int, int>>** ups_table, double minimum_ups_count_for_hitratio, int sig,int sel) {
	int window_start_time = 0;
	int window_end_time = window_size + max_start; // -- begin cycle max�� �׻� ������ġ�� ũ�� ���� ���� ���� ��

	int index[MAX_CLK] = { 0, };
	int section_start_time[MAX_CLK];
	int section_end_time[MAX_CLK];
	double max_percent = 99999;

	for (int j = 0; j < num_clocks; j++) { section_start_time[j] = start[j]; }
	for (int j = 0; j < num_clocks; j++) { section_end_time[j] = start[j] + cycles[j]; }

	int max = -1;
	int real_max = -1;

	bool finished[MAX_CLK] = { false, };

	while (1) { // window �̵� ��Ű�� while��
		int for_break = 1;
		for (int i = 0; i < num_clocks; i++) {
			if (finished[i] == false)for_break = 0;
		}
		if (for_break == 1) {
			break;
		}

		int window_sum = 0;

		UPSList ups_in_window[MAX_CLK]; // �� clock ���� �ش� window ���� ���ԵǴ� UPS�� list
		for (int clk = 0; clk < num_clocks; clk++) { // ���� window���� clk�� sum(window) ����ϴ� for��
			if (finished[clk])
				continue;

			// window �������� �̵��ϴ°� �ƴ϶� max cycle �������� �̵��ϴ� ���,
			// �ߺ��ż� ���� �� �ֱ⶧����
			// �Ź� ���� index ���� ��ġ����
			// ���� window ���η� index�� �̵��ϵ��� while������ ��ߵ�
			// (index�� �����ص��� �ʰ� ��� ������Ű�� �ߺ��ż� ���ԵǴ� �ñ����� ���ȵ�)
			int temp_start_time = section_start_time[clk];
			int temp_index = index[clk];

			while (section_start_time[clk] < window_start_time) {
				section_start_time[clk] += cycles[clk];
				index[clk] ++;
			}

			if (index[clk] >= section_cnt[clk]) { // 22-02-22(Lyy) �� �κ� �����ϸ� �� �ñ����� ����غ��°� ������ ��
				finished[clk] = true;
				continue;
			}

			section_end_time[clk] = section_start_time[clk] + cycles[clk];

			while (section_end_time[clk] <= window_end_time) {

				for (auto it = time_section[clk][index[clk]].getList()->begin(); it != time_section[clk][index[clk]].getList()->end(); it++) {
					ups_in_window[clk].add(it->first, it->second);
				}

				section_start_time[clk] += cycles[clk];
				section_end_time[clk] += cycles[clk];
				index[clk]++;

				if (index[clk] >= section_cnt[clk]) {
					finished[clk] = true;
					break;
				}
			}

			ups_in_window[clk].removeDup();
			window_sum += get_count_sum(ups_table[clk], ups_in_window[clk]);

			// �Ź� ���� ��ġ�� �̵� ������ߵ�
			// ������ ���Եƴ� �ñ����� �ٽ� ���Ե� �� �ֵ���.
			section_start_time[clk] = temp_start_time;
			index[clk] = temp_index;
		}

		if (window_sum > max&& sig == 0&&sel==alg) {//basic, 1 th algorithm
			max = window_sum;
			best_window->sum_window = max;
			best_window->max_pos.first = window_start_time;
			best_window->max_pos.second = window_end_time;
			for (int i = 0; i < num_clocks; i++) {
				best_window->ups_in_window[i] = ups_in_window[i];
			}
			if (max > minimum_ups_count_for_hitratio)sig = 1;
		}
		else if (window_sum > max&&sel==early_alg) { //get early_time, 2 nd algorithm
			double per_max = (double)window_sum / minimum_ups_count_for_hitratio;
			if (per_max >= 1)per_max = 1.0; //per_max minimum_ups_count_for_hitratiost 0~1
			double per_loc = (double)window_end_time / max_loc;
			double percent = per_loc / per_max;
			max = window_sum;
			if (max_percent > percent) {
				best_window->max_pos.first = window_start_time;
				best_window->max_pos.second = window_end_time;
				best_window->sum_window = max;
				real_max = max;
				max_percent = percent;
				for (int i = 0; i < num_clocks; i++) {
					best_window->ups_in_window[i] = ups_in_window[i];
				}
			}
		}

		window_start_time += max_cycle;
		window_end_time += max_cycle;
	}
}

// 22-02-12 (Lyy)
// �־��� window size���� hit ratio�� �ѱ� �� �ִ� section���� ����
// sel���� alg�̸� get_time_section���� ȣ��
// sel���� early_alg�̸� get_early_time_section���� ȣ��
Extract_Results extract_sections(vector<pair<int, int>>* extracted, vector<int>* num_update, vector<pair<int, int>>** ups_table, int window_size, int original_cps_sum, int total_sum,int sel){

	double minimum_ups_count_for_hitratio = (double)total_sum * target_hit_ratio / 100.00;//minimum_ups_count_for_hitratio ������ target hitratio�� �޼��ϱ� ���� ������Ʈ �ؾ��� ups�� �ּҰ����̴�.
	minimum_ups_count_for_hitratio = minimum_ups_count_for_hitratio - original_cps_sum;
	int sig = 0;

	int block_cnt = 0;

	int cps_sum = original_cps_sum;
	double current_hit_ratio = (double)cps_sum / total_sum;

	while (current_hit_ratio < target_hit_ratio) { // �ݺ� �� ���� block �ϳ� �̴� while��
		block_cnt++;

		Window_Info best_window_info;

		find_best_window(&best_window_info, window_size, ups_table, minimum_ups_count_for_hitratio, sig,sel);

		reflect_update(ups_table, best_window_info.ups_in_window);

		cps_sum += best_window_info.sum_window;
		current_hit_ratio = ((double)(cps_sum) / total_sum) * 100;
		minimum_ups_count_for_hitratio = minimum_ups_count_for_hitratio - best_window_info.sum_window;

		extracted->push_back(make_pair(best_window_info.max_pos.first, best_window_info.max_pos.second));
		num_update->push_back(best_window_info.sum_window);
	} // �ݺ� �� �� �� best window �ϳ� �����ϴ� while�� ��

	Extract_Results result;
	result.blocks_needed = block_cnt;
	result.hit_ratio = current_hit_ratio;
	return result;
}

void write_result(int window_size, int blocks_needed, vector<int> num_update, vector<pair<int, int>> extracted, double hit_ratio,int sel) {
	cout << "<<Window Size = " << window_size << ">>" << endl;
	cout << "Blocks needed = " << blocks_needed << endl;
	cout << "----------------------------------------------------------------" << endl;
	cout << setw(10) << "Start" << "\t|\tEnd" << "\t  |  Can update UPS Block(s)" << endl;
	cout << "----------------------------------------------------------------" << endl;
	auto it_num_update = num_update.begin();
	for (auto it = extracted.begin(); it != extracted.end(); it++) {
		cout << setw(13) << it->first << "ns | " << setw(13) << it->second << "ns | " << setw(15) << *it_num_update << endl;
		it_num_update++;
	}

	double current_penalty = caculate_penalty(window_size, blocks_needed, extracted);
	
		if (current_penalty < lowest_penalty) {
			lowest_penalty = current_penalty;
			best_periods = extracted;
			best_window_size = window_size;
			hit_ratio_after_updating_best = hit_ratio;
		}
	

	cout << "----------------------------------------------------------------" << endl;
	cout << "Penalty : " << current_penalty << endl;
	cout << "Hit ratio after update : " << hit_ratio << endl;
	cout << endl << endl;

}
/*
 func_name : get_time_section
 func : get best time section for incremental update
 if sel_alg ==alg -> get_time_section by basic algorithm
 else if sel_alg==early_alg -> get_early_time_section by get early time algorithm
*/

/// <summary>
/// fddfdfdddd
/// </summary>
/// <param name="new_ups_table"></param>
/// <param name="original_cps_sum"></param>
/// <param name="total_sum"></param>
/// <param name="sel_alg"></param>
void get_time_section(vector<pair<int, int>>** new_ups_table, int original_cps_sum, int total_sum,int sel_alg) {
	cout << endl << sel_alg<<"_Algorithm Start" << endl;

	int blocks_needed = -1;
	double window_size = (num_clocks == 1) ? (double)max_cycle : (double)max_cycle * 2;
	double dcpw;
	int icpw;
	double initial_window_size = (double)window_size;
	double last_window_size=window_size;
	int reducing_sig = 0; //if block_needed is 1 ,  reducing sig=1, and reducing window size until block_needed over 1 or window size below initial_window_size*2
	int up_sig = 0;
	while (window_size<max_cycle*MAX_MUL&& window_size < MAX) { // window ũ�� �ٲ㰡�鼭 �õ� �ϴ� while ��
		// �Ź� copy�ϴ� ���� : reflect update���� ups table�� �����ϱ� ������,
		// ���ο� window size�� ó������ ���������� ������ �����ؾߵ�.
		vector<pair<int, int>>** ups_table_copy = new vector<pair<int, int>> *[num_clocks];// first = UPS number, second = count
		copy_ups_table(new_ups_table, ups_table_copy);

		Extract_Results result; // blocks_needed�� update ���� hit ratio ������ ��� ����ü

		vector<pair<int, int>> extracted;
		vector<int> num_update;

		// ���� window size�� hit ratio�� �ѱ� �� �ֵ��� �ñ��� ����
		result = extract_sections(&extracted, &num_update, ups_table_copy, window_size, original_cps_sum, total_sum,sel_alg);

		blocks_needed = result.blocks_needed;

		write_result(window_size, blocks_needed, num_update, extracted, result.hit_ratio, sel_alg);

		cout << endl << endl;
		if (sel_alg == alg) {
			bool brk = decide_size(blocks_needed, reducing_sig, up_sig, window_size, last_window_size, initial_window_size);
			if (!brk) {
				break;
			}
		}
		else if (sel_alg == early_alg) {
			window_size *= 2;
			if (blocks_needed == 1) {
				break;
			}
		}
	} // ������ window size �õ��ϴ� while���� ��

	cout << endl << endl;
	sort(best_periods.begin(), best_periods.end());

}

double get_maxloc() {
	double max = 0;
	for (int i = 0; i < num_clocks; i++) {
		double size = section_cnt[i];
		if (max < size * cycles[i])max = size * cycles[i];
	}

	return max;
}

void print_best_time_section() {

	write_dumpfile(best_periods, fn, hit_ratio_after_updating_best); //write dump file
	cout << endl << endl;
	cout << "<<<<Final Result>>>>" << endl;
	cout << " - Best window size : " << endl << best_window_size << endl << endl;;
	cout << " - Best sections to upgrade : " << endl;
	cout << "------------------------------------" << endl;
	cout << "start   ~   end" << endl;
	cout << "------------------------------------" << endl;
	for (auto it = best_periods.begin(); it != best_periods.end(); it++) {
		cout << (double)it->first << "ns ~ " << (double)it->second << "ns" << endl;
	}
	cout << "------------------------------------" << endl;
	cout << endl;
	cout << "Hit ratio after update : " << hit_ratio_after_updating_best << endl;
}

void input_filename() {
	cout << "Input model name (ex. ethmac) : ";
	cin >> file_name; //input file_name
	fn = file_name;
	file_name = file_name + ".pm.quality.rpt";
}

void input_target_hit_ratio(double &target_hit_ratio, int cps_sum, int total_sum) {
	cout << "before hit ratio = " << cps_sum << '/' << total_sum << " = " << (double)cps_sum / total_sum << endl << endl;
	while (1) {
		cout << "Input Target Hit Ratio(0~100%, if you want exit, input -1) : ";
		cin >> target_hit_ratio;//input target hitratio
		if (target_hit_ratio == -1) {
			exit(100);
		}
		else if (target_hit_ratio / 100 < (double)cps_sum / total_sum) { //if already over hit ratio, exit
			cout << "already over " << target_hit_ratio << " hit ratio " << endl;
			cout << "current hit ratio is " << (double)cps_sum / total_sum << endl;
			continue;
		}
		else if (target_hit_ratio < 0 || target_hit_ratio>100) {
			cout << "Target hit ratio is minimum_ups_count_for_hitratiost be \"0~100 %\"" << endl;
			continue;
		}
		else {
			break;
		}
	}
}

//func name: decide_size
// perform : 1��° �ñ��� ���� �˰��򿡼� ���� window size�� �������ִ� �Լ�
// ó�� block_needed�� 1�� �����ϱ� ������ ������������ 2�辿 ����
//block_needed�� 1�� �޼��ϰ� �Ǹ� �ش� size ���� ���� size���� block_needed�� 1�����Ҽ� �ִ��� Ž��
bool decide_size(int block, int &down, int &up, double &tmp, double &last,double init) {
	if (down == 0) {
		if (block == 1) {
			down = 1;
			if (tmp <= init * 2) {
				return false;
			}
			tmp = (tmp + last) / 2;
			return true;

		}
		else {
			last = tmp;
			tmp = tmp * 2;
			return true;
		}

	}
	else {
		if (up == 0) {
			if (block == 1) {
				tmp = (tmp + last) / 2;
				if (check_int(tmp, init)) {
					return true;
				}
				else {
					return false;
				}
			}
			else {
				up = 1;
				last = 2 * tmp - last;
				tmp = (tmp + last) / 2;
				if (check_int(tmp, init)) {
					return true;
				}
				else {
					return false;
				}
			}
		}
		else {
			if (block == 1) {
				up = 0;
				last = 2 * tmp - last;
				tmp = (tmp + last) / 2;
				if (check_int(tmp, init)) {
					return true;
				}
				else {
					return false;
				}
			}
			else {
				tmp = (tmp + last) / 2;
				if (check_int(tmp, init)) {
					return true;
				}
				else {
					return false;
				}
			}

		}
	}
}

// func name : check_ int
// perform : window size a�� ����. �ش� a �� initial window_size�� ���������� äũ �����谡 �ƴ϶�� return false
bool check_int(double a,double initial) {
	a = a / initial;
	int b = a;
	if (a==b)return true;
	else return false;
}