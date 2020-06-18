#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <fstream>
#include <set>
#include <queue>
#include <stack>
using namespace std;


//数据结构定义

//LR1项的结构
struct Lr1Item {
	string leftp;//产生式左部
	string rightp[10];//产生式右部
	set<string> predict;//LR1项的预测符
	int numofrights;//右部符号个数
	int dot; //右部中圆点在第几个符号前面
	bool operator <(const Lr1Item& item) const//重载<，用于比较两个LR1项
	{
		if (strcmp(leftp.c_str(), item.leftp.c_str()) == 0)
		{
			int i = 0;
			while (i < 10)
			{
				if (strcmp(rightp[i].c_str(), item.rightp[i].c_str()) != 0)
					return rightp[i] < item.rightp[i];//产生式右部不同
				i++;

			}
			if (predict == item.predict)
				return dot < item.dot;//预测符相同但圆点位置不同           
			else
				return predict < item.predict;//预测符不同
		}
		else
			return leftp < item.leftp;//左部不同
	}
};

//LR1自动机的状态的结构
struct pdaState {
	int name;//状态名，如I0
	int numofitems;//该状态拥有的LR1项数目
	set<Lr1Item> items;//用于存放状态中的等价LR1项
	bool operator < (const pdaState& state) const //重载<，用于比较状态中的LR1项
	{
		return items < state.items;
	}
};

//LR1自动机从一个状态下推到另一个状态的边的结构
struct pdaEdge {
	string symbol;//遇到的符号
	int startState;//原状态名
	int endState;//后来的状态名
	bool operator < (const pdaEdge& edge) const//重载<，用于比较边
	{
		if (startState == edge.startState)
		{
			if (endState == edge.endState)
				return symbol < edge.symbol;
			else
				return endState < edge.endState;
		}
		else
			return startState < edge.startState;

	}
};

//整个LR1自动机的结构
struct Lr1Pda {
	int numofstates;//整个自动机的状态数
	int numofedges;//整个自动机的边的数量
	set<pdaState> states;//存放自动机的所有状态
	set<pdaEdge> edges;//存放自动机所有边
};

//LR1分析表的结构
struct table {
	vector<string> symbol;//一个状态的边上符号的集合
	vector<int> nextState;//边上所指后继状态的集合
};

struct op {//存储操作符
	string dest;//左操作符，还是右操作符
	int level;//level越大，操作符优先级越高
	string name;//操作符名称
}oper[10];


//存放终结符
set<string> terminal;
//存放非终结符
set<string> nonterminal;

vector<Lr1Item> Item;//全局变量，保存最初的LR1项
pdaState I0;
Lr1Pda pda;//整个自动机


bool isNonterminal(string str, set<string> nonterminal)//判断是否为非终结符,根据参数也可判断是否为终结符
{
	set<string>::iterator it;
	for (it = nonterminal.begin(); it != nonterminal.end(); it++)
	{
		if (str == *it)
			return true;
	}
	return false;
}

//求first集 symbols_after_dot为与圆点隔一个符号的符号，existedPredict为当前LR1项已有的展望符
void analysisOfFirst(string symbols_after_dot, set<string> existedPredict, set<string>& predict_to_be_added)
{
	if (isNonterminal(symbols_after_dot, terminal))//如果是终结符
	{
		predict_to_be_added.insert(symbols_after_dot);
		return;
	}
	else if (symbols_after_dot == "")//如果圆点已经在最后一个符号前面
	{
		set<string>::iterator it;
		for (it = existedPredict.begin(); it != existedPredict.end(); it++)
		{
			predict_to_be_added.insert(*it);
		}
		return;
	}
	else//如果是非终极符
	{
		stack<string> nonTer;//用栈存储不断加入待求first集的非终结符，即若有A->.BC,C->DE,求first(C)==first(D)∪...,D比C先出栈
		set<string> extendedTer;//存放已经被求过first集的非终结符
		nonTer.push(symbols_after_dot);
		while (!nonTer.empty())
		{
			string leftpart = nonTer.top();//以该非终结符为左部
			nonTer.pop();
			extendedTer.insert(leftpart);//实际上放后面比较合理？

			vector<Lr1Item>::iterator it;
			for (it = Item.begin(); it != Item.end(); it++)
			{
				if ((*it).leftp == leftpart)//当遍历到以该非终结符为左部的产生式(LR1项目)时
				{
					string rightpart = (*it).rightp[0];//取右部第一个符号
					if (isNonterminal(rightpart, terminal))//如果这个符号为终结符，则加入其为展望符
					{
						predict_to_be_added.insert(rightpart);
					}
					else if (extendedTer.count(rightpart) == 0)//如果这个符号为非终结符且未求过first集
					{
						nonTer.push(rightpart);//存放于栈顶，在下轮while中求first集
					}

				}
			}
		}

	}


}

