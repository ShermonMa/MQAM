#ifndef _GRAPH_H
#define _GRAPH_H

#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <string>
#include "symmetry.h"
#include "type.h"
#include "globalVar.h"
using namespace std;

#include <chrono>
#define Get_Time() std::chrono::high_resolution_clock::now()
// #define Duration(start) std::chrono::duration_cast<\
//     std::chrono::microseconds>(Get_Time() - start).count()/(float)1000


using vec = std::vector<int>;
using two_vec = std::vector<vec>;
using u_set = std::unordered_set<int>;
using u_map = std::unordered_map<int, std::set<int>>;
using umap_bool = std::unordered_map<int, bool>;
using umap_uset = std::unordered_map<int, u_set>;
using umap_twovec = std::unordered_map<int, two_vec>;
using two_u_map = std::unordered_map<int, u_map>;
using two_map = std::map<int, std::map<int, int>>;
using two_umap = std::unordered_map<int, std::unordered_map<int, int>>;
using two_umap_twovec = std::unordered_map<int, umap_twovec>;
using tri_umap_bool = std::unordered_map<int, std::unordered_map<int, umap_bool>>;
using tri_umap = std::unordered_map<int, two_umap>;
using tri_u_map = std::unordered_map<int, two_u_map>; 


class Graph
{
public:
	Graph()
	{
		std::cout<<"start..."<<endl;
		range = new int[Config::MAX_QUERY_GRAPH_NUMBER*2];
		Q.reserve(Config::MAX_QUERY_GRAPH_NUMBER*Config::MAX_QUERY_GRAPH_SIZE);
		update = nullptr;
	}

	//Read data func
	void inputQ(const std::string& path);
	void inputG(const std::string& path);
	void inputUpdate(const std::string& path);
	//Automorphism Process
	void symProcess();
	//Order generation
	struct od_node    //orderTree里每个顶点
	{
		int label;
		vec u_set;
	};
	void genOrder(); 
	//Index Build
	void constructIndex();
	void clear() {
        Q.clear();
        G.clear();
        // if (update != nullptr) delete[] update;
        // update = nullptr;
        // if (range != nullptr) delete[] range;
        // range = nullptr;

        Q_adjList.clear();
        q_labelEdge.clear();
        QSym.clear();
        symGroupData.clear();
        labelEdgeOfCommSub.clear();
        OrderOfCommSub.clear();
        OrderOfRemainQ.clear();
        OrderOfEdgeOfRemainQ.clear();
        labelEdgeOfRemainingQ.clear();
        remainQ_adjList.clear();
        remainQ_initialCandU.clear();
        odTree_nodes.clear();
        odTree_startNodes.clear();
        orderTree.clear();
        Auxiliary.clear();
    }

	//Getters and prints
	two_u_map& getQAdjList(){return Q_adjList;}
	vector<vertex_Q>& getQ()	{return Q;}
	int getQGNum()		{return QGNum;}
	vector<vertex_G>& getG()	{return G;}
	int* getStream()	{return update;}
	int getStreamNum()	{return updateSize;}
	int* getRange() {return range;}
	std::unordered_map<int, std::multimap<int, vec>>& getQueryEdges(){return q_labelEdge;}
	tri_u_map& getLabelEdgeOfCommSub()	{return labelEdgeOfCommSub;}
	two_umap_twovec& getOrderOfCommSub(){return OrderOfCommSub;}
	umap_twovec& getOrderOfRemainQ()	{return OrderOfRemainQ;}
	two_umap_twovec& getOrderOfEdgeOfRemainQ(){return OrderOfEdgeOfRemainQ;}
	tri_u_map& getLabelEdgeOfRemainingQ()	{return labelEdgeOfRemainingQ;}
	two_u_map& getRemainQ_adjList()			{return remainQ_adjList;}
	two_u_map& getRemainQ_initialCandU()	{return remainQ_initialCandU;}
	std::vector<od_node>& getOdTreeNodes(){return odTree_nodes;}
	two_u_map& getOdTreeStartNodes()	{return odTree_startNodes;}
	tri_u_map& getOrderTree()	{return orderTree;}
	void printSymmetryRecord(int q_id);
	std::vector<SymmetryRecord>& getQSym()	{return QSym;}
    SymmetryRepresenter& getSymRepresenter(){return symRepresenter;}
	vector<SymGroupData>& getSymGroupData() {return symGroupData;}


private:
	//Basic Variables
	vector<vertex_Q> Q;
	vector<vertex_G> G;
	int* update;
	int updateSize = 0;
	int* range;
	int QGNum = 0;
	two_u_map Q_adjList;          //查询图id - 邻接表
	std::unordered_map<int, std::multimap<int, vec>> q_labelEdge;//label(ui) - label(uj) -v[0]是查询图id，v[1]是label(ui)在该查询图对应的所有查询点id A-B-[0 7 8]
    std::vector<SymmetryRecord> QSym;
	SymmetryRepresenter symRepresenter;
	std::vector<SymGroupData> symGroupData;
	//edges
	two_u_map commSubgraph;      //生成查询图时，先生成的核心公共子图 commmon_subgraph_id - ui - adjs
	tri_u_map labelEdgeOfCommSub;  //核心公共子图按label存边 label(ui) - label(uj) - ui - adjs 
	tri_u_map labelEdgeOfRemainingQ; //剩余图中按label存边,同上
	two_u_map remainQ_adjList;   // q_id - ui - adjs
	two_u_map remainQ_initialCandU; // join剩余查询图时,初始候选集需要包含哪些u, q_id - ui - adjs
	//Orders
	two_umap_twovec OrderOfCommSub;//!!! ui - uj - {[u, 1], [u, 0],...},其中label(ui)<label(uj),若label相等则i<j, 1表示major顶点,0表示minor顶点,是在公共区域里的major/minor属性
	umap_twovec OrderOfRemainQ; //!!! qi - {[u, 1], [u, 0],...}, 每个查询图除去公共部分以后，剩余图的匹配顺序
	two_umap_twovec OrderOfEdgeOfRemainQ;//!!! ui - uj - {[u, 1], [u, 0],...}

