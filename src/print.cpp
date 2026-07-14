#include "graph.h"
#include "match.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <set>
#include <memory>
#include "symmetry.h"
#include <stack>
using u_map = std::unordered_map<int,std::set<int>>;

//print
void printMatchCand(std::unordered_map<int,std::set<int>> cand, bool flag) {
    if(cand.empty())
    {
        std::cout<<"MatchCand is Empty"<<endl;
        return;
    }
    if(flag)std::cout << "---------------------printMatchCand----------------------" << std::endl;
    for (auto it = cand.begin(); it != cand.end(); it++) {
        std::cout << it->first << " : ";
        for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
            std::cout << *it2 << " ";
        }
        std::cout << std::endl;
    }
    if(flag)std::cout << "===================printMatchCand=================" << std::endl;
}
// void printSymmetryNgbrGroup(SymmetryRecord &sr)
// {
//     std::cout<<"symmetryNgbrGroup的个数："<<sr.symmetryNgbrGroup.size()<<std::endl;
//     for(auto [behalf , group] : sr.symmetryNgbrGroup)
//     {
//         if(behalf == sr.ngbrSymRepresenter[behalf-sr.range.first])
//         {
//             std::cout<<"symmetryNgbrGroup代表："<<behalf<<" 对应的symmetryNgbrGroup："<<std::endl;
//             for(auto each : *group)
//             {
//                 std::cout<<each<<" ";
//             }
//             std::cout<<endl;
//         }
//     }
// }




void Graph::printSymmetryRecord(int QGraphID)
{
    if(QSym[QGraphID].matchNum == 1)
    {
        std::cout<<"图"<<QGraphID<<"不存在自同构"<<endl;
        return;
    }
    std::cout<<"图"<<QGraphID<<"自同构个数："<<QSym[QGraphID].matchNum<<std::endl;
    
    // for(int i = 0 ; i < QSym[QGraphID].allSymmetryOutCome.size(); i++)
    // {
    //     std::cout<<"第"<<i+1<<"个自同构："<<std::endl;
    //     for(int j = 0 ; j < QSym[QGraphID].allSymmetryOutCome[i].size(); j++)
    //     {
    //         std::cout<<QSym[QGraphID].allSymmetryOutCome[i][j]<<", ";
    //     }
    //     std::cout<<std::endl;
    // }
    // std::cout<<"每个结点的对称替代有："<<std::endl;
    // for(int i = 0 ; i < QSym[QGraphID].eachVertexCandidate.size(); i++)
    // {
        
    //     for(int j = 0 ; j < QSym[QGraphID].eachVertexCandidate[i].size(); j++)
    //     {
    //         std::cout<<QSym[QGraphID].eachVertexCandidate[i][j]<<", ";
    //     }
    //     std::cout<<std::endl;
    // }
}
void Match::print_v(int vi)
{
	cout << "vi = " << vi << endl;
	//cout << "vi=" << vi << ", label=" << initG[vi].label << ", nbr:{";
	/*for (auto& ne : initG[vi].nei)
	{
		cout << "label=" << ne.first << "-[";
		for (auto& nb : ne.second)
		{
			cout << nb << ",";
		}
		cout << "], ";
	}
	cout << "}" << endl;

	for (auto& ca : initG[vi].cand)
	{
		cout << "cand: q_id=" << ca.first << ": ";
		for (auto& can : ca.second)
		{
			cout << "match u=" << can.first << ", u_nbr label={";
			for (auto& can_lb : can.second)
			{
				cout << can_lb << ",";
			}
			cout << "}" << endl;
		}
	}

	for (auto& se : initG[vi].self_LI)
	{
		cout << "self_LI: q_id=" << se.first << " v_Match_u={";
		for (auto& sel : se.second)
		{
			cout << sel << ",";
		}
		cout << "}" << endl;
	}*/

	for (auto& nb : initG[vi].nbr_LI)
	{
		cout << "u_nbr=" << nb.first << ", v_nbr={";
		for (auto& nbr : nb.second)
		{
			cout << nbr << ",";
		}
		cout << "}" << endl;
	}
	cout << endl;
}

void Match::print_time()
{
	//cout << "updateindex 1 cost (ms):" << time_slot1_1 << endl;
	//cout << "updateindex 2 cost (ms):" << time_slot1_2 << endl;
	//cout << "updateindex 3 cost (ms):" << time_slot1_3 << endl;
	cout << "update index total cost (ms):" << time_slot1 << endl;

	//cout << "searchmatch ---checkEdgeNbr--- (ms):" << time_slot2_checkEdgeNbr << endl;
	//cout << "searchmatch ---removeEdge_updateQ--- (ms):" << time_slot2_removeEdge_updateQ << endl;
	//cout << "searchmatch ---checkNodeNbr--- (ms):" << time_slot2_checkNodeNbr << endl;
	//cout << "searchmatch ---removeNode_updateQ--- (ms):" << time_slot2_removeNode_updateQ << endl;
	//cout << "search match --- generate match order --- (ms):" << time_slot1_generate_match_order << endl;
	//cout << "search match --- count_Match --- (ms):" << time_slot2_count_Match << endl;
	cout << "search match total cost (ms):" << time_slot2 << endl;
	
	//cout << "count_match --- major 1 --- (ms):" << time_slot3_1 << endl;
	//cout << "count_match --- major 2 --- (ms):" << time_slot3_2 << endl;
	//cout << "count_match --- minor (ms):" << time_slot3_3 << endl;
}

void Match::print_unordered_map(std::unordered_map<int, int> p)
{
	//cout << "------ print_unodered_map ------" << endl;
	std::map<int, int> m;
	for (auto& k : p)
	{
		m.emplace(k.first, k.second);
	}
	for (auto& k : m)
	{
		cout << "(" << k.first << " - " << k.second << ")";
	}
	//cout << endl;
}

void Match::print_umap(u_map p)
{
	//cout << "-------------print_umap ---------" << endl;
	for (auto& k : p)
	{
		cout << k.first << "-{";
		for (auto& d : k.second)
		{
			cout << d << ", ";
		}
		cout << "}" << endl;
	}
	cout << endl;
}

void Match::print_multimap(std::multimap<int, int> p)
{
	cout << "-------------print_multimap ---------" << endl;
	for (auto& k : p)
	{
		cout << "(" << k.first << " - " << k.second << ")";
	}
	cout << endl;
}

void Match::print_set(std::set<int> s)
{
	for (auto& u: s)
	{
		cout << u << ", ";
	}
	cout << endl;
}

void Match::printOrderCommSub(two_umap_twovec orderCommSub)
{
	for (auto& [edgeOne,Left] : orderCommSub)
	{
		for(auto& [edgeTwo,twoVector]:Left)
		{
			std::cout<<"edgeTwo: "<<edgeOne<<" - "<<edgeTwo<<endl;
			for(auto& i :twoVector)
			{
				for(auto& j : i)
				{
					std::cout<<j<<" ";
				}
				std::cout<<endl;
			}
		}
	}
}