void inputyacc()
{
	fstream input;
	string* buff = new string[BUFSIZ];
	input.open("minic.y", ios::in);
	char* getchar = new char[BUFSIZ];
	input.getline(getchar, BUFSIZ);
	int line = 0;//表示读到的行数
	//第一部分将%{和}%之间的放入到缓冲区中,getline每次获取文档中的一行代码	
	while (!(getchar[0] == '%' && getchar[1] == '}'))
	{
		line++;
		input.getline(getchar, BUFSIZ);
	}
	//cout << "ok" << endl;
	//将定义部分保存起来
	int opleftlevel = 0;
	int opnum = 0;
	line++;
	input.getline(getchar, BUFSIZ);
	while (!(getchar[0] == '%' && getchar[1] == '%'))
	{
		if (getchar[0] == '%')
		{
			char* star = strtok(getchar, "%");
			char* tok = strtok(star, "\t ");//第一个为制表符
			if (strcmp("token", tok) == 0)//token序列后的符合为终结符
			{
				tok = strtok(NULL, "\t ");
				while (tok != NULL)
				{
					terminal.insert(tok);
					tok = strtok(NULL, "\t ");
				}
			}
			else if (strcmp("left", tok) == 0)
			{
				tok = strtok(NULL, "\t ");
				while (tok != NULL && tok[0] != '/')
				{
					oper[opnum].name.operator=(tok);
					oper[opnum].level = opleftlevel;
					oper[opnum].dest.operator=("left");
					opnum++;
					tok = strtok(NULL, "\t ");
				}
				opleftlevel++;
			}
			else  if (strcmp("nonassoc", tok) == 0)
			{

				tok = strtok(NULL, "\t ");
				while (tok != NULL && tok[0] != '/')
				{
					tok = strtok(NULL, "\t ");
				}
				opleftlevel++;
			}
			else  if (strcmp("union", tok) == 0)
			{
			}
			else if (strcmp("type", tok) == 0)
			{

			}
		}
		line++;
		input.getline(getchar, BUFSIZ);
	}

	//将产生式部分处理
	line++;
	input.getline(getchar, BUFSIZ);
	while (!(getchar[0] == '%' && getchar[1] == '%'))
	{
		char* seg = strtok(getchar, "\t ");
		//首先是读取该产生式的左部，其次是冒号，然后一个右部，竖号，然后另一个右部，其实是另一个产生式，最后是分号，表示结束
		//同时产生式中出现的非终极符，需要存入到
		while (seg == NULL)
		{
			line++;
			input.getline(getchar, BUFSIZ);
			seg = strtok(getchar, "\t ");
		}
		//从不为空行的开始
		string left;
		Lr1Item temp;//临时产生式

		left = string(seg);
		temp.leftp = left;
		nonterminal.insert(left);//利用set集合的性质，不允许有重复的出现，避免了去判断该符号是否已经存在
		seg = strtok(NULL, "\t ");//去除“：”
		seg = strtok(NULL, "\t ");
		int ptr = 0;
		while (1)
		{
			//	cout << "can I in" << endl;
			//此间会出现：，|等符号，需要将其识别出来  还有的就是加入到终极符和非终极符中   
			if (strcmp(seg, "|") == 0 || strcmp(seg, ";") == 0)
			{
				if (ptr > 0)
				{
					cout << temp.leftp << "->";
					int j = 0;
					while (j < 10)
					{
						if (strlen(temp.rightp[j].c_str()) == 0)
							break;
						cout << temp.rightp[j] << " ";
						j++;
					}
					temp.dot = 0;
					temp.numofrights = j;
					cout << temp.numofrights << endl;
					Item.push_back(temp);
					int i = 0;
					while (i < 10)
					{

						cout << temp.rightp[i] << " ";
						temp.rightp[i] = "";
						i++;
					}
					cout << endl;
				}

				ptr = 0;
				if (strcmp(seg, ";") == 0)
					break;
				seg = strtok(NULL, "\t ");
				if (seg == NULL)
					seg = "eplison";
			}
			temp.rightp[ptr].operator=(seg);
			if (string(seg).length() > 1)
			{
				if (!isNonterminal(string(seg), terminal))
				{
					if (!isNonterminal(string(seg), nonterminal))
						nonterminal.insert(string(seg));
				}
			}
			ptr++;
			seg = strtok(NULL, "\t ");
			while (seg == NULL)
			{
				line++;
				input.getline(getchar, BUFSIZ);
				seg = strtok(getchar, "\t ");
			}
		}
		line++;
		input.getline(getchar, BUFSIZ);
	}
	//查看是否到文件结尾，若是结束否则读入缓冲区
	//米用的东西，不再读入
}


