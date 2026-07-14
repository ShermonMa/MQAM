#ifndef _TYPE_H
#define _TYPE_H

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <vector>
#include <set>
#include <string>
using namespace std;

struct vertex_Q
{
    int id;
    int q_label;
    int q_id;
    int comm_id;          //公共子图id
    std::set<int> q_neis; //邻接点集合
    std::unordered_set<int> q_neiLabels;    //邻接点label集合
    std::unordered_map<int, std::set<int>> q_neiSet;       //邻接点label - 邻接点id集合
    std::vector<int> ngbrsLabelCnt;
};
//data graph
struct vertex_G
{
    int label; //匹配顶点是label，不匹配是-1
    std::unordered_map<int, std::set<int>> nei; //label - 邻接点id
    std::unordered_map<int, std::unordered_map<int, std::set<int>>> cand;//匹配查询图id - 匹配查询点 - 查询点邻域label (每个label对应的邻接点集都已匹配)
    std::unordered_map<int, std::set<int>> self_LI; //查询图id - 可匹配的查询点集合  (在index中，表示该顶点有出边)
    std::unordered_map<int, std::set<int>> nbr_LI; //领域查询点 - 匹配领域查询点的邻接点集合  (在index中，表示该顶点与某个邻接点之间有双向边)
};




#endif