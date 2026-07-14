#include "match.h"
#include <iostream>
#include <algorithm>
#include "type.h"
#include <chrono>
#include <fstream>
#define Get_Time() std::chrono::high_resolution_clock::now()
#define Duration(start) std::chrono::duration_cast<\
    std::chrono::microseconds>(Get_Time() - start).count()/(float)1000
// int currentvi;
// int currentvj;

Match::Match(two_u_map& qAdjList, std::vector<vertex_Q>& qg, 
			std::unordered_map<int, std::multimap<int, vec>>& qg_edges, 
			std::vector<vertex_G>& initialG, int* stream, tri_u_map& commSub, 
			tri_u_map& remainQ, two_u_map& remainQ_adjList, 
			two_u_map& remainQ_initialCandU, two_umap_twovec& od_commSub, 
			umap_twovec& od_remainQ, two_umap_twovec& od_edgeOfRemainQ, 
			std::vector<Graph::od_node>& ot_node, two_u_map& ot_bgNode, tri_u_map& ot ,  
			std::vector<SymGroupData>& Sym,
			SymmetryRepresenter &sr, int* rg,int updateS,std::vector<SymmetryRecord>& QSym)
{
	Q_adjList = qAdjList;
	q = qg;
	q_edges = qg_edges;
	initG = initialG;
	update = stream;
	CommSub = commSub;
	RemainQ = remainQ; 
	RemainQ_adjList = remainQ_adjList;
	RemainQ_initialCandU = remainQ_initialCandU;
	orderCommSub = od_commSub;
	orderRemainQ = od_remainQ;
	orderEdgeOfRemainQ = od_edgeOfRemainQ;
	orderTree_node = ot_node;
	orderTree_bgNode = ot_bgNode;
	orderTree = ot;
	numQForAComm = Q_adjList.size() / (q.end() - 1)->comm_id;
	numNodesForAQ = Q_adjList.begin()->second.size();
	symGroupData = Sym;
	symRepresenter = sr;	
	range = rg;
	updateSize = updateS;
	double time_slot1 = 0.0, time_slot2 = 0.0;  //time_slot1_1 = 0.0, time_slot1_2 = 0.0, time_slot1_3 = 0.0, time_slot2_checkEdgeNbr = 0.0, time_slot2_removeEdge_updateQ = 0.0, time_slot2_checkNodeNbr = 0.0, time_slot2_removeNode_updateQ = 0.0, time_slot2_count_Match = 0.0;
	QGNum = Q_adjList.size();
	this->QSym = QSym;
}


void Match::updateAndMatch()
{

	std::chrono::high_resolution_clock::time_point time1, time2;

	// std::cout<<"Ready To Print OrderCommSub"<<endl;
	// printOrderCommSub(orderCommSub);
	// std::cout<<"Finished Print OrderCommSub"<<endl;
	long long add_match = 0;
	long long del_match = 0;
	// if(coutComm)
	// {
	// 	std::ofstream ofs(commpath+"/commInsertion.graph", std::ios::trunc);
	// 	std::ofstream ofs2(commpath+"/remainInsertion.graph", std::ios::trunc);
	// }

	for (int t = 0; t < 2*updateSize; t += 2) {
		int v1 = update[t];
		int v2 = update[t + 1];
		// std::cout<<v1<<" "<<v2<<"  |  ";
		// if(t%10==0)std::cout<<endl;
		//删边
		bool skip = false;
		if (v1 < 0 || v2 < 0) {

			//cout << "delete (v1,v2)=" << v1 << "," << v2 << endl;
			skip = checkInputEdge(-v1, -v2);
			if (skip)
			{
				del_match += searchMatch(-v1, -v2);
				updateIndex(v1, v2);
			}

		}

		//加边
		else {
			//cout << "add (v1,v2)=" << v1 << "," << v2 << endl;
			skip = checkInputEdge(v1, v2);
			if (skip)
			{
				time1= Get_Time();
				updateIndex(v1, v2);
				time_slot1 += Duration(time1);

				time2 = Get_Time();
				add_match += searchMatch(v1, v2);
				time_slot2 += Duration(time2);
			}
		}
		// if(skip == false)
		// {
		// 	if(coutComm )
		// 	{
		// 		string path = commpath + "/remainInsertion.graph";
		// 		std::ofstream outFile(path, std::ios_base::app);
		// 		if (!outFile) return ;

		// 		outFile << "e "<<abs(v1)<<" "<<abs(v2)<<" 0"<<endl;
		// 		outFile.close();
		// 	}
		// }
		changed_selfLI.clear();
		stream_matched_edges.clear();
	}
	allMatch = add_match+del_match;
	std::cout << "positive matches: " << add_match << std::endl;
	std::cout << "negative matches: " << del_match << std::endl;
	std::cout << "comm matches: " << matchFromComm << std::endl;
	std::cout << "remain matches: " << matchFromRemain << std::endl;
	//print_time();
}

