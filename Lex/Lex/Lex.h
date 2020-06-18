#pragma once
#include <fstream>
#include <vector>
#include <deque>
#include <string>
#include <map>
#include <array>
using std::ifstream;
using std::ofstream;
using std::vector;
using std::deque;
using std::array;
using std::map;
using std::string;

int ParseLexFile(ifstream& ifs, ofstream& ofs);

// 不确定的有穷自动机:
// 状态集合为 Ntran 的所有行
// 输入字母表为 ASCII 字符（连同 ε 共 129 列）
// 转换函数为成员 Ntran
// 开始状态是 Ntran 的第一行
// 接受状态是 Ntran 的最后一行
class NFA{
public:
	typedef array<vector<size_t>, 129> NST;	// NFA 状态表中的每一行：到其他状态的转换
											// 每一行含 129 个 vector
											// 表示在 128 个 ASCII 和 epsilon 上的后继集合
											// 在添加时就不会重复，不用 set 用 vector 添加、更改效率更高
	NFA() {
		Ntran.push_back(NST());
	}
	NFA(char ch);
	inline size_t get_size()const { return Ntran.size(); }
	void opt_union(const NFA&);
	void opt_concat(const NFA&);
	void opt_star();
	void opt_plus();
	void opt_quest();
	vector<size_t> merge_nfa(const vector<NFA>&);
	vector<size_t> epsilon_closure(size_t s)const;
	vector<size_t> epsilon_closure(const vector<size_t>& ss)const;
	vector<size_t> move(const vector<size_t>& ss, char a)const;
	~NFA() {}
private:
	deque<NST> Ntran;		// state 的集合（用 deque 更快的随机访问和双端增删）
};

// 确定的有穷自动机：
// 状态集合为 Dtran 的所有行
// 输入字母表为 ASCII 字符（128 列）
// 转换函数为成员 Dtran
// 开始状态时 Dtran 第一行
// 接受状态在成员 accepts 中体现
class DFA {
public:
	typedef array<size_t, 128> DST;		// DFA 状态表中的每一行：到其他状态的转换
										// 每一行含 128 个 size_t
										// 表示在 128 个 ASCII 上的后继，用 none 表示空
	DFA(const NFA& , const vector<size_t>& );
	// DFA(const NFA& , size_t );
	inline size_t get_size()const { return Dtran.size(); }
	inline size_t get_tran(size_t i, size_t ch)const { return Dtran[i][ch]; }
	inline const vector<size_t> get_accepts()const { return accepts; }
	void minimize();
	void delete_dead_states();
private:
	vector<DST> Dtran;				// 状态转换
	vector<size_t> accepts;			// （接受）状态对应的模式编号，非接受状态对应 -1

	DST newDST() {					// -1 表示无转换
		DST st;
		for (size_t& i : st) {
			i = -1;
		}
		return st;
	}
};