void analysisOfLr1()//LR1分析
{
	//初始化整个pda
	pda.numofstates = 0;
	pda.numofedges = 0;

	//初始化第0个状态
	int nameofstate = 0;
	pdaState init, temp; //temp状态用于临时存放
	Lr1Item tempItem;//用于后续在一个状态内对待扩展LR1项扩展时的临时存放

	//初始化第一个LR1项
	Lr1Item item;
	item.leftp.operator=("S'");
	item.rightp[0].operator=("program");
	item.predict.insert("#");
	item.numofrights = 1;
	item.dot = 0;
	Item.push_back(item);
	//初始化第一个LR1项后更新初始状态inte为(S' -> program, #)
	init.name = nameofstate;
	nameofstate++;//用于给下一个状态命名
	init.items.insert(item);
	init.numofitems = 1;

	//创建一个状态队列,存放尚未扩展等价项的状态
	queue<pdaState> stateQueue;
	stateQueue.push(init);//init状态入队列

	//创建一个已扩展等价项的状态集合，表示该集合中的状态都已扩展完成
	set<pdaState> extendedState;

	//开始扩展等价项目
	while (!stateQueue.empty())
	{
		temp = stateQueue.front();
		stateQueue.pop();
		extendedState.insert(temp);    //实际上应该放在更靠后的位置更合理？

		//创建一个LR1项队列，存放尚未扩展等价项的LR1项
		queue<Lr1Item> itemQueue;
		//创建一个已扩展等价项的LR1项集合，表示该集合中的LR1项均已扩展
		set<Lr1Item> extendedItem;

		//使用迭代器遍历状态temp中的所有LR1项
		set<Lr1Item> ::iterator vt;
		//对temp状态中的每个需要扩展等价项的LR1项放入带扩展队列
		for (vt = temp.items.begin(); vt != temp.items.end(); vt++)
		{
			if (isNonterminal((*vt).rightp[(*vt).dot], nonterminal))//判断每个LR1项中右部圆点之后的符号是否为非终结符
			{
				itemQueue.push((*vt));//该项目需要扩展，放入待扩展项目队列
			}
		}

		//开始对一个状态内所有LR1项扩展
		while (!itemQueue.empty())
		{
			tempItem = itemQueue.front();
			itemQueue.pop();

			int dot = tempItem.dot;
			string str = tempItem.rightp[dot];//取当前待扩展LR1项圆点后的符号

			extendedItem.insert(tempItem);        //这行也是也许应该放后面在事实上更合理，但逻辑上都放这里有好处？

			if (dot + 1 <= tempItem.numofrights)//如果当前圆点后还有符号
			{
				if (isNonterminal(tempItem.rightp[dot], nonterminal))//如果圆点后符号为非终结符，则进行等价项扩展
				{
					set<string> newPredict;//用于存放当前LR1项新加入的展望符
					string symbol;//当前LR1项中该被求first集的符号，即S->.ABα，sybmol=B，求first(B)或者first(Bα)
					symbol = tempItem.rightp[tempItem.dot + 1];
					analysisOfFirst(symbol, tempItem.predict, newPredict);

					vector<Lr1Item>::iterator it;//设置迭代器遍历读入yacc后的保存的所有产生式
					for (it = Item.begin(); it != Item.end(); it++)
					{
						if ((*it).leftp == tempItem.rightp[dot])//如果某个产生式的左部为当前LR1项待约非终结符
						{
							Lr1Item toolItem;//创建一个"工具"LR1项，临时将那个产生式的信息赋给它
							toolItem.leftp = (*it).leftp;
							toolItem.numofrights = (*it).numofrights;
							toolItem.dot = (*it).dot;
							for (int i = 0; i < 10; i++)
							{
								toolItem.rightp[i] = (*it).rightp[i];
							}
							set<string>::iterator pre;
							for (pre = newPredict.begin(); pre != newPredict.end(); pre++)
							{
								toolItem.predict.insert((*pre));
							}

							temp.items.insert(toolItem);//临时状态中新增了一个扩展的等价项

							int originalsize = extendedItem.size();
							extendedItem.insert(toolItem);//toolItem加入已扩展LR1项的集合，这里并不是真的已经完成扩展
							int latersize = extendedItem.size();
							if (originalsize + 1 == latersize)//若相等，说明toolItem尚未扩展，上面提前加入已扩展集合，在之后的队列itemQueue扩展
							{
								itemQueue.push(toolItem);
							}
						}
					}
				}
			}

		}

		pda.states.insert(temp);//在完整的自动机中加入完成扩展等价项的temp状态
		if (pda.numofstates == 0)
			I0 = temp;
		pda.numofstates++;

		//求各LR1项圆点后的符号，构成集合
		set<string> tag;//符号集合
		set<Lr1Item>::iterator usedfortemp;
		for (usedfortemp = temp.items.begin(); usedfortemp != temp.items.end(); usedfortemp++)
		{
			if ((*usedfortemp).rightp[(*usedfortemp).dot] != "")
				tag.insert((*usedfortemp).rightp[(*usedfortemp).dot]);
		}

		//开始对状态进行下推，对状态temp下推，temp与后继状态之间添加对应的pdaEdge对象
		set<string>::iterator edge;
		for (edge = tag.begin(); edge != tag.end(); edge++)
		{
			pdaEdge tempedge;
			pdaState tempstate;//作为新后继状态的中间临时状态
			set<Lr1Item>::iterator vitem;
			int itemcount = 0;//对后继状态中LR1项计数

			for (vitem = temp.items.begin(); vitem != temp.items.end(); vitem++)
			{
				if ((*edge) == (*vitem).rightp[(*vitem).dot])//如果边上的符号等于某LR1项圆点后的符号
				{
					Lr1Item yaitem;
					yaitem.leftp = (*vitem).leftp;
					yaitem.numofrights = (*vitem).numofrights;//后继状态中的LR1项左部、右部个数照抄前面状态的LR1项
					yaitem.dot = (*vitem).dot + 1;//后继状态中圆点向后一个符号
					for (int j = 0; j < 10; j++)
					{
						yaitem.rightp[j] = (*vitem).rightp[j];//照抄右部
					}
					set<string>::iterator iterator;
					for (iterator = (*vitem).predict.begin(); iterator != (*vitem).predict.end(); iterator++)
					{
						yaitem.predict.insert((*iterator));//照抄展望符
					}

					tempstate.items.insert(yaitem);//后继状态中插入该LR1项
					itemcount++;
				}
			}

			tempstate.name = nameofstate;//给后继状态命名
			tempstate.numofitems = itemcount;//明确目前后继状态中LR1项数目
			tempedge.symbol = (*edge);//明确进入后继状态时所遇到的符号
			tempedge.startState = temp.name;


			int orisize = extendedState.size();
			extendedState.insert(tempstate);
			int latsize = extendedState.size();
			if (orisize + 1 == latsize)//新后继状态
			{
				tempstate.name = nameofstate;//给后继状态命名
				tempedge.endState = tempstate.name;
				nameofstate++;
				pda.edges.insert(tempedge);
				stateQueue.push(tempstate);

			}
			else//后继状态是自己或别的已经存在的状态
			{
				set<pdaState>::iterator it_state;//遍历已扩展状态集合
				for (it_state = extendedState.begin(); it_state != extendedState.end(); it_state++)
				{
					bool flag = true;
					if (tempstate.items.size() == (*it_state).items.size())//如果找到LR1项数目相等的已扩展状态
					{
						int num = tempstate.items.size();
						Lr1Item* newitem = new Lr1Item[num];
						set<Lr1Item>::iterator it_lr1;
						int k = 0;
						for (it_lr1 = tempstate.items.begin(); it_lr1 != tempstate.items.end(); it_lr1++)
						{
							newitem[k] = (*it_lr1);//将当前临时后继状态中的所有LR1项指针给newitem[]
							k++;
						}

						set<Lr1Item>::iterator it_temp;
						int m = 0;
						for (it_temp = (*it_state).items.begin(); it_temp != (*it_state).items.end(); it_temp++)//遍历找到的已扩展状态中的LR1项
						{
							if (newitem[m] < (*it_temp) || (*it_temp) < newitem[m])//如果存在LR1项不同（顺序是一致的）
							{
								flag = false;
							}

							m++;
						}
						if (flag)//如果所有LR1项一致，则当前边指向的后继状态为已经存在的
						{
							tempedge.endState = (*it_state).name;
							pda.edges.insert(tempedge);
						}
					}
				}//回到for循环，继续在已扩展状态集合中找与tempstate状态所有LR1项都相同的状态
			}

			pda.numofedges++;

		}

	}

}