double Match::getTime_indexUpdate()
{
	return time_slot1;
}

double Match::getTime_search()
{
	return time_slot2;
}

int Match::getPartialMatchNum()
{
	return partialMatch_num;
}

bool Match::checkInputEdge(int v1, int v2)
{   // 无关结点的更新可以跳过
	if (initG[v1].label == -1 || initG[v2].label == -1) 
	{
		return false;
	}
	if (q_edges.at(initG[v1].label).find(initG[v2].label) == q_edges.at(initG[v1].label).end())
	{
		return false;
	}
	// std::cout<<"_EDGE_OK_";
	return true;
}

//更新索引
void Match::updateIndex(int v1, int v2)
{
	//std::chrono::high_resolution_clock::time_point time1, time2, time3;

	if (v1 < 0 || v2 < 0)
	{
		del_G_Nei(-v1, -v2);
		del_G_Nei(-v2, -v1);
		delEdge_UpdateCandAndSelfLI(-v1, -v2);
		delEdge_update_nbrLI_v_and_nbrs(-v1, -v2);
		delEdge_update_nbrLI_v_and_nbrs(-v2, -v1);
		delEdge_update_nbrLI_vi_and_vj(-v1, -v2);
	}
	else
	{
		//time1= Get_Time();
		initG[v1].nei.try_emplace(initG[v2].label).first->second.insert(v2);
		initG[v2].nei.try_emplace(initG[v1].label).first->second.insert(v1);
		//time_slot1_1 += Duration(time1);

		//time2 = Get_Time();
		addEdge_UpdateCandAndSelfLI(v1, v2);
		addEdge_UpdateCandAndSelfLI(v2, v1);
		//time_slot1_2 += Duration(time2);

		//time3 = Get_Time();
		addEdge_UpdateNbrLI_v_and_nbrs(v1, v2);
		addEdge_UpdateNbrLI_v_and_nbrs(v2, v1);
		addEdge_UpdateNbrLI_vi_and_vj(v1, v2);
		//time_slot1_3 += Duration(time3);

	}
}

// void Match::store_Changed_selfLI(int v, int Qi, int u)
// {
// 	changed_selfLI.at(v).try_emplace(Qi).first->second.insert(u);
// }

//①加边
// void Match::add_G_Nei(int v, int v_nbr)
// {
// 	initG[v].nei.try_emplace(initG[v_nbr].label).first->second.insert(v_nbr);
// }

void Match::addEdge_UpdateCandAndSelfLI(int v, int v_nbr)
{
	changed_selfLI.try_emplace(v);
	multimap<int, vec>::iterator it1 = q_edges[initG[v].label].lower_bound(initG[v_nbr].label);
	multimap<int, vec>::iterator it2 = q_edges[initG[v].label].upper_bound(initG[v_nbr].label);
	for (; it1 != it2; it1++)
	{
		vec val = it1->second;//vec:[QGID QVid1 QVid2 ...]
		long int thr = val.size();
		for (auto i = 1; i < thr; i++)    //val[0]是查询图id
		{
			int u = val[i];  //v可以匹配的u
			int v_nbr_label = initG[v_nbr].label;
			if (initG[v].nei.at(v_nbr_label).size() == q[u].q_neiSet.at(v_nbr_label).size())
			{  //加入边(v, v_nbr)后，正好v领域中v_nbr_label的顶点满足匹配，cand中增加label，可能更新self_LI
				add_CandAndSelfLI(v, val[0], u, v_nbr_label);
			}
		}
	}
}

