#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <set>
#include <array>
#include <list>
#include "Lex.h"

using namespace std;

// ������� char ��ʾ������ʽ�еĴ��أ����λΪ 1 ����������������ǲ�����

// ������תΪ�����
inline char to_operator(char ch) {
	return ch | 0x80;
}
// �����תΪ������
inline char to_char(char ch) {
	return ch & 0x7f;
}
// �ж��Ƿ��������
inline bool is_optr(char ch) {
	return ch & 0x80;
}

// �й� NFA �Ķ���

// ���ַ�תΪ NFA
NFA::NFA(char ch) {
	NST s0, s1;
	s0[(size_t)ch].push_back(1);
	Ntran.push_back(s0);
	Ntran.push_back(s1);
}
// Thompson: ��
void NFA::opt_union(const NFA& rhs) {	// ȡ����
	size_t x = get_size();
	size_t y = rhs.get_size();
	Ntran.pop_back();			// ɾ��ԭ����״̬
	for (auto& s : Ntran) {		// ��ԭ����״̬��ת�����ȫ���ĳ��µĽ���״̬ x + y - 2
		for (auto& t : s) {
			for (auto& spt : t) {
				if (spt == x - 1) {
					spt = x + y - 3;
				}
			}
		}
	}
	// ���Ҳ��Զ�����״̬ 0 ���״̬�ϲ���ת�����ȫ�� + (x - 2)��
	for (size_t i = 1; i < y; ++i) {
		Ntran.push_back(rhs.Ntran[i]);
	}
	for (size_t i = x - 1; i < x + y - 2; ++i) {
		for (auto& t : Ntran[i]) {
			for (auto& spt : t) {
				spt += x - 2;
			}
		}
	}
	// ���Ҳ��Զ�����״̬ 0 ��Ϣ�ϲ���ת�����ȫ�� + (x - 2)��
	for (size_t ch = 0; ch < 129; ++ch) {
		for (auto t : rhs.Ntran[0][ch]) {
			Ntran[0][ch].push_back(t + x - 2);
		}
	}
}
// Thompson: ����
void NFA::opt_concat(const NFA& rhs) {	// ����
	size_t x = get_size();
	size_t y = rhs.get_size();
	Ntran.pop_back();			// ɾ��ԭ����״̬
	for (auto s : rhs.Ntran) {	// ���Ҳ��Զ���״̬ȫ���������
		Ntran.push_back(s);
	}
	for (size_t i = x - 1; i < x + y - 1; ++i) {
		for (auto& t : Ntran[i]) {
			for (auto& spt : t) {
				spt += x - 1;
			}
		}
	}
}
// Thompson: �հ�
void NFA::opt_star() {		// �հ�
	NST oldNtran0 = Ntran[0];
	Ntran.pop_front();
	Ntran.push_front(NST());						// �µ���ʼ״̬ 0
	Ntran[0][128].push_back(Ntran.size() - 1);		// ���� 0 ״̬������״̬�� epsilon ת��
	*(Ntran.end() - 1) = oldNtran0;					// ���ý���״̬�������ת������ԭ 0 ״̬��ͬ��
	Ntran.push_back(NST());							// �����µĽ���״̬ x
	(*(Ntran.end() - 2))[128].push_back(Ntran.size() - 1);	// ���� x - 1 ״̬�� x ״̬�� epsilon ת��
}
// Thompson: ���հ�
void NFA::opt_plus() {		// ���հ�
	// �ȼ��� a & a*
	NFA cp = *this;
	cp.opt_star();
	opt_concat(cp);
}
// Thompson: �ʺ�
void NFA::opt_quest() {		// һ������
	// ��һ���ӳ�ʼ״̬������״̬�� epsilon ת��
	Ntran[0][128].push_back(Ntran.size() - 1);
}
// �ϲ�һϵ�� NFA�����η��ؽ���״̬����ţ����� i ���Զ�����Ӧ�Ľ���״̬�洢�ڵ� i ��λ��
vector<size_t> NFA::merge_nfa(const vector<NFA>& nfas) {
	vector<size_t> accepts;
	Ntran.clear();
	Ntran.push_back(NST());	// �½�һ��ֻ�п�ʼ״̬�� NFA
	size_t offset = 1;
	for (auto nfa : nfas) {
		Ntran[0][128].push_back(offset);	// �½�״̬ 0 �����ϲ��Զ����ĳ�ʼ״̬�� epsilon ת��
		size_t size = nfa.get_size();
		for (auto s : nfa.Ntran) {
			Ntran.push_back(s);			// ������ϲ��Զ���������״̬
		}
		for (size_t i = 0; i < size; ++i) {
			for (auto& t : Ntran[i + offset]) {
				for (auto& spt : t) {
					spt += offset;			// ״̬�ż���ƫ��
				}
			}
		}
		offset += size;
		accepts.push_back(offset - 1);		// �������״̬
	}
	return accepts;
}
// ��һ��״̬�� epsilon �հ�
vector<size_t> NFA::epsilon_closure(size_t s) const {
	vector<size_t> tmp{ s };
	return epsilon_closure(tmp);
}
// ��һ��״̬�� epsilon �հ�
vector<size_t> NFA::epsilon_closure(const vector<size_t>& ss) const {
	set<size_t> resSet(ss.begin(), ss.end());	// �� ss ��ʼ�� resSet
	stack<size_t, vector<size_t>> stk(ss);		// �� ss ��ʼ�� stack
	while (!stk.empty()) {
		size_t t = stk.top();
		stk.pop();
		for (size_t u : Ntran[t][128]) {
			if (!resSet.count(u)) {
				resSet.insert(u);
				stk.push(u);
			}
		}
	}
	vector<size_t> res;							// �����ת��Ϊ vector ����Ϊ�˷��㺯������
	for (size_t i : resSet) {					// ע�ⷵ�ص� vector һ�����Ź����
		res.push_back(i);
	}
	return res;
}
// ��һ��״̬�� move �������˺����Ľ������һ�����ϣ��������ظ���Ԫ�أ���ʼ����� epsilon �հ�ʹ�ã�
vector<size_t> NFA::move(const vector<size_t>& ss, char a) const {
	vector<size_t> res;
	vector<size_t> ec = epsilon_closure(ss);
	for (size_t s : ec) {
		for (size_t d : Ntran[s][(size_t)a]) {
			res.push_back(d);
		}
	}
	return res;
}

