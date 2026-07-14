
#ifndef _SYMMETRY_H
#define _SYMMETRY_H

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <vector>
#include <set>
#include <string>
#include <deque>
#include <queue>
#include <memory>
#include <stack>
#include <string.h>
#include <cstring>
//Symmetry Structure
struct SymmetryRepresenter
{
	// std::vector<int> allSymGraphCommID;
	std::unordered_map<int,std::set<int>> forbidStart;//-ui--uj不允许使用这样的边开始遍历
	int* symmetryRepresenter;
	int* symmetryNgbrRepresenter;
};
struct SymmetryNgbrGroup
{
	std::set<int> vertices;
	bool isSelfConnected;
};
struct SymmetryRecord
{
	
	int nodeNum;	int matchNum = 0;
	int rangeS,	rangeE;
	int* originGraph;
	//用于记录一跳对称点的组长结点--注意偏移量为range.first
	int* ngbrSymRepresenter;int* symmetryRepresenter;
	std::vector<std::vector<int>> allSymmetryOutCome;
	std::vector<std::vector<int>> eachVertexCandidate;
	//注意两个代表索引是从0开始的，因此需要和后面的存储做偏移量转换
	std::unordered_map<int,std::shared_ptr<SymmetryNgbrGroup>> ngbrGroup;
	std::unordered_map<int,std::unordered_map<int,int>> startMulti; //ui--uj--起始的对称计数
	std::unordered_map<int,std::set<int>> forbidden;//禁止作为起始边的顶点对。正反序都存
	//每组仅有一个代表值有key

	std::unordered_map<int,std::shared_ptr<std::set<int>>> symmetryGroup;//并未完全弃用symmetryGroup的全连接版本
	SymmetryRecord(int rangeS,int rangeE)
	{
		this->nodeNum = rangeE - rangeS +1;
		this->rangeS = rangeS;
		this->rangeE = rangeE;
		this->eachVertexCandidate.resize(rangeE - rangeS +1);
		originGraph = new int[nodeNum];
		ngbrSymRepresenter = nullptr;
		symmetryRepresenter = nullptr;
		for(int i = 0;i<nodeNum ; i++)
		{
			originGraph[i] = i+rangeS;
		}
	}
SymmetryRecord(const SymmetryRecord& other)
        : nodeNum(other.nodeNum),
          matchNum(other.matchNum),
          rangeS(other.rangeS),
          rangeE(other.rangeE),
          allSymmetryOutCome(other.allSymmetryOutCome),
          eachVertexCandidate(other.eachVertexCandidate),
          ngbrGroup(other.ngbrGroup),  // 注意：如果 SymmetryNgbrGroup 需要深拷贝，需自定义拷贝逻辑
          startMulti(other.startMulti),
          forbidden(other.forbidden),
          symmetryGroup(other.symmetryGroup)  // 同样注意 set<int> 的深拷贝问题
    {
        // 拷贝三个 int 数组
		originGraph = new int[nodeNum];
		memcpy(originGraph, other.originGraph, nodeNum * sizeof(int));
        if (other.matchNum > 1) {
            ngbrSymRepresenter = new int[nodeNum];
            memcpy(ngbrSymRepresenter, other.ngbrSymRepresenter, nodeNum * sizeof(int));
        } else {
            ngbrSymRepresenter = nullptr;
        }
        if (other.matchNum > 1) {
            symmetryRepresenter = new int[nodeNum];
            memcpy(symmetryRepresenter, other.symmetryRepresenter, nodeNum * sizeof(int));
        } else {
            symmetryRepresenter = nullptr;
        }
    }
	~SymmetryRecord(){
		delete[] originGraph;
		if(matchNum!=1)
		{
			delete[] ngbrSymRepresenter;
			delete[] symmetryRepresenter;
		}
	}
};
struct SymmetryUnit
{
	int matchedNum = 0;
	std::set<int> vertices;
	std::vector<int> candidates;
	std::stack<int> current;//表示candi从左往右选的位置
};
struct SymmetryManager//每个图有一个SymmetryManager
{
	std::unordered_map<int,std::shared_ptr<SymmetryUnit>> symmetryUnit;//每个unit是一个小组，记录本组结点和候选,多个同组key指向一个数据结构
	std::unordered_map<int,std::set<int>> unitVertices;//不使用，仅展示表示我们需要一个结构，用于批量删除同组key
};
struct SAuxUnit
{
	int matchedNum = 0;
	int lastChoice;
	int size;
	std::set<int> vertices;
	std::set<int> candPool;
};
struct DicForRemain
{
	int rangeS;
	int* ngbrRepre;
	int startUi,startUj,startVi,startVj;
};

// using  marchG = std::unordered_map<int,std::shared_ptr<SAuxUnit>> ;
// struct symRecursionAux
// {
// 	std::unordered_map<int,std::shared_ptr<SAuxUnit>> marchG;
// 	std::unordered_map<int,std::set<int>> unitVertices;//不使用，仅展示表示我们需要一个结构，用于批量删除同组key
// };
struct SymGroupData//需要传入到match的SymmetryRecord变量
{
	std::unordered_map<int,std::shared_ptr<SymmetryNgbrGroup>> ngbrGroup;
	int rangeS,rangeE;
	int matchNum;

	std::unordered_map<int,std::set<int>> forbidden;//禁止作为起始边的顶点对。正反序都存
	std::unordered_map<int,std::unordered_map<int,int>> startMulti;
};
void symmetryNodeGrouplize(SymmetryRecord &);
void symmetryNgbrRecognition(SymmetryRecord &sr);
void printSymmetryGroup(SymmetryRecord &sr);
void printSymmetryNgbrGroup(SymmetryRecord &sr);
void printMatchCand(std::unordered_map<int,std::set<int>> cand, bool flag);
#endif