void Match::add_CandAndSelfLI(int v, int Qi, int u, int vNbrLabel)
{
	auto& theSet = initG[v].cand.try_emplace(Qi).first->second.try_emplace(u).first->second;
	theSet.insert(vNbrLabel);
	if(theSet.size() == q[u].q_neiLabels.size())
	{
		// store_Changed_selfLI(v, Qi, u);
		changed_selfLI.at(v).try_emplace(Qi).first->second.insert(u);
		initG[v].self_LI.try_emplace(Qi).first->second.insert(u);
	}
}

void Match::addEdge_UpdateNbrLI_v_and_nbrs(int v, int v_nbr)
{
	/* ①只遍历changed selfli：v1新增的selfli，v1邻接点的nbrli、v1中nbrli相应更新
	   ②v1和v2的selfLI都需要判断是否在它们的nbrLi中对应增加，即v1的nbrli增加v2的selfli，v2的nbrli中增加v1的selfli*/

	for (auto& i : changed_selfLI.at(v))  //Qi = i.first, u_set = i.second
	{
		for (auto& u : i.second)
		{
			for (auto& uNbr : q[u].q_neiSet)
			{
				int label = uNbr.first;
				const set<int>& uNbrSet = uNbr.second;
				set<int> vnbr_set = initG[v].nei.at(label);
				vnbr_set.erase(v_nbr);  //跳过v_nbr
				for (auto& vNbr : vnbr_set)
				{
					if (initG[vNbr].self_LI.find(i.first) != initG[vNbr].self_LI.end())
					{
						const set<int>& vNbr_self = initG[vNbr].self_LI.at(i.first);
						vec vNbr_match_uNbr;
						set_intersection(uNbrSet.begin(), uNbrSet.end(), vNbr_self.begin(), vNbr_self.end(), back_inserter(vNbr_match_uNbr));
						if (!vNbr_match_uNbr.empty())
						{
							for (auto& matched_uNbr : vNbr_match_uNbr)
							{
								add_nbrLI(v, matched_uNbr, vNbr);//v中nbr_LI存vNbr
								add_nbrLI(vNbr, u, v);           //反存，vNbr中nbr_LI存v
							}
						}
					}
				}
			}
		}
	}
}

void Match::addEdge_UpdateNbrLI_vi_and_vj(int v, int v_nbr)
{
	int vnbr_label = initG[v_nbr].label;
	for (auto& i : initG[v].self_LI)
	{
		if (initG[v_nbr].self_LI.find(i.first) != initG[v_nbr].self_LI.end())
		{
			const set<int>& vNbr_self = initG[v_nbr].self_LI.at(i.first);
			// FYI std::unordered_map<int, std::unordered_map<int, vec>> stream_matched_edges; // 查询图id - u - u_nbrs
			// unordered_map<int, vec> temp;
			// stream_matched_edges.emplace(i.first, temp);

			auto it = stream_matched_edges.try_emplace(i.first).first;
			for (auto& u : i.second)
			{
				if (q[u].q_neiSet.find(vnbr_label) != q[u].q_neiSet.end())
				{
					const set<int>& u_vNbrL_nbrs = q[u].q_neiSet.at(vnbr_label);
					vec vNbr_match_uNbr;
					set_intersection(u_vNbrL_nbrs.begin(), u_vNbrL_nbrs.end(), vNbr_self.begin(), vNbr_self.end(), back_inserter(vNbr_match_uNbr));
					if (!vNbr_match_uNbr.empty())
					{
						it->second.emplace(u, vNbr_match_uNbr);
						for (auto& matched_uNbr : vNbr_match_uNbr)
						{
							add_nbrLI(v, matched_uNbr, v_nbr);//v中nbr_LI存vNbr
							add_nbrLI(v_nbr, u, v);           //反存，vNbr中nbr_LI存v
						}
					}
				}
			}

			if (it->second.size() == 0)
			{
				stream_matched_edges.erase(it);
			}
		}
	}
}