// �й� DFA �Ķ���

// �úϲ��� NFA ���������״̬����ʼ�� DFA
DFA::DFA(const NFA& nfa, const vector<size_t>& nacn) {

	vector<vector<size_t>> Dstates;					// DFA ��ÿ��״̬��Ӧ NFA ��һ��״̬����
	stack<size_t> unFlaged;							// δ��ǵ� DFA ״̬

	Dstates.push_back(nfa.epsilon_closure(0));		// Dstates �ʼֻ�� epsilon-closure(s0)
	Dtran.push_back(newDST());						// Dtran �����һ��״̬
	unFlaged.push(0);								// ���Ҳ��ӱ��

	while (!unFlaged.empty()) {
		size_t Tidx = unFlaged.top();		// ȡ��һ��δ��� DFA ״̬
		unFlaged.pop();
		size_t Uidx = 0;
		vector<size_t> T = Dstates[Tidx];	// ��Ӧ�� NFA ״̬����
		for (size_t a = 0; a < 128; ++a) {
			vector<size_t> U = nfa.epsilon_closure(nfa.move(T, (char)a));
			if (!U.empty()) {				// ȷʵ��ת��
				bool UinDstates = false;
				for (size_t i = 0; i < Dstates.size(); ++i) {	// U �Ƿ��� Dstates ��
					if (U == Dstates[i]) {	// ��Ϊ�� epsilon �հ��ĺ����Ľ�������Ź���ģ����Կ���ֱ�ӱȽ�
						UinDstates = true;
						Uidx = i;
						break;
					}
				}
				if (!UinDstates) {				// U ���� Dstates ��
					Dstates.push_back(U);		// ���� Dstates
					Dtran.push_back(newDST());	// Dtran �����һ��״̬
					Uidx = Dstates.size() - 1;
					unFlaged.push(Uidx);		// ���Ҳ��ӱ��
				}
				Dtran[Tidx][(size_t)a] = Uidx;
			}
		}
	}
	
	for (size_t i = 0; i < Dstates.size(); ++i) {		// �ж� DFA ״̬�Ƿ��ǽ���״̬
		size_t firstAccept = -1;						// ȡ�����г���ģʽ��Ӧ����״̬
		for (auto s : Dstates[i]) {
			if (nacn[s] != -1) {
				firstAccept = nacn[s] < firstAccept ? nacn[s] : firstAccept;
			}
		}
		accepts.push_back(firstAccept);
	}
}
// ��С�� DFA
//void DFA::minimize() {
//	vector<size_t> sttGroup(get_size());	// ״̬�����ĸ���
//	vector<list<size_t>> grpStates;			// ÿ����������Щ״̬
//	vector<array<size_t, 128>> newTran;
//	size_t maxAccNum = 0;					// ���Ľ���״̬���
//	for (auto n : accepts) {
//		if (n != -1) {
//			maxAccNum = n > maxAccNum ? n : maxAccNum;
//		}
//	}
//	for (size_t i = 0; i < maxAccNum + 2; ++i) {	// �ʼ������ maxAccNum + 2 ����
//													// �ֱ��Ӧ maxAccNum + 1 ����ͬ�Ľ���״̬��
//													// �� 1 ������״̬��
//		grpStates.push_back(list<size_t>());
//	}
//	for (size_t i = 0; i < get_size(); ++i) {
//		if (accepts[i] == -1) {				// �Էǽ���״̬
//			sttGroup[i] = maxAccNum + 1;
//			grpStates[maxAccNum + 1].push_back(i);
//		}
//		else {
//			sttGroup[i] = accepts[i];
//			grpStates[accepts[i]].push_back(i);
//		}
//	}
//	bool modified = false;
//	do {
//		modified = false;
//		for (auto g : grpStates) {
//			if (g.size() == 1) {					// ���������ֻ��һ��״̬�򲻿���
//				continue;
//			}
//			else {
//				for (list<size_t>::iterator s = g.begin(); s != g.end(); ++s) {
//					for (list<size_t>::iterator t = s + 1; t != g.end(); ++t) {
//						for (size_t ch = 0; ch < 128; ++ch) {
//							if (sttGroup[Dtran[*s][ch]] != sttGroup[Dtran[*t][ch]]) {
//
//							}
//						}
//						if (modified) {
//							break;
//						}
//					}
//					if (modified) {
//						break;
//					}
//				}
//			}
//		}
//	} while (modified);
//}
//// ɾ����״̬
//void DFA::delete_dead_states() {
//	for (size_t i = 0; i < get_size(); ++i) {
//		if (accepts[i] != -1) {				// �ǽ���״̬������
//			continue;
//		}
//		else {
//
//		}
//	}
//}