table* analysisOfTable()//构建LR1分析表
{
	ofstream output("../Debug/table.txt");

	table* Lr1table = new table[pda.numofstates];//假设当成每个状态含一张表
	set<pdaEdge>::iterator edge = pda.edges.begin();
	for (edge; edge != pda.edges.end(); edge++)
	{
		Lr1table[(*edge).startState].symbol.push_back((*edge).symbol);
		Lr1table[(*edge).startState].nextState.push_back((*edge).endState);
	}

	//开始输出分析表,间隔一个制表符
	cout << "State" << "        ";
	output << "State" << "      ";

	set<string>::iterator it;//先画好action部分的属性
	for (it = terminal.begin(); it != terminal.end(); it++)
	{
		cout << (*it) << "      ";
		output << (*it) << "        ";
	}
	cout << "#" << "        ";
	output << "#" << "         ";

	for (it = nonterminal.begin(); it != nonterminal.end(); it++)//再画Goto部分的属性
	{
		cout << (*it) << "      ";
		output << (*it) << "        ";
	}
	cout << endl;
	output << endl;

	//开始输出每个状态的分析表，一行一个状态
	for (int i = 0; i < pda.numofstates; i++)
	{
		cout << i << "		";
		output << i << "	  ";

		set<pdaState>::iterator state;
		for (state = pda.states.begin(); state != pda.states.end(); state++)
		{
			if ((*state).name == i)
			{
				set<Lr1Item>::iterator it;
				for (it = (*state).items.begin(); it != (*state).items.end(); it++)
				{
					if ((*it).dot == (*it).numofrights)//如果是需要规约的LR1项，找出其为第几个产生式
					{
						int count = 1;
						vector<Lr1Item>::iterator it_item;
						for (it_item = Item.begin(); it_item != Item.end(); it_item++)//set容器能否保证读LR1项的顺序为自然向下？
						{
							bool flag = true;
							if ((*it).leftp == (*it_item).leftp)//若左部相同
							{
								for (int j = 0; j < 10; j++)
								{
									if ((*it).rightp[j] != (*it_item).rightp[j])//若右部不同，
									{
										flag = false;
									}
								}
							}
							else//左部不同
							{
								flag = false;
							}

							if (flag)//如果左右部相同，即找到其产生式
							{
								if ((*it).leftp == "S'")
								{
									count = 0;//第0个产生式
								}
								break;//跳出for
							}
							count++;
						}

						set<string>::iterator it_pre;
						for (it_pre = (*it).predict.begin(); it_pre != (*it).predict.end(); it_pre++)//解决移入规约冲突和规约规约冲突
						{
							bool conflict = false;
							int k;
							for (k = 0; k < Lr1table[i].nextState.size(); k++)
							{
								if (Lr1table[i].symbol[k] == (*it_pre))//如果边上的符号中找到了该LR1项展望符中的符号，又该项已经是待规约项，冲突
								{
									conflict = true;
									break;
								}
							}

							int reducenum = 0 - count;//负数表示规约

							if (conflict)//有冲突，处理，选择规约？
							{
								Lr1table[i].nextState[k] = reducenum;//改写值
							}
							else//不冲突，规约，规约项是无边的，故使用push_back()放末尾
							{
								Lr1table[i].nextState.push_back(reducenum);
								Lr1table[i].symbol.push_back((*it_pre));//给待规约项加个符号，相当于增加一条边
							}

						}
					}
				}
			}
		}
		for (int m = 0; m < Lr1table[i].nextState.size(); m++)
		{
			if (Lr1table[i].nextState[m] == 0)//accept
			{
				cout << "accept" <<"  "<<Lr1table[i].symbol[m]<< "		   ";
				output << "accept" <<"  "<<Lr1table[i].symbol[m]<< "		     ";
			}
			else if (Lr1table[i].nextState[m] > 0)//移进项
			{
				cout << "s" << Lr1table[i].nextState[m] <<"  "<<Lr1table[i].symbol[m]<< "		  ";
				output << "s" << Lr1table[i].nextState[m] <<"  "<<Lr1table[i].symbol[m]<< "		    ";
			}
			else//负数代表规约项
			{
				cout << "r" << Lr1table[i].nextState[m] <<"  "<<Lr1table[i].symbol[m]<< "		    ";
				output << "r" << Lr1table[i].nextState[m] <<"  "<<Lr1table[i].symbol[m]<< "		      ";
			}

		}

		cout << endl;
		output << endl;
	}
	return Lr1table;
}
ofstream outc("../Debug/seuyacc.cpp");
void gc(string content, int num)
{
	for (int i = 0; i < num; i++)
		outc << "\t";

	outc << content << endl;
}