void Match::add_nbrLI(int v, int u_nbr, int v_nbr)
{
	initG[v].nbr_LI.try_emplace(u_nbr).first->second.insert(v_nbr);
}

//②删边
void Match::del_G_Nei(int v, int v_nbr)
{
	auto it = initG[v].nei.find(initG[v_nbr].label);
	auto& nbr = it->second;
	nbr.erase(v_nbr);
	if(nbr.empty())initG[v].nei.erase(it);
}

void Match::delEdge_UpdateCandAndSelfLI(int vi, int vj)
{
	//和加边不同的是，已知(vi,vj)匹配的所有(ui,uj)，但是不知道删边后vi不再匹配哪些ui，vj同理
	u_map temp;
	changed_selfLI.emplace(vi, temp);
	changed_selfLI.emplace(vj, temp);

	//判断vi
	auto it = initG[vi].nei.find(initG[vj].label);
	if (it != initG[vi].nei.end())
	{
		for (auto& i : stream_matched_edges)
		{
			for (auto& edge : i.second)
			{
				if (q[edge.first].q_neiSet.at(initG[vj].label).size() - it->second.size() == 1)
				{  //删除边(vi, vj)后，vi领域中label(vj)的顶点不满足匹配了，cand中减少label
					del_CandaAndSelfLI(vi, i.first, edge.first, initG[vj].label);
				}
			}
		}
	}
	else
	{
		for (auto& i : stream_matched_edges)
		{
			for (auto& edge : i.second)
			{
				if (q[edge.first].q_neiSet.at(initG[vj].label).size() == 1)
				{
					del_CandaAndSelfLI(vi, i.first, edge.first, initG[vj].label);
				}
			}
		}
	}

	//判断vj
	auto it2 = initG[vj].nei.find(initG[vi].label);
	if ( it2 != initG[vj].nei.end())
	{
		for (auto& i : stream_matched_edges)
		{
			for (auto& edge : i.second)
			{
				for (auto& uj : edge.second)
				{
					if (q[uj].q_neiSet.at(initG[vi].label).size() - it2->second.size() == 1)
					{  //删除边(vi, vj)后，vj领域中label(vi)的顶点不满足匹配了，cand中减少label
						del_CandaAndSelfLI(vj, i.first, uj, initG[vi].label);
					}
				}
			}
		}
	}
	else
	{
		for (auto& i : stream_matched_edges)
		{
			for (auto& edge : i.second)
			{
				for (auto& uj : edge.second)
				{
					if (q[uj].q_neiSet.at(initG[vi].label).size() == 1)
					{
						del_CandaAndSelfLI(vj, i.first, uj, initG[vi].label);
					}
				}
			}
		}
	}


	/*u_map temp;
	changed_selfLI.emplace(v, temp);
	multimap<int, vec>::iterator it1 = q_edges[initG[v].label].lower_bound(initG[v_nbr].label);
	multimap<int, vec>::iterator it2 = q_edges[initG[v].label].upper_bound(initG[v_nbr].label);
	for (; it1 != it2; it1++)
	{
		vec val = it1->second;
		long int thr = val.size();
		for (auto i = 1; i < thr; i++)    //val[0]是查询图id
		{
			int u = val[i];  //v可以匹配的u
			int v_nbr_label = initG[v_nbr].label;
			if (initG[v].nei.find(v_nbr_label) != initG[v].nei.end())
			{
				if (q[u].q_neiSet.at(v_nbr_label).size() - initG[v].nei.at(v_nbr_label).size() == 1)
				{  //删除边(v, v_nbr)后，v领域中label为v_nbr_label的顶点不满足匹配了，cand中减少label
					del_CandaAndSelfLI(v, val[0], u, v_nbr_label);
				}
			}
			else
			{
				if (q[u].q_neiSet.at(v_nbr_label).size() == 1)
				{  //删除边(v, v_nbr)后，v领域中label为v_nbr_label的顶点不满足匹配了，cand中减少label
					del_CandaAndSelfLI(v, val[0], u, v_nbr_label);
				}
			}
		}
	}*/
}