// �ж� ch �Ƿ���������ʽ�б���ת����ַ� 
inline bool need_escape(char ch) {
	if (ch == '\\' || ch == '\"' || ch == '.' || ch == '^' || ch == '$' || ch == '[' || ch == ']'
		|| ch == '*' || ch == '+' || ch == '?' || ch == '{' || ch == '}' || ch == '|' || ch == '/'
		|| ch == '(' || ch == ')') {
		return true;
	}
	else {
		return false;
	}
}

// �ж��Ƿ������������֮ǰû��ʵ������ķ�б�ܣ������ķ�б������Ϊż����
inline bool is_not_escaped(string::const_iterator it, const string& str) {
	if (it == str.begin()) {
		return true;
	}
	else {
		it--;
		int backSlashCount = 0;
		for (; it != str.begin(); --it) {
			if (*it == '\\') {
				++backSlashCount;
			}
			else {
				break;
			}
		}
		if (backSlashCount % 2 == 0) {
			return true;
		}
		else {
			return false;
		}
	}
}

// ��һ���з������ÿո���Ʊ���ָ���������
pair<string, string> split_by_blank(const string& str) {
	bool flag = true;
	string name, expr;
	for (auto c : str) {
		if (flag) {
			if (c != '\t') {
				name.push_back(c);
			}
			else {
				flag = false;
			}
		}
		else {
			if (c != '\t') {
				expr.push_back(c);
			}
			else {
				if (expr.empty()) continue;
				else break;
			}
		}
	}
	return pair<string, string>(name, expr);
}

