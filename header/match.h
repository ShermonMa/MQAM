#ifndef _MATCH_H
#define _MATCH_H

#include <vector>
#include "graph.h"
#include "symmetry.h"
#include <stack>
#include "type.h"
#include "globalVar.h"
#include "spp.h"
using namespace std;
using classifier = std::unordered_map<std::string, u_map>;

class Match
{
public:
	Match(two_u_map &, std::vector<vertex_Q> &, std::unordered_map<int, std::multimap<int, vec>> &,
		  std::vector<vertex_G> &, int*, tri_u_map &, tri_u_map &, two_u_map &, two_u_map &, two_umap_twovec &, umap_twovec &,
		  two_umap_twovec &, std::vector<Graph::od_node> &, two_u_map &, tri_u_map &,
		  std::vector<SymGroupData> &, SymmetryRepresenter &sr,int* ,int,std::vector<SymmetryRecord>& QSym);
	// void initiate(); // 对称结构初始化
	void updateAndMatch();
	double getTime_indexUpdate();
	double getTime_search();
	// bool symAuxToggle = true;
	long int partialMatch_num = 0;
	int getPartialMatchNum();
	long long allMatch=0;
	long long matchFromComm = 0,matchFromRemain = 0;
	vector<SymGroupData> symGroupData;
		bool coutAble = false;
	// 		bool coutComm = false;
	// string commpath;
private:
	two_u_map Q_adjList;
	std::vector<vertex_Q> q;
	std::unordered_map<int, std::multimap<int, vec>> q_edges;
	std::vector<vertex_G> initG;
	int* update;
	int* range; // 更新流
	tri_u_map CommSub;
	tri_u_map RemainQ;
	two_u_map RemainQ_adjList;
	two_u_map RemainQ_initialCandU;
	two_umap_twovec orderCommSub;

	umap_twovec orderRemainQ;
	two_umap_twovec orderEdgeOfRemainQ;

	std::vector<Graph::od_node> orderTree_node;
	two_u_map orderTree_bgNode;
	tri_u_map orderTree;

	bool checkInputEdge(int, int);

	// 更新索引
	void updateIndex(int, int);
	void store_Changed_selfLI(int, int, int);
	two_u_map changed_selfLI;													// v - 查询图id - 匹配状态改变的u
	std::unordered_map<int, std::unordered_map<int, vec>> stream_matched_edges; // 查询图id - u - u_nbrs

	// add edges
	void add_G_Nei(int, int);
	void addEdge_UpdateCandAndSelfLI(int, int);
	void add_CandAndSelfLI(int, int, int, int);
	void addEdge_UpdateNbrLI_v_and_nbrs(int, int);
	void addEdge_UpdateNbrLI_vi_and_vj(int, int);
	void add_nbrLI(int, int, int);

	// delete edges
	void del_G_Nei(int, int);
	void delEdge_UpdateCandAndSelfLI(int, int);
	void del_CandaAndSelfLI(int, int, int, int);
	void delEdge_update_nbrLI_v_and_nbrs(int, int);
	void delEdge_update_nbrLI_vi_and_vj(int, int);
	void del_nbrLI(int, int, int);

	// Symmetry Update -- new Varaiables
	std::vector<SymmetryRecord> QSym;

	struct SymmetryGroup
	{
		int representer;
		bool isSelfConnected;
		bool havingCommon;

		// bool candidatesInited = false; // 记录是否初始化candidates
		set<int> crucialPoints;
		set<int> vertices;
		// set<int> commVertices;
		// vector<int> candidates;
		// int startGVertices = 0;
		// unordered_map<int, int> commChoice;
		// unordered_map<int, int> currentChoice;
		// std::stack<int> currentThreshold;

	};
	std::vector<std::unordered_map<int, std::shared_ptr<SymmetryGroup>>> SymmetryGroupsOfAllQG;
	SymmetryRepresenter symRepresenter;
	int currentvi, currentvj;
	int currentviToUi, currentvjToUj; // vi vj选中的ui uj
	int currentEdgeStart, currentEdgeEnd;
	int QGNum;
	int updateSize;
	int numQForAComm;
	int numNodesForAQ;
	struct matching_Q
	{
		int dif;
		int* ngbrRepre;
		std::vector<int> transitionFlag;
		std::unordered_map<int,SAuxUnit> group;
		SAuxUnit& at(int index) {
			return group.at(transitionFlag[index-dif]);
		}
		bool find(int index) {
			return transitionFlag[index-dif] != -1;
		}
		matching_Q(int numNodesForAQ,int q_id)
		{
			transitionFlag.resize(numNodesForAQ,-1);
			this->dif = q_id*numNodesForAQ;
		}
	};
	vector<SymmetryManager> eachQManager;
	//newEnumerationFunc
	int newSym_countRemainQMatch(int ui, int uj, int index_u, int sum, u_map cand, unordered_map<int, int> match, set<int> used_v,int isoph);
	long long newSym_countJoinedMatch(int q_id, int index_u, int sum, u_map cand, 
				unordered_map<int, int> match, set<int> used_v, int dif, int isoph);
	pair<u_map,set<int>> newSym_updateCand(bool, int, int, u_map, std::unordered_map<int, int>, std::set<int>, int);
	pair<u_map,set<int>> newSym_cmpJoinedInitCand(int, std::unordered_map<int, int>, std::set<int>, int);

