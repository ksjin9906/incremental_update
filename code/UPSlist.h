#pragma once
#include <vector>
#include <algorithm>
#pragma warning(disable:4996)
class UPSList {
private:
	// module number, UPS number
	vector<pair<int, int>> UPSs;

public :

	void add(int module_num, int ups_num) {
		UPSs.push_back(make_pair(module_num, ups_num));
	}

	vector<pair<int, int>>* getList() {
		return &UPSs;
	}

	void removeDup() { // 한 window 사이즈에 나타나는 ups 중복 없애기 
		sort(UPSs.begin(), UPSs.end());
		UPSs.erase(unique(UPSs.begin(), UPSs.end()), UPSs.end());
	}

};