// ��������ʽ�ַ��������ɷ��ŵ����У������������ź�����
vector<char> deal_brkt_qt(const string& exp) {

	vector<char> res;
	string inBracket;							// ��������ʵ�ʵ��ַ�
	bool bracketFlag = false;					// ���������
	bool notBracketFlag = false;				// ��������ţ�������ʽ��
	bool quotationFlag = false;					// �������

	for (string::const_iterator it = exp.begin(); it != exp.end(); ++it) {
		// �������ţ��������������ڲ�����
		if (!quotationFlag && !bracketFlag && !notBracketFlag && *it == '\"' && is_not_escaped(it, exp)) {
			quotationFlag = true;
		}
		// ���Ž���
		else if (quotationFlag && !bracketFlag && !notBracketFlag && *it == '\"' && is_not_escaped(it, exp)) {
			quotationFlag = false;
		}
		// �����ڲ�
		else if (quotationFlag) {
			res.push_back(*it);
		}
		// ����������
		else if (!bracketFlag && !notBracketFlag && *it == '[' && is_not_escaped(it, exp)) {
			if (*(it + 1) != '^') {
				bracketFlag = true;
			}
			else {
				++it;
				notBracketFlag = true;
			}
			inBracket.clear();
		}
		// �����Ž���
		else if (bracketFlag && *it == ']' && is_not_escaped(it, exp)) {
			bracketFlag = false;
			res.push_back(to_operator('('));
			bool first = true;
			for (auto c = inBracket.begin(); c != inBracket.end(); ++c) {
				if (first) {					// ���˵�һ���������֮��� |
					first = false;
				}
				else {
					res.push_back(to_operator('|'));
				}
				if (*c == '\\') {				// �����ת��ģ�ת��������忴��
					if (need_escape(*(++c))) {
						res.push_back(*c);
					}
					else {
						switch (*c) {
						case 'a':
							res.push_back('\a');
							break;
						case 'b':
							res.push_back('\b');
							break;
						case 'f':
							res.push_back('\f');
							break;
						case 'n':
							res.push_back('\n');
							break;
						case 'r':
							res.push_back('\r');
							break;
						case 't':
							res.push_back('\t');
							break;
						case 'v':
							res.push_back('\v');
							break;
						}
					}
				}
				else {
					res.push_back(*c);
				}
			}
			res.push_back(to_operator(')'));
		}
		// �����Ž�����������ʽ��
		else if (notBracketFlag && *it == ']' && is_not_escaped(it, exp)) {
			notBracketFlag = false;
			res.push_back(to_operator('('));
			bool allChar[128];					// �����ļ��㣺ԭ���������ַ���Ӧλ�õ� bool ֵ�� 0
			for (size_t i = 0; i < 128; ++i) {
				allChar[i] = true;
			}
			for (auto c = inBracket.begin(); c != inBracket.end(); ++c) {
				if (*c == '\\') {
					if (need_escape(*(++c))) {
						allChar[(size_t)(*c)] = false;
					}
					else {
						switch (*c) {
						case 'a':
							allChar[(size_t)'\a'] = false;
							break;
						case 'b':
							allChar[(size_t)'\b'] = false;
							break;
						case 'f':
							allChar[(size_t)'\f'] = false;
							break;
						case 'n':
							allChar[(size_t)'\n'] = false;
							break;
						case 'r':
							allChar[(size_t)'\r'] = false;
							break;
						case 't':
							allChar[(size_t)'\t'] = false;
							break;
						case 'v':
							allChar[(size_t)'\v'] = false;
							break;
						}
					}
				}
				else {
					allChar[(size_t)(*c)] = false;
				}
			}
			bool first = true;
			for (size_t i = 0; i < 128; ++i) {
				if (allChar[i]) {
					if (first) {
						first = false;
					}
					else {
						res.push_back(to_operator('|'));
					}
					char ch = (char)i;
					if (need_escape(ch)) {			// ��Ҫת��ģ����ת�����
						res.push_back(ch);
					}
					else {
						res.push_back(ch);
					}
				}
			}
			res.push_back(to_operator(')'));
		}
		// ���������ڲ�����䵽 inBracket
		else if (bracketFlag || notBracketFlag) {
			if (*it != '-' || *(it + 1) == ']') {
				inBracket.push_back(*it);
			}
			else {		// ��������ļ��ţ���ĳĳ�ַ���ĳĳ�ַ�
				for (char ch = *(it - 1) + 1; ch < *(it + 1); ++ch) {
					inBracket.push_back(ch);
				}
			}
		}
		else {
			if (*it == '\\') {
				if (need_escape(*(++it))) {
					res.push_back(*it);
				}
				else {
					switch (*it) {
					case 'a':
						res.push_back('\a');
						break;
					case 'b':
						res.push_back('\b');
						break;
					case 'f':
						res.push_back('\f');
						break;
					case 'n':
						res.push_back('\n');
						break;
					case 'r':
						res.push_back('\r');
						break;
					case 't':
						res.push_back('\t');
						break;
					case 'v':
						res.push_back('\v');
						break;
					}
				}
			}
			else {
				if (need_escape(*it)) {
					res.push_back(to_operator(*it));
				}
				else {
					res.push_back(*it);
				}
			}
		}
	}
	return res;
}

