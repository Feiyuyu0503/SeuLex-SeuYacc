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

// ��ȷ���������Զ���:
// ״̬����Ϊ Ntran ��������
// ������ĸ��Ϊ ASCII �ַ�����ͬ �� �� 129 �У�
// ת������Ϊ��Ա Ntran
// ��ʼ״̬�� Ntran �ĵ�һ��
// ����״̬�� Ntran �����һ��
class NFA{
public:
	typedef array<vector<size_t>, 129> NST;	// NFA ״̬���е�ÿһ�У�������״̬��ת��
											// ÿһ�к� 129 �� vector
											// ��ʾ�� 128 �� ASCII �� epsilon �ϵĺ�̼���
											// �����ʱ�Ͳ����ظ������� set �� vector ��ӡ�����Ч�ʸ���
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
	deque<NST> Ntran;		// state �ļ��ϣ��� deque �����������ʺ�˫����ɾ��
};

// ȷ���������Զ�����
// ״̬����Ϊ Dtran ��������
// ������ĸ��Ϊ ASCII �ַ���128 �У�
// ת������Ϊ��Ա Dtran
// ��ʼ״̬ʱ Dtran ��һ��
// ����״̬�ڳ�Ա accepts ������
class DFA {
public:
	typedef array<size_t, 128> DST;		// DFA ״̬���е�ÿһ�У�������״̬��ת��
										// ÿһ�к� 128 �� size_t
										// ��ʾ�� 128 �� ASCII �ϵĺ�̣��� none ��ʾ��
	DFA(const NFA& , const vector<size_t>& );
	// DFA(const NFA& , size_t );
	inline size_t get_size()const { return Dtran.size(); }
	inline size_t get_tran(size_t i, size_t ch)const { return Dtran[i][ch]; }
	inline const vector<size_t> get_accepts()const { return accepts; }
	void minimize();
	void delete_dead_states();
private:
	vector<DST> Dtran;				// ״̬ת��
	vector<size_t> accepts;			// �����ܣ�״̬��Ӧ��ģʽ��ţ��ǽ���״̬��Ӧ -1

	DST newDST() {					// -1 ��ʾ��ת��
		DST st;
		for (size_t& i : st) {
			i = -1;
		}
		return st;
	}
};