	tuple<int,set<int>,set<int>> analyseUpdateCand(int u,const u_map& cand,const set<int>& newNodes);
	int analyseInitCand_Joint(int q_id,const u_map& cand,int dif);
	void deleteSymGroup(const set<int>&,int,const set<int>&,const u_map&);
	int addSymGroup(int repre,const set<int>& candidates,const set<int>& vectors);
	int computeStartNum(int u,int j);
	// func

	bool symCheckFromRemainSimplified(int q_id, int ui, int uj, int vi, int vj);
	bool symFilterBeforeInit_Remain(const set<int> &cand, int u);

	set<int> findNbrLIForFirstNodeSymMatch_Remain(int ui, int uj, int u);
	

	// symCheck type func检查类函数
	//inline bool isNgbrSymmetryVertex(int);
	//inline bool isSymmetryVertex(int q_id);
	bool isCommEdge(int ui, int uj);
	//inline bool isSymmetryVertexSameNgbr(int ui, int uj);
	inline bool isNgbrSymmetryVertex(int vertex_id){
		return symRepresenter.symmetryNgbrRepresenter[vertex_id] != -1;
	}
	inline bool isSymmetryVertex(int vertex_id){
		return symRepresenter.symmetryRepresenter[vertex_id] != -1;
	}
	inline bool isSymmetryVertexSameNgbr(int ui,int uj){
		if(symRepresenter.symmetryRepresenter[ui] == -1)return false;
		return symRepresenter.symmetryNgbrRepresenter[ui] == symRepresenter.symmetryNgbrRepresenter[uj];
	}
	// 计算matches
	struct Aux
	{
		vector<std::unordered_map<int, int>> match;
		vector<u_map> candidate;
		vector<std::set<int>> used_v;
		vec comm_q_id; // 公共子图对应的q_id, 一般为该组查询图的首个id,不满足匹配首个id时,为特殊情况
		vector<pair<int, int>> startUiUj;
	};

	struct SymAux
	{
	};
	unordered_map<int, unordered_map<int, Aux>> Auxiliary; // 存公共子图的所有匹配结果  vi - vj - aux

	long long searchMatch(int, int);
	bool checkEdgeInCommSub(int, int);
	bool checkEdgeInOrderTree(int, int);
	long long searchMatchedEdgeFromCommon(int, int);
	int countCommSubMatch(int, int, int, int, u_map, unordered_map<int, int>, set<int>, int dif);
	void addAuxiliary(int, int, int, std::unordered_map<int, int> copy_match, u_map, std::set<int>,pair<int,int>);
	int countJoinedMatch(int, int, int, u_map, unordered_map<int, int>, set<int>, int);

	int searchMatchedEdgeFromRemainQ(int, int);
	int countRemainQMatch(int, int, int, int, u_map, unordered_map<int, int>, set<int>);

	u_map cmpInitCand(int, int, int, int);
	bool checkBidirectionalEdgeOnIndex(int, int, int, int);
	u_map genEdgeCand(int, int, int, int, u_map);
	u_map updateCand(bool, int, int, u_map, std::unordered_map<int, int>, std::set<int>, int);
	u_map cmpJoinedInitCand(int, std::unordered_map<int, int>, std::set<int>, int);

	void print_v(int);
	void print_time();
	void print_unordered_map(std::unordered_map<int, int>);
	void print_umap(u_map);
	void print_multimap(std::multimap<int, int>);
	void print_set(std::set<int>);
	void printOrderCommSub(two_umap_twovec orderCommSub);
	double time_slot1, time_slot2, time_slot3, time_slot1_1, time_slot1_2, time_slot1_3, time_slot2_checkEdgeNbr, time_slot2_removeEdge_updateQ, time_slot2_checkNodeNbr, time_slot2_removeNode_updateQ, time_slot1_generate_match_order, time_slot2_count_Match, time_slot3_1, time_slot3_2, time_slot3_3;
	
};

#endif

// !_MATCH_H //
//