void Match::del_CandaAndSelfLI(int v, int Qi, int u, int vNbrLabel)
{
	
	unordered_map<int,set<int>>&  vCand = initG[v].cand.at(Qi);
	auto candIt = vCand.find(u);
	if (candIt != vCand.end())
	{
		auto& theSet = candIt->second;
		theSet.erase(vNbrLabel);
		if (theSet.size() == 0)
		{
			vCand.erase(candIt);
			if (vCand.size() == 0) {
				initG[v].cand.erase(Qi);
			}
		}
	}

	unordered_map<int,set<int>>& vSelf = initG[v].self_LI;//[QGID , {QVids...}]
	auto selfIt = initG[v].self_LI.find(Qi);
	if (selfIt != initG[v].self_LI.end())
	{
		set<int>& uSet = selfIt->second;
		auto uIt = uSet.find(u);
		if (uIt != uSet.end())
		{   //更新self_LI
			// store_Changed_selfLI(v, Qi, u);
			changed_selfLI.at(v).try_emplace(Qi).first->second.insert(u);
			uSet.erase(u);
			if (uSet.size() == 0)
			{
				vSelf.erase(selfIt);
			}
		}
	}
}

void Match::delEdge_update_nbrLI_v_and_nbrs(int v, int v_nbr)
{
	/*①遍历changed selfli：v1邻接点的nbrli(这里邻接点包括了v2)、v1中nbrli相应更新
	②(v1,v2)匹配的所有边，它们的nbrLi中对应删除，即v1的nbrli删除v2的selfli，v2的nbrli中删除v1的selfli*/
	for (auto& i : changed_selfLI.at(v))//i:[QGID u1 u2 u3]
	{
		u_set processed_uNbrs;
		u_set delNbr_of_u;
		auto& tempNbrLI = initG[v].nbr_LI;
		for (auto& u : i.second)  //v不再匹配u deleteEdge
		{
			for (auto& uNbr : q[u].q_neis)
			{
				if (tempNbrLI.find(uNbr) != tempNbrLI.end())
				{
					for (auto& vv : tempNbrLI.at(uNbr))
					{
						del_nbrLI(vv, u, v);   //反删
					}
				}

				if (processed_uNbrs.find(uNbr) == processed_uNbrs.end())
				{
					processed_uNbrs.emplace(uNbr);
					auto it = initG[v].self_LI.find(i.first);
					if (it != initG[v].self_LI.end())
					{
						vec comm;
						set_intersection(q[uNbr].q_neiSet.at(initG[v].label).begin(), q[uNbr].q_neiSet.at(initG[v].label).end(), it->second.begin(), it->second.end(), back_inserter(comm));
						if (comm.empty())
						{
							// delNbr_of_u.emplace(uNbr);  //需要正删
							tempNbrLI.erase(uNbr);
						}
					}
					else
					{
						// delNbr_of_u.emplace(uNbr);  //需要正删
						tempNbrLI.erase(uNbr);
					}
				}
			}
		}

		// for (auto& dn : delNbr_of_u)
		// {
		// 	tempNbrLI.erase(dn);
		// }
	}
}

void Match::delEdge_update_nbrLI_vi_and_vj(int vi, int vj)
{
	for (auto& i : stream_matched_edges)
	{
		for (auto& edge : i.second)  //key = ui
		{
			del_nbrLI(vj, edge.first, vi);

			for (auto& uj : edge.second)
			{
				del_nbrLI(vi, uj, vj);
			}
		}
	}
}