void gencode(table* mtable)
{
	gc("#define _CRT_SECURE_NO_WARNINGS", 0);
	gc("#include<iostream>", 0);
	gc("#include <stack>", 0);
	gc("#include<queue>", 0);
	gc("#include<fstream>", 0);
	gc("#include<vector>", 0);
	gc("#include<map>", 0);
	gc("#include<string>", 0);
	gc("using namespace std;", 0);
	gc("#define BUFF 512", 0);

	gc("bool isused = true;", 0);
	gc("bool iseplsion = false;", 0);
	gc("struct production", 0);
	gc("{", 0);
	gc("string leftp;", 1);
	gc("string rightp[10];", 1);
	gc("int numofrights;", 1);
	gc("};", 0);
	gc("vector<production> productions(50);", 0);
	gc("struct token{", 0);
	gc("string name;", 1);
	gc("string type;", 1);
	gc("};", 0);
	gc("queue<token> tokenqueue;", 0);
	gc("struct table2 {", 0);
	gc("vector<string> tg;", 1);
	gc("vector<int> next;", 1);
	gc("};", 0);
	//gc("", 0);
	gc("void printbuffer(vector<string> a)", 0);
	gc("{", 0);
	gc("vector<string>::iterator i;", 1);
	gc("for (i = a.begin();i != a.end();i++)", 1);
	gc("{", 1);
	gc("cout << (*i) << \" \";", 2);
	gc("}", 1);
	gc("}", 0);

	gc("void tableAnalyse(stack<token>& mtokenstack, stack<int>& mnstack, vector<string>& moutbuffer, token t, table2*& GOTO2)", 0);
	gc("{", 0);
	gc("for (int i = 0;i < GOTO2[mnstack.top()].tg.size();i++)", 1);
	gc("{", 1);
	gc("if (t.type == GOTO2[mnstack.top()].tg[i])", 2);
	gc("{", 2);
	gc("if (GOTO2[mnstack.top()].next[i] > 0)", 3);
	gc("{", 3);
	gc("cout << \"\t转移到状态:\" << GOTO2[mnstack.top()].next[i] << endl;", 4);
	gc("mnstack.push(GOTO2[mnstack.top()].next[i]);", 4);
	gc("if (t.type != \"eplsion\")", 4);
	gc("{", 4);
	gc("mtokenstack.push(t);", 5);
	gc("moutbuffer.push_back(t.name);", 5);
	gc("}", 4);
	gc("break;", 4);
	gc("}", 3);
	gc("else if (GOTO2[mnstack.top()].next[i] < 0)", 3);
	gc("{", 3);
	gc("int num = 0 - GOTO2[mnstack.top()].next[i];", 4);
	gc("cout << \"\t使用产生式规约:\" << productions[num].leftp << \"--->\";", 4);
	gc("for (int i = 0;i < productions[num].numofrights;i++)", 4);
	gc("{", 4);
	gc("cout << productions[num].rightp[i];", 5);
	gc("if (productions[num].rightp[0] != \"eplsion\")", 5);
	gc("{", 5);
	gc("mtokenstack.pop();", 6);
	gc("moutbuffer.pop_back();", 6);
	gc("}", 5);
	gc("mnstack.pop();", 5);
	gc("isused = false;", 5);
	gc("}", 4);
	gc("token a{ productions[num].leftp ,productions[num].leftp };", 4);
	gc("tableAnalyse(mtokenstack, mnstack, moutbuffer, a, GOTO2);", 4);
	gc("break;", 4);
	gc("}", 3);
	gc("else", 3);
	gc("{", 3);
	gc("cout << \"编译完成\" << endl;system(\"pause\");", 4);
	gc("exit(1);", 4);
	gc("}", 3);
	gc("}", 2);
	gc("if (GOTO2[mnstack.top()].tg[i] == \"eplsion\")", 2);
	gc("iseplsion = true;", 3);
	gc("if (i == GOTO2[mnstack.top()].tg.size() - 1)", 2);
	gc("{", 2);
	gc("if (iseplsion == true)", 3);
	gc("{", 3);
	gc("token b{ \"eplsion\",\"eplsion\" };", 4);
	gc("iseplsion = false;", 4);
	gc("tableAnalyse(mtokenstack, mnstack, moutbuffer, b, GOTO2);", 4);
	gc("isused = false;", 4);
	gc("break;", 4);
	gc("}", 3);
	gc("cout << endl;", 3);
	gc("cout << \"fail in compiling.\" << endl;", 3);
	gc("exit(1);", 3);
	gc("}", 2);
	gc("}", 1);
	gc("iseplsion = false;", 1);
	gc("}", 0);

	gc("int main()", 0);
	gc("{", 0);
	outc << "map<int, string> I2S;\n I2S[1]=\"ASSIGN\";I2S[2]=\"COMMA\";I2S[3]=\"DIVIDE\";I2S[4]=\"DOT\";I2S[5]=\"ELSE\";I2S[6]=\"EQUAL\";I2S[7]=\"FLOAT\";I2S[8]=\"IF\";I2S[9]=\"INT\";I2S[10]=\"LBRACE\";I2S[11]=\"LBRACK\";I2S[12]=\"LPAR\";I2S[13]=\"MINUS\";I2S[14]=\"NAME\";I2S[15]=\"NUMBER\";I2S[16]=\"PLUS\";I2S[17]=\"RBRACE\";I2S[18]=\"RBARCK\";I2S[19]=\"RETURN\";I2S[20]=\"RPAR\";I2S[21]=\"SEMICOLON\";I2S[22]=\"STRUCT\";I2S[23]=\"TIMES\";" << endl;
	gc("fstream input;", 1);
	gc("char content[BUFF];", 1);
	gc("input.open(\"../Debug/token.txt\", ios::in);", 1);
	gc("token nowtoken;", 2);
	gc("while (!input.eof()) {", 2);
	gc("input.getline(content, BUFF);", 3);
	gc("char* p;", 3);
	gc("p = strtok(content, \", \");", 3);
	gc("nowtoken.type = I2S.at(atoi(p));", 3);
	gc("p = strtok(NULL, \", \");", 3);
	gc("nowtoken.name = p;", 3);
	gc("tokenqueue.push(nowtoken);", 3);
	gc("}", 2);
	gc("token endtoken;", 2);
	gc("endtoken.name = \"#\";", 2);
	gc("endtoken.type = \"#\";", 2);
	gc("tokenqueue.push(endtoken);", 2);
	outc << "\t\tint num =" << to_string(pda.numofstates);  outc << ";" << endl;
	gc("table2* GOTO2 = new table2[num];", 2);

	for (int i = 0; i < pda.numofstates; i++)
	{
		vector<string>::iterator iter = mtable[i].symbol.begin();
		for (iter; iter != mtable[i].symbol.end(); iter++)
			outc << "\t\tGOTO2[" << i << "].tg.push_back(\"" << (*iter) << "\");" << endl;
		vector<int>::iterator iter2 = mtable[i].nextState.begin();
		for (iter2; iter2 != mtable[i].nextState.end(); iter2++)
			outc << "\t\tGOTO2[" << i << "].next.push_back(" << (*iter2) << ");" << endl;
	}
	vector<Lr1Item>::iterator pit;
	int i = 1;
	for (pit = Item.begin(); pit != Item.end(); pit++)
	{
		outc << "\t\tproductions[" << i << "].leftp=" << "\"" << (*pit).leftp << "\"" << ";" << endl;
		outc << "\t\tproductions[" << i << "].numofrights=" << (*pit).numofrights << ";" << endl;
		for (int j = 0; j < (*pit).numofrights; j++)
		{
			outc << "\t\tproductions[" << i << "].rightp[" << j << "]=" << "\"" << (*pit).rightp[j] << "\"" << ";" << endl;
		}
		i++;
	}
	gc("stack<token> tokenstack;//token栈", 2);
	gc("stack<int> nstack;//state栈", 2);
	gc("nstack.push(0);", 2);
	gc("token ini;", 2);
	gc("ini.name = \"#\";", 2);
	gc("ini.type = \"#\";", 2);
	gc("tokenstack.push(ini);", 2);
	gc("vector<string> outbuffer;", 2);
	gc("outbuffer.push_back(\"#\");", 2);
	gc("token now=tokenqueue.front();", 2);
	gc("cout << \"编译开始！\" << endl;", 2);
	gc("cout << \"词法单元栈\" << \"\t\" << \"移进规约具体操作\"<<endl;", 2);
	gc("while (!(tokenqueue.empty()))", 2);
	gc("{", 2);
	gc("printbuffer(outbuffer);", 3);
	gc("tableAnalyse(tokenstack, nstack, outbuffer, now, GOTO2);", 3);
	gc("if (isused)", 3);
	gc("{", 3);
	gc("tokenqueue.pop();", 4);
	gc("now = tokenqueue.front();", 4);
	gc("}", 3);
	gc("isused = true;", 3);

	gc("}", 2);




	gc("}", 0);



}

int main()
{
	inputyacc();
	analysisOfLr1();

	gencode(analysisOfTable());
	system("pause");
	return 0;
}