// ���� map ����������
vector<char> explain_defs(const vector<char>& rgx, const map<string, vector<char>>& mp) {
	vector<char> res;
	bool braceFlag = false;
	string defName;			// �õ������������
	for (vector<char>::const_iterator it = rgx.begin(); it != rgx.end(); ++it) {
		if (*it == to_operator('{') && !braceFlag) {
			braceFlag = true;
			defName.clear();
		}
		else if (*it == to_operator('}') && braceFlag) {
			braceFlag = false;
			if (mp.count(defName) == 0) {}
			else {
				res.push_back(to_operator('('));
				for (auto c : mp.at(defName)) {
					res.push_back(c);
				}
				res.push_back(to_operator(')'));
			}
		}
		else if (braceFlag) {
			defName.push_back(*it);
		}
		else {
			res.push_back(*it);
		}
	}
	return res;
}

// ������
vector<char> deal_dot(const vector<char>& seq) {
	vector<char> res;
	for (vector<char>::const_iterator it = seq.begin(); it != seq.end(); ++it) {
		if (*it == to_operator('.')) {			// ���������
			res.push_back(to_operator('('));
			bool first = true;
			for (unsigned i = 0; i < 128; ++i) {
				if (i!='\n') {
					if (first) {
						first = false;
						res.push_back((char)i);
					}
					else {
						res.push_back(to_operator('|'));
						res.push_back((char)i);
					}
				}
			}
			res.push_back(to_operator(')'));
		}
		else {
			res.push_back(*it);
		}
	}
	return res;
}