void Match::del_nbrLI(int v, int u_nbr, int v_nbr)
{
	auto& nbr_li = initG[v].nbr_LI;
	auto it = nbr_li.find(u_nbr);
	if (it != nbr_li.end())
	{
		auto& neighbors = it->second;
		neighbors.erase(v_nbr);
		if (neighbors.empty())
		{
			nbr_li.erase(it);
		}
	}
}


//搜索匹配结果
long long Match::searchMatch(int vi, int vj)
{
	//symGroupReflash();
	eachQManager.clear();
	eachQManager.resize(QGNum);//应该在每次进行匹配前清空
	long long update_result = 0;
		// std::cout<<"*****newEdge"<<vi<<" "<<vj<<"*****"<<std::endl;
	// coutAble = true;
	// if(!(vi==44&&vj==7288))return 0;
	//if(!(vi==326&&vj==22166))return 0;
	// if(vi>20000||vj>20000)return 0;
	 //if(vi<20000||vj<20000)return 0;
	//return 1;
	long long result_comm = 0,result_remain = 0;
	if (initG[vi].label < initG[vj].label){

		bool flag = checkEdgeInCommSub(vi, vj); 
		if (flag)
		{
			update_result += searchMatchedEdgeFromCommon(vi, vj);
			result_comm = update_result;
		}
		bool flag2 = checkEdgeInOrderTree(vi, vj);
		if (flag2)
		{
			update_result += searchMatchedEdgeFromRemainQ(vi, vj);
			result_remain = update_result - result_comm;
		}
	}
	else if (initG[vi].label == initG[vj].label)
	{
		bool flag = checkEdgeInCommSub(vi, vj);
		if (flag)
		{
			update_result += searchMatchedEdgeFromCommon(vi, vj);
			update_result += searchMatchedEdgeFromCommon(vj, vi);
			result_comm = update_result;
		}
		bool flag2 = checkEdgeInOrderTree(vi, vj);
		if (flag2)
		{
			update_result += searchMatchedEdgeFromRemainQ(vi, vj);
			update_result += searchMatchedEdgeFromRemainQ(vj, vi);
			result_remain = update_result - result_comm;
		}
	}
	else
	{
		bool flag = checkEdgeInCommSub(vj, vi);
		if (flag)
		{
			update_result += searchMatchedEdgeFromCommon(vj, vi);
			result_comm = update_result;
		}
		bool flag2 = checkEdgeInOrderTree(vj, vi);
		if (flag2)
		{
			update_result += searchMatchedEdgeFromRemainQ(vj, vi);
			result_remain = update_result - result_comm;
		}
	}
	matchFromComm += result_comm;
	matchFromRemain += result_remain;
	// if(update_result!=0&&true) 
	// {	
	// 	std::cout<<"*****newEdge"<<vi<<" "<<vj<<"*****"<<std::endl;
	// 	std::cout<<"update_result = "<<update_result<<"  "<<result_comm<<" | "<<result_remain<<std::endl;
	// }
	return update_result;
}