	struct Aux            //计算下一个匹配顶点的Auxiliary信息
	{
		unordered_map<int, int> major;  // label - u, 相同label只留度最大的在剩余查询图上
		u_map minor;  // label  - [u,...]
		u_map adj;
		set<int> matched;
	};
	unordered_map<int, unordered_map<int, Aux>> Auxiliary;  // ui - uj - aux

	//OrderTrees
	std::vector<od_node> odTree_nodes;
	two_u_map odTree_startNodes;  // label(ui) - label(uj) - {od_node, ....}
	tri_u_map orderTree;   // label(ui) - label(uj) - 邻接表
	void ReInitiateQueryVertices();
	//Data Process
	void storeCommSubgraph(int, int, int);
	void storeLabelEdge(bool, int, int);
	void addLabelEdge(bool, int, int);
	void traverseRemainingQ(u_map);

	//Automorphism Recognition
	void selfSearch(int q_id);
    void backTrack(int nodeIndex, const std::unordered_map<int,std::set<int>>& adjList, 
        std::vector<bool>& usedFlag, std::vector<int>& outcome,std::pair<int,int> range);
    bool isAutomorphism(const std::unordered_map<int,std::set<int>>& adjList, std::vector<int>& outcome,std::pair<int,int> range);
    bool sameFeature(std::vector<int> aNgbr, std::vector<int> bNgbr);
	void symmetryNodeGrouplize(SymmetryRecord &);
	void generateQSymVInComm();
	void calculateCruialPointsAndSelfConnected(int q_id);
	void orderCutRemain();
	void computeStartMulti(bool isComm,int id,int ui,int uj);
	bool isLegalEdge_Remain(int, int);
	bool isLegalEdge_Comm(int ui,int uj);
	bool isCommEdge(int ui,int uj);
	inline bool isNgbrSymmetryVertex(int vertex_id){return symRepresenter.symmetryNgbrRepresenter[vertex_id] != -1;}
	inline bool isSymmetryVertex(int vertex_id){return symRepresenter.symmetryRepresenter[vertex_id] != -1;}
	//-------要在这里找到每个图公共部分中的对称点是谁，来给后面用------

	//起始边在核心部分,核心公共部分的匹配顺序，以及剩余非公共部分的匹配顺序
	//①公共部分匹配顺序
	void cmpOrderOfCommSub();
	void cmpOrderOf_UiUj(bool, int, int, int, u_map, std::set<int>);
	vec cmpMajor(int, u_map, std::set<int>);
	vec cmpFinalMajor(vec, vec);
	void addOrderOfCommSub(bool, int, int, int);
	u_map removeMatchedNodeFromQ(int, u_map);
	u_map removeAnEdgeFromQ(int, int, u_map);
	void cmpMinorForCommSub(bool, int, int);
	u_map removeCommSubFromQ(int, int);
	int cmpDifBtw_ActualId_And_IdInCommSub(int, int);
	//②剩余部分匹配顺序
	void cmpOrderOfRemainingQ(int, u_map, std::set<int>);
	std::multimap<int, int> cmpNextMajorFromRemainingQ(int, u_map, std::set<int>);
	void addOrderOfRemainingQ(bool, int, std::set<int>);
	void symOrderEdgeTransition(int ui,int uj,unordered_map<int,unordered_map<int,unordered_map<int,set<int>>>>& order);
	bool checkEdgeRemainExist(int ui,int uj);
	void prepareSymData();

	//起始边不是核心公共部分
	//1.分别计算每条查询边的匹配顺序 using AUX
	void cmpOrderOfEdgeOfRemainQ();
	void updateAuxiliary_Matched(bool, int, int, int);
	void updateAuxiliary_Adj(bool, int, int, u_map);
	void cmpMajorAndMinor(int, int, int);
	void updateMajorAndMinor(int, int);
	void addMajorInAuxiliary(int, int, int);
	void addMinorInAuxiliary(int, int, int);
	void deleteMinorInAuxiliary(int, int, int);
	bool compareDegree(int, int, int);
	void generateOrderOfEdgeOfRemainQ(multimap<int, int>, multimap<int, int>);
	multimap<int, int> divideNextByLabel(string);
	multimap<int, int> sortLabelByNum(unordered_map<int, int>);
	void cmpNextForAllEdge(multimap<int, int>, multimap<int, int>);
	void addOrderOfEdgeOfRemainQ(bool, int, int, int);
	//2.label相同的查询边合并成tree
	void summaryAllOrder(int, int);
	two_u_map classfiyNodes(two_u_map, int, int, int);
	void update_odNode_uSet(u_map, int, int);
	void addOdTree_startNodes(int, int, int);
	void addOdTree_adjKey(int, int, int);
	void update_OdTree_adj(int, int, int, int);
	void combineAGroupOfOrders(int, int, u_map, int, int);



	//index
	void update_nbr_LI(int, int, int, int);

	//print
	void print_Q();
	void print_u(int);
	void print_umap(u_map);
	void print_two_umap(two_u_map);
	void print_two_umap_twovec(two_umap_twovec);
	void print_vec(vec);
	void print_set(set<int>);
	void print_umap_twovec(umap_twovec);
	void print_tri_u_map(tri_u_map);
	void print_Auxiliary(unordered_map<int, unordered_map<int, Aux>>);
	void print_multimap(std::multimap<int, int>); 

};


#endif // !_GRAPH_H