// ������ӷ��� to_operator('&')����������ʽ��
vector<char> seq_to_infix(const vector<char>& seq) {
	vector<char> res;
	bool pre = false;
	for (vector<char>::const_iterator it = seq.begin(); it != seq.end(); ++it) {
		if (pre) {
			if (!is_optr(*it) || to_char(*it) == '(') {
				res.push_back(to_operator('&'));
				res.push_back(*it);
			}
			else {
				res.push_back(*it);
			}
		}
		else {
			res.push_back(*it);
		}
		if (!is_optr(*it) || to_char(*it) == ')' || to_char(*it) == '*' || to_char(*it) == '+' || to_char(*it) == '?') {
			pre = true;
		}
		else {
			pre = false;
		}
	}
	return res;
}

// ����׺��������ʽת��Ϊ��׺��������ʽ
vector<char> infix_to_suffix(const vector<char>& seq) {
	vector<char> res;
	stack<char> optrStk;
	map<char, int> priority{
		{'*', 7},
		{'+', 7},
		{'?', 7},
		{'&', 5},
		{'|', 3}
	};
	for (vector<char>::const_iterator it = seq.begin(); it != seq.end(); ++it) {
		if (!is_optr(*it)) {
			res.push_back(*it);
		}
		else {
			if (*it == to_operator('(')) {
				optrStk.push(*it);
			}
			else if (*it == to_operator(')')) {
				while (!optrStk.empty() && to_char(optrStk.top()) != '(') {
					res.push_back(optrStk.top());
					optrStk.pop();
				}
				if (!optrStk.empty()) {
					optrStk.pop();
				}
			}
			else {
				if (optrStk.empty() || to_char(optrStk.top()) == '(') {
					optrStk.push(*it);
				}
				else {
					if (priority[to_char(*it)] > priority[to_char(optrStk.top())]) {
						optrStk.push(*it);
					}
					else {
						while (!optrStk.empty() && priority[to_char(*it)] <= priority[to_char(optrStk.top())]) {
							res.push_back(optrStk.top());
							optrStk.pop();
						}
						optrStk.push(*it);
					}
				}
			}
		}
	}
	while (!optrStk.empty()) {
		res.push_back(optrStk.top());
		optrStk.pop();
	}
	return res;
}

// ����׺���ʽת�� NFA
NFA suffix_to_nfa(const vector<char>& seq) {
	stack<NFA> s;
	for (vector<char>::const_iterator it = seq.begin(); it != seq.end(); ++it) {
		if (!is_optr(*it)) {
			s.push(NFA(to_char(*it)));
		}
		else {
			if (to_char(*it) == '|') {
				NFA rhs = s.top();
				s.pop();
				NFA lhs = s.top();
				s.pop();
				lhs.opt_union(rhs);
				s.push(lhs);
			}
			else if (to_char(*it) == '&') {
				NFA rhs = s.top();
				s.pop();
				NFA lhs = s.top();
				s.pop();
				lhs.opt_concat(rhs);
				s.push(lhs);
			}
			else if (to_char(*it) == '*') {
				NFA lhs = s.top();
				s.pop();
				lhs.opt_star();
				s.push(lhs);
			}
			else if (to_char(*it) == '+') {
				NFA lhs = s.top();
				s.pop();
				lhs.opt_plus();
				s.push(lhs);
			}
			else if (to_char(*it) == '?') {
				NFA lhs = s.top();
				s.pop();
				lhs.opt_quest();
				s.push(lhs);
			}
			else {
			}
		}
	}
	return s.top();
}