bool Match::checkEdgeInCommSub(int vi, int vj)
{
	if (CommSub.find(initG[vi].label) != CommSub.end()) 
	{
		if (CommSub.at(initG[vi].label).find(initG[vj].label) != CommSub.at(initG[vi].label).end()) 
		{    //公共子图中有可以匹配的查询边
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool Match::checkEdgeInOrderTree(int vi, int vj)
{
	if (orderTree.find(initG[vi].label) != orderTree.end())
	{
		if (orderTree.at(initG[vi].label).find(initG[vj].label) != orderTree.at(initG[vi].label).end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}




u_map Match::cmpInitCand(int vi, int vj, int ui, int uj)      //vi match ui, vj match uj
{   
	u_map cand;
	cand = genEdgeCand(vi, ui, vj, uj, cand);
	if (q[ui].q_neis.size() == 1 && cand.empty())
	{
		cand = genEdgeCand(vj, uj, vi, ui, cand);
	}
	if (!cand.empty())
	{
		cand = genEdgeCand(vj, uj, vi, ui, cand);
	}

	return cand;
}

bool Match::checkBidirectionalEdgeOnIndex(int vi, int vj, int ui, int uj)
{
	int q_id = q[ui].q_id;
	if (initG[vi].self_LI.find(q_id) != initG[vi].self_LI.end() && initG[vj].self_LI.find(q_id) != initG[vj].self_LI.end())
	{
		if (initG[vi].self_LI.at(q_id).find(ui) != initG[vi].self_LI.at(q_id).end() && initG[vj].self_LI.at(q_id).find(uj) != initG[vj].self_LI.at(q_id).end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

u_map Match::genEdgeCand(int v, int u, int v_nbr, int u_nbr, u_map cand)
{
	std::set<int> nbrs = q[u].q_neis;
	nbrs.erase(u_nbr);
	for (auto& j : nbrs)
	{
		if (initG[v].nbr_LI.find(j) != initG[v].nbr_LI.end())
		{
			std::set<int> temp = initG[v].nbr_LI.at(j);
			temp.erase(v_nbr);
			if (temp.empty())
			{
				cand.clear();
				break;
			}
			if (cand.find(j) != cand.end())
			{
				vec comm;
				set_intersection(cand.at(j).begin(), cand.at(j).end(), temp.begin(), temp.end(), back_inserter(comm));
				if (!comm.empty())
				{
					std::set<int> comm_set(comm.begin(), comm.end());
					cand.at(j) = comm_set;
				}
				else
				{
					cand.clear();
					break;
				}
			}
			else
			{
				cand.emplace(j, temp);
			}
		}
		else
		{
			cand.clear();
			break;
		}
	}
	return cand;
}

u_map Match::updateCand(bool joinOperation, int v, int u, u_map cand, std::unordered_map<int, int> match, std::set<int> used_v, int dif)
{
	//cand中删除其他u候选集中的v
	//selfLI中按照查询图分类地存储了诸多该数据点对应地查询点，所以现在我们要在已有的Match下
 	bool flag = false;
	if (cand.size() <= initG[v].self_LI.at(q[u].q_id).size())//这个是q2-qn的量要小于 v的selfLI
	{
		//std::cout<<"CandiNum < selfLI"<<endl;
		for (auto& i: cand)//i.first是查询图点
		{
			//q1匹配了d1，那么需要查询看看d1是否有q2,q3...   如果有，q2,q3需要删去d1
			if (initG[v].self_LI.at(q[u].q_id).find(i.first) != initG[v].self_LI.at(q[u].q_id).end())
			{
				cand.at(i.first).erase(v);
				if (cand.at(i.first).empty())
				{
					cand.clear();
					flag = true;
					break;
				}
			}
		}
	}
	else//这个是q2-qn的量要多于v的selfLI
	{
		for (auto& i : initG[v].self_LI.at(q[u].q_id))//对于每个在v的selfLI中，如果cand中包含这个i，那么就删去这个cand中的v
		{
			//std::cout<<"CandiNum > selfLI"<<endl;
			if (cand.find(i) != cand.end())
			{
				cand.at(i).erase(v);
				if (cand.at(i).empty())
				{
					cand.clear();
					flag = true;
					break;
				}
			}
		}
	}


	if (flag)
	{
		//cout<<"update cand ---- 1"<<endl;
		return cand;
	}
	else
	{   //cand中增加v的领域匹配u的领域
		set<int> u_adjs;
		if (joinOperation)
		{  //join剩余图中的更新cand操作
			u_adjs = RemainQ_adjList.at(q[u].q_id).at(u);
		}
		else
		{  //普通匹配过程中的更新cand操作
			u_adjs = q[u].q_neis;
		}

		for (auto& u : u_adjs)
		{	
			int i = 0;
			if (joinOperation)
			{
				i = u - dif;
				if (match.find(i) == match.end())//i是u的公共子图偏置，需要i在Comm没匹配过
				{ 
					if (match.find(u) != match.end())
					{   //i已经在match中被匹配过了
						continue;
					}
				}
				else
				{
					continue;
				}
			}
			else
			{
				i = u;
				if (match.find(i) != match.end())
				{
					continue;
				}
			}
			
			if (joinOperation)
			{
				i = i + dif;//i变回u
			}
			else
			{
				i = u;
			}

			if (initG[v].nbr_LI.find(i) == initG[v].nbr_LI.end())
			{
				//对于其中一个u（函数传入的那个U）的邻居i没有在nbrLi中，则此点不可选，清空返回
				//print_umap(initG[v].nbr_LI);//打印要点注释
				cand.clear();
				break;
			}
			else
			{
				vec rest_c;//留下不存在于used中的NbrLi
				set_difference(initG[v].nbr_LI.at(i).begin(), initG[v].nbr_LI.at(i).end(), used_v.begin(), used_v.end(), back_inserter(rest_c));
				if (rest_c.empty())
				{
					cand.clear();
					break;
				}
				else
				{
					std::set<int> rest_c_set(rest_c.begin(), rest_c.end());
					if (cand.find(i) != cand.end())//如果cand包含这个i，说明有cand了
					{
						vec comm;//留下存在于restC中的Cand
						set_intersection(cand.at(i).begin(), cand.at(i).end(), rest_c_set.begin(), rest_c_set.end(), back_inserter(comm));
						if (comm.empty())
						{
							cand.clear();
							break;
						}
						else
						{
							std::set<int> temp(comm.begin(), comm.end());
							cand.erase(i);
							cand.emplace(i, temp);
						}
					}
					else
					{
						cand.erase(i);
						cand.emplace(i, rest_c_set);
					}
				}
				//到这里才算成功完成了一个新点的Cand计算
			}
		}
	}
	// std::cout<<"Try Output cand in updateCand"<<endl;
	// for(auto& i: cand)
	// {
	// 	std::cout<<"("<<i.first<<":";
	// 	for(auto each : i.second)std::cout<<each<<" ";
	// 	std::cout<<")"<<endl;
	// }
	// std::cout<<"Finish Output cand in updateCand"<<endl;
	return cand;
}

u_map Match::cmpJoinedInitCand(int q_id, std::unordered_map<int, int> match, std::set<int> used_v, int dif)
{
	u_map cand;
	for (auto& i : RemainQ_initialCandU.at(q_id))
	{
		if (match.find(i.first-dif) == match.end())
		{
			bool flag = false;
			for (auto& j : i.second)
			{   //遍历每个u,把它们领域满足匹配i.first的候选集合取交
				int u = j - dif;
				if (initG[match.at(u)].nbr_LI.find(i.first) != initG[match.at(u)].nbr_LI.end())
				{
					vec rest_c;
					set_difference(initG[match.at(u)].nbr_LI.at(i.first).begin(), initG[match.at(u)].nbr_LI.at(i.first).end(), used_v.begin(), used_v.end(), back_inserter(rest_c));
					if (!rest_c.empty())
					{
						std::set<int> rest_c_set(rest_c.begin(), rest_c.end());
						if (cand.find(i.first) != cand.end())
						{
							vec comm;
							set_intersection(cand.at(i.first).begin(), cand.at(i.first).end(), rest_c_set.begin(), rest_c_set.end(), back_inserter(comm));
							if (!comm.empty())
							{
								std::set<int> comm_set(comm.begin(), comm.end());
								cand.at(i.first) = comm_set;
							}
							else{
								flag = true;
								break;
							}
						}
						else{
							cand.emplace(i.first, rest_c_set);
						}
					}
					else{
						flag = true;
						break;
					}
				}
				else{
					flag = true;
					break;
				}
			}
			if (flag){
				cand.clear();
				break;
			}
		}
	}

	return cand;
}