void gen_code(ofstream& ofs, const DFA& dfa, const vector<string>& actions) {
	const vector<size_t> accepts = dfa.get_accepts();
	ofs << "unsigned tran[][128] = {\n";
	for (size_t i = 0; i < dfa.get_size(); ++i) {
		ofs << '\t' << "{\t";
		for (size_t ch = 0; ch < 128; ++ch) {
			ofs << dfa.get_tran(i, ch);
			if (ch != 127) {
				ofs << ',';
			}
			ofs << '\t';
		}
		ofs << '}';
		if (i != dfa.get_size() - 1) {
			ofs << ',';
		}
		ofs << '\n';
	}
	ofs << "};\n\n";
	ofs << "int yylex() {\n";
	ofs << '\t' << "while (*p) {\n";
	ofs << '\t' << '\t' << "if (*p == '\\n')\t++line;\n";
	ofs << '\t' << '\t' << "char *forward = p;\n";
	ofs << '\t' << '\t' << "unsigned lastAccept = -1;\n";
	ofs << '\t' << '\t' << "unsigned stateNum = 0;\n";
	ofs << '\t' << '\t' << "for (int i = 0; *forward; ++i) {\n";
	ofs << '\t' << '\t' << '\t' << "stateNum = tran[stateNum][(unsigned)*forward];\n";
	ofs << '\t' << '\t' << '\t' << "if (stateNum == -1) {\n";
	ofs << '\t' << '\t' << '\t' <<'\t' << "break;\n";
	ofs << '\t' << '\t' << '\t' << "}\n";
	ofs << '\t' << '\t' << '\t' << "else if (stateNum == ";
	bool first = true;
	for (size_t i = 0; i < accepts.size(); ++i) {
		if (accepts[i] != -1) {
			if (first) {
				first = false;
			}
			else {
				ofs << " || stateNum == ";
			}
			ofs << i;
		}
	}
	ofs << ") {\n";
	ofs << '\t' << '\t' << '\t' << '\t' << "lastAccept = stateNum;\n";
	ofs << '\t' << '\t' << '\t' << "}\n";
	ofs << '\t' << '\t' << '\t' << "++forward;\n";
	ofs << '\t' << '\t' << "}\n";
	ofs << '\t' << '\t' << "yytextlen = forward - p;\n";
	ofs << '\t' << '\t' << "int i = 0;\n";
	ofs << '\t' << '\t' << "for (; i < yytextlen; ++i) {\n";
	ofs << '\t' << '\t' << '\t' << "yytext[i] = p[i];\n";
	ofs << '\t' << '\t' << "}\n";
	ofs << '\t' << '\t' << "yytext[i] = '\\0';\n";
	ofs << '\t' << '\t' << "p = forward;\n";
	ofs << '\t' << '\t' << "switch (lastAccept) {\n";
	for (size_t i = 0; i < accepts.size(); ++i) {
		if (accepts[i] != -1) {
			ofs << '\t' << '\t' << "case " << i << ":\n";
			ofs << '\t' << '\t' << '\t' << actions[accepts[i]] << '\n';
			ofs << '\t' << '\t' << '\t' << "break;\n";
		}
	}
	ofs << '\t' << '\t' << "}\n";
	ofs << '\t' << "}\n";
	ofs << '\t' << "printf(\"unexpected eof\");\n";
	ofs << '\t' << "return 0;\n";
	ofs << "}\n\n";
}

// ����lex�ļ������ɴʷ������������ش����к�
int ParseLexFile(ifstream& ifs, ofstream& ofs) {

	vector<string> names;				// ������-���֣��붨��������Ӧ��
	vector<string> definitions;			// ������-���壨������������Ӧ��
	vector<string> rules;				// �����붯��������Ӧ��
	vector<string> actions;				// �����������������Ӧ��
	string toCopy;						// ��������
	string subRout;						// �ӳ��򲿷�

	// �����ļ�������

	{
		vector<string> allLines;							// �洢������
		string line;										// ��ǰ��
		int lineCount = 0;									// �к�
		vector<int> lineTypes;								// ����е����ԣ�
		int line_type = 0;									// 1��������ʽ����
															// 2�����򲿷�
															// 3����������
															// 4���ӳ��򲿷�

		while (getline(ifs, line)) {						// �����ҳ����� %%�����ļ��ֳ���������
			++lineCount;
			// if (line.empty()) continue;
			allLines.push_back(line);
			lineTypes.push_back(line_type);
			if (line._Equal("%%")) {
				if (line_type == 0) {
					line_type = 2;
				}
				else if (line_type == 2) {
					line_type = 4;
					lineTypes.back() = 0;
				}
				else
					return lineCount;
			}
		}

		bool regxFlag = true;								// �Ե�һ�� %% ֮ǰ�Ĳ��֣������� %{ �� %} ֮��Ĵ���
		line_type = 1;
		for (size_t i = 0; i < lineTypes.size(); ++i) {
			if (regxFlag && lineTypes[i] == 2) {
				lineTypes[i - 1] = 0;
				regxFlag = false;
				continue;
			}
			if (regxFlag) {
				lineTypes[i] = line_type;
				if (allLines[i]._Equal("%{")) {
					line_type = 3;
					lineTypes[i] = 0;
				}
				else if (allLines[i]._Equal("%}") && line_type == 3) {
					line_type = 1;
					lineTypes[i] = 0;
				}
			}
		}

		for (size_t i = 0; i < allLines.size(); ++i) {
			switch (lineTypes[i]) {
			case 1:
				names.push_back(split_by_blank(allLines[i]).first);
				definitions.push_back(split_by_blank(allLines[i]).second);
				break;
			case 2:
				rules.push_back(split_by_blank(allLines[i]).first);
				actions.push_back(split_by_blank(allLines[i]).second);
				break;
			case 3:
				toCopy.append(allLines[i]);
				toCopy.push_back('\n');
				break;
			case 4:
				subRout.append(allLines[i]);
				subRout.push_back('\n');
				break;
			}
		}

		// ��������д�����嶯����������Ϊ�յģ���ƴ������
		vector<string>::iterator last1 = rules.begin(), last2 = actions.begin();
		for (auto it1 = rules.begin(), it2 = actions.begin(); it1 != rules.end() && it2 != actions.end(); ++it1, ++it2) {
			if (*it1 == "") {
				last2->append(*it2);
				it1 = rules.erase(it1);
				it2 = actions.erase(it2);
				it1--;
				it2--;
			}
			last1 = it1;
			last2 = it2;
		}
	}

	// ������͹������������

	vector<vector<char>> defsSeq;
	for (auto d : definitions) {
		defsSeq.push_back(deal_brkt_qt(d));
	}
	vector<vector<char>> rulesSeq;
	for (auto r : rules) {
		rulesSeq.push_back(deal_brkt_qt(r));
	}

	// �����������е�Ƕ�׶��壨����һ����������ǰһ����������ݣ����������Ƶ������ӳ��

	map<string, vector<char>> mapNameToDef;
	mapNameToDef.insert(pair<string, vector<char>>(names[0], defsSeq[0]));	// ���ȼ����һ��
	for (size_t i = 1; i < defsSeq.size(); ++i) {							// �����Ķ���ݹ�ʹ���ѽ�����ӳ��
		mapNameToDef.insert(pair<string, vector<char>>(names[i], explain_defs(defsSeq[i], mapNameToDef)));
	}

	// ����������ʽ�е�������

	for (auto& pd : rulesSeq) {
		vector<char> npd = explain_defs(pd, mapNameToDef);
		pd = npd;
	}

	// ��������ʽȫ��ת��Ϊ NFA

	vector<NFA> nfas;
	for (auto r : rulesSeq) {
		nfas.push_back(suffix_to_nfa(infix_to_suffix(seq_to_infix(deal_dot(r)))));
	}

	// �ϲ����� NFA������� NFA �ͽ���״̬��ű�

	NFA mergedNFA;
	vector<size_t> NAcceptedStates = mergedNFA.merge_nfa(nfas);
	vector<size_t> Naccept(mergedNFA.get_size());
	for (auto& acn : Naccept) {
		acn = -1;
	}
	for (size_t i = 0; i < NAcceptedStates.size(); ++i) {
		Naccept[NAcceptedStates[i]] = i;
	}

	// �� NFA ת��Ϊ DFA���� DFA ��С��

	DFA dfa(mergedNFA, Naccept);
	//dfa.minimize();
	//dfa.delete_dead_states();
	
	// ���� DFA ���ɴʷ�������Դ�ļ�

	ofs << toCopy << '\n';
	gen_code(ofs, dfa, actions);
	ofs << subRout << '\n';

	return 0;
}
