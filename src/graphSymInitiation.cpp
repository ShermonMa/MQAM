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
#include <numeric> // std::iota
#include <string.h>
using u_map = std::unordered_map<int,std::set<int>>;

void Graph::ReInitiateQueryVertices()
{
    for(int i = 0 ; i < Q.size() ; i++)
    {
        vertex_Q& cur = Q[i];
        for(int eachNgbr : cur.q_neis)
        {
            int label = Q[eachNgbr].q_label;
            if(cur.ngbrsLabelCnt.size() >label){
                if(cur.ngbrsLabelCnt[label] == 0)cur.ngbrsLabelCnt[0]++;
                cur.ngbrsLabelCnt[label]++;
            }
            else{
                while(cur.ngbrsLabelCnt.size() < label ){
                    cur.ngbrsLabelCnt.push_back(0);
                }
                cur.ngbrsLabelCnt.push_back(1);
                cur.ngbrsLabelCnt[0]++;
            }
        }
    }
    QSym.reserve(QGNum);
    symRepresenter.symmetryNgbrRepresenter = new int[Q.size()];
    symRepresenter.symmetryRepresenter = new int[Q.size()];
    int cur = 0;
    for(int i = 0 ; i < QGNum; i++)
    {
        selfSearch(i);
    }
    //TODO :: symmetryRepresenter的构成

}

void Graph::selfSearch(int QGraphID)
{
    //std::cout<<"正在进行图"<<QGraphID<<"的自同构查询"<<std::endl;
    const int start = range[QGraphID*2];
    const int end = range[QGraphID*2+1];
    // std::cout<<"range:"<<start<<" - "<<end<<std::endl;
    // SymmetryRecord newOne(start,end);
    QSym.push_back(SymmetryRecord(start,end));

    std::vector<bool> flag(end - start + 1,0);
    std::vector<int> outcome(end - start + 1,-1);
    backTrack(start,Q_adjList.at(QGraphID),flag,outcome,std::make_pair(start,end));

    // 输出匹配结果数量
    // printSymmetryRecord(QGraphID);
    symmetryNodeGrouplize(QSym[QGraphID]);

}


bool Graph::isAutomorphism(const std::unordered_map<int,std::set<int>>& adjList, 
        std::vector<int>& outcome,std::pair<int,int> range)
{
    for(const auto& [vertexID, ngbrs] : adjList)
    {
        int mappedVertex = outcome[vertexID - range.first];
        std::set<int> newNgbrs;
        for(int ngbr : ngbrs)
        {
            newNgbrs.insert(outcome[ngbr - range.first]);
        }
        if(adjList.at(outcome[vertexID - range.first]) != newNgbrs)
        {
            return false;
        }
    }
    return true;
}
void Graph::backTrack(int nodeIndex, const std::unordered_map<int,std::set<int>>& adjList, 
    std::vector<bool>& usedFlag, std::vector<int>& outcome,std::pair<int,int> range)
{
    if(nodeIndex > range.second)
    {
        if(isAutomorphism(adjList,outcome,range))
        {
            SymmetryRecord& currentSym  = QSym[QSym.size()-1];
            currentSym.allSymmetryOutCome.push_back(outcome);
            for(int i = 0 ; i < outcome.size(); i++)
            {
                currentSym.eachVertexCandidate[i].push_back(outcome[i]);
                //currentSym.toggle.push_back(true);
            }
            currentSym.matchNum++;
        }
    }

    for(int i = 0; i <= range.second - range.first; i++)
    {
        if(usedFlag[i] == false && Q[i + range.first].q_label == Q[nodeIndex].q_label)
        {
            if(sameFeature(Q[i + range.first].ngbrsLabelCnt,Q[nodeIndex].ngbrsLabelCnt))
            {
                usedFlag[i] = true;
                outcome[nodeIndex - range.first] = i+range.first;
                backTrack(nodeIndex+1,adjList,usedFlag,outcome,range);
                outcome[nodeIndex - range.first] = -1;
                usedFlag[i] = false;
            }

        }
    }
}

bool Graph::sameFeature(std::vector<int> aNgbr, std::vector<int> bNgbr)
{
    if(aNgbr.size()!= bNgbr.size())return false;
    for(int i = 0 ; i < aNgbr.size(); i++)
    {
        if(bNgbr[i] != aNgbr[i])return false;
    }
    return true;
}

//本函数主要负责更新两个数据结构isSymmetryVertex和symmetryGroup
void Graph::symmetryNodeGrouplize(SymmetryRecord &sr)
{
    if(sr.matchNum==1)return;
    int nodeNum = sr.nodeNum;
    int rangeS = sr.rangeS;
    sr.ngbrSymRepresenter = new int[nodeNum];
    std::memset(sr.ngbrSymRepresenter, -1, nodeNum * sizeof(int));

    sr.symmetryRepresenter = new int[nodeNum];
    std::memset(sr.symmetryRepresenter, -1, nodeNum * sizeof(int));
    for(int i = 0; i<nodeNum; i++)
    {
        //std::cout<<"NewNode"<<sr.originGraph[i]<<std::endl;
        //已经因为同类匹配点而引入了symmetryGroup
        std::set<int> symGroup ;
        if(sr.symmetryRepresenter[i] != -1)continue;
        int minimunV = sr.originGraph[i];
        //get in set
        for(const int & eachSymV : sr.eachVertexCandidate[i])
        {
            if(eachSymV != i+rangeS)
            {
                if(eachSymV < minimunV)minimunV = eachSymV;
                symGroup.insert(eachSymV);
            }
        }
        if(symGroup.size()<=1)continue;
        symGroup.insert(sr.originGraph[i]);
        std::shared_ptr<std::set<int>> groupPtr = std::make_shared<std::set<int>>(symGroup);
        sr.symmetryGroup.emplace(minimunV,groupPtr);
        for(const auto& eachSymV : symGroup)
        {
            sr.symmetryRepresenter[eachSymV-rangeS] = minimunV;
            if(eachSymV == minimunV)continue;
            sr.symmetryGroup.emplace(eachSymV,groupPtr);
        }
    }

    //symmetryNgbrGrouplize(sr); 我选择内联到一起了
    vector<int> traverseFlag(nodeNum,0);
    for(int i = 0; i<nodeNum; i++)
    {
        if(traverseFlag[i]==1)continue;
        if(sr.symmetryRepresenter[i]==-1)continue;
        int nonNgbrRepre = sr.symmetryRepresenter[i];//一定是本组最小值点',,一个对称组可能会被切分成多个小组
        int representer = i+rangeS;
        bool isSelfConnected = false;
        bool havingCommon = false;
        set<int> ngbrSym;
        for(int j = i+1; j<nodeNum; j++)//补齐对称点
        {
            if(sr.symmetryRepresenter[j]==-1)continue;
            if(sr.symmetryRepresenter[j]==nonNgbrRepre)
            {
                int newNode = sr.originGraph[j];
                bool ngbrJudge_selfConnected = false;
                bool ngbrJudge_common = false;
                set<int> commNode;
                if(Q[newNode].q_neis.find(representer)!=Q[newNode].q_neis.end())
                {
                    ngbrJudge_selfConnected = true;
                    isSelfConnected = true;
                }
                std::set_intersection(Q[representer].q_neis.begin(),Q[representer].q_neis.end(),
                    Q[newNode].q_neis.begin(),Q[newNode].q_neis.end(),
                    std::inserter(commNode,commNode.begin()));
                if(commNode.size()>0)
                {
                    ngbrJudge_common = true;
                    havingCommon = true;
                }
                if(ngbrJudge_selfConnected || ngbrJudge_common)
                {
                    ngbrSym.insert(newNode);
                    traverseFlag[j] = 1;
                } 
            }
        }
        ngbrSym.insert(i+rangeS);
        if(ngbrSym.size()<=1)continue;
        SymmetryNgbrGroup ngbrGroup;
        ngbrGroup.isSelfConnected = isSelfConnected;
        // ngbrGroup.havingCommon = havingCommon;
        // ngbrGroup.representer = representer;
        ngbrGroup.vertices = ngbrSym;

        //然后是关键点
        // set<int> crucialPoints = Q[representer].q_neis;
        sr.ngbrSymRepresenter[representer - rangeS] = representer;
        for(auto each : ngbrSym)
        {
            sr.ngbrSymRepresenter[each - rangeS] = representer;
            // set<int> tempPoints;
            // std::set_intersection(crucialPoints.begin(),crucialPoints.end(),Q[each].q_neis.begin(),Q[each].q_neis.end(),
            //     std::inserter(tempPoints,tempPoints.begin()));
            // crucialPoints = std::move(tempPoints);
        }
        //试图去除having common字段
        // ngbrGroup.crucialPoints = std::move(crucialPoints);
        // if(ngbrGroup.crucialPoints.size()==0)ngbrGroup.havingCommon = false;
        // else ngbrGroup.havingCommon = true;
        // if(!havingCommon && !isSelfConnected)
        // {
        //     std::cout<<"错误：本对称组既没有自连接，也没有公共点"<<endl;
        // }
        // if(!havingCommon&& ngbrGroup.vertices.size()==2)
        // {std::cout<<"本对称组无公共点且仅有两点自连接，过滤此无关对称组"<<endl;continue;}
        
        std::shared_ptr<SymmetryNgbrGroup> ngbrGroupPtr = std::make_shared<SymmetryNgbrGroup>(ngbrGroup);
        sr.ngbrGroup.emplace(representer,ngbrGroupPtr);
        for(auto ngbr : ngbrSym)
        {
            sr.ngbrGroup.emplace(ngbr,ngbrGroupPtr);
        }
    }

    //同步到symRepresenter
    memcpy(symRepresenter.symmetryNgbrRepresenter + rangeS,
        sr.ngbrSymRepresenter,
        nodeNum * sizeof(int));
    memcpy(symRepresenter.symmetryRepresenter + rangeS,
        sr.symmetryRepresenter,
        nodeNum * sizeof(int));
    // for(int i = rangeS ; i<= sr.rangeE ; i++)
    // {
    //     symRepresenter.symmetryNgbrRepresenter[i] = sr.ngbrSymRepresenter[i-rangeS];
    //     symRepresenter.symmetryRepresenter[i] = sr.symmetryRepresenter[i-rangeS];
    // }
}


void Graph::computeStartMulti(bool isComm,int id,int ui,int uj)
{

    //已经是合法边了
    if(isComm)//计算Comm匹配顺序来的
    {
        int qNumsForAComm = Q_adjList.size()/commSubgraph.size();
        int eachQNums = Q_adjList.begin()->second.size();
        int enableEdgeCnt = 0;
        for(int i = 0 ; i < qNumsForAComm; i++)
        {
            int dif = (qNumsForAComm*(id-1) - Q[ui].q_id+i)*eachQNums;
            int newUi = ui + dif, newUj = uj + dif;
            if(QSym[Q[newUi].q_id].matchNum == 1)
            {
                enableEdgeCnt++;
                continue;
            }
            SymmetryRecord& sr = QSym[Q[newUi].q_id];
            int rangeS = sr.rangeS;
            if(!isLegalEdge_Comm(newUi,newUj))
            {
                sr.forbidden[newUi].insert(newUj);
                continue;
            }
            bool uiSym = isSymmetryVertex(newUi);
            bool ujSym = isSymmetryVertex(newUj);
            int multi = 1;
            if(!uiSym && !ujSym)
            {
                multi == 1;
            }
            else if(uiSym && ujSym)
            {
                multi*= sr.symmetryGroup.at(newUi)->size();
                int repreJ = sr.symmetryRepresenter[newUj-rangeS];
                int adj = 0;
                for(auto each : Q[newUi].q_neis)
                {
                    if(sr.symmetryRepresenter[each-rangeS] == repreJ)adj++;
                }
                multi*=adj;
            }
            else
            {
                int symPoint,commPoint;
                if(uiSym){symPoint = newUi;commPoint = newUj;}
                else { symPoint = newUj;commPoint = newUi;}
                int repre = sr.symmetryRepresenter[symPoint-rangeS];
                int adj = 0;
                for(auto each : Q[commPoint].q_neis)
                {
                    if(sr.symmetryRepresenter[each-rangeS] == repre)adj++;
                }
                multi*=adj;
            }
            enableEdgeCnt++;
            sr.startMulti[newUi][newUj] = multi;
        }
        if(enableEdgeCnt == 0)
        {
            symRepresenter.forbidStart[ui].insert(uj);
        }
    }
    else//计算Remain匹配顺序来的
    {
        if(QSym[Q[ui].q_id].matchNum == 1)return;
        bool uiSym = isSymmetryVertex(ui);
        bool ujSym = isSymmetryVertex(uj);
        int multi=1;
        SymmetryRecord& sr = QSym[Q[ui].q_id];
        int rangeS = sr.rangeS;
        if(!uiSym && !ujSym)
        {
            multi == 1;
        }
        else if(uiSym && ujSym)
        {
            multi*= sr.symmetryGroup.at(ui)->size();
            int repreJ = sr.symmetryRepresenter[uj-rangeS];
            int adj = 0;
            for(auto each : Q[ui].q_neis)
            {
                if(sr.symmetryRepresenter[each-rangeS] == repreJ)adj++;
            }
            multi*=adj;
        }
        else
        {
            int symPoint,commPoint;
            if(uiSym){symPoint = ui;commPoint = uj;}
            else { symPoint = uj;commPoint = ui;}
            int repre = sr.symmetryRepresenter[symPoint-rangeS];
            int adj = 0;
            for(auto each : Q[commPoint].q_neis)
            {
                if(sr.symmetryRepresenter[each-rangeS] == repre)adj++;
            }
            multi*=adj;
        }
        sr.startMulti[ui][uj] = multi;
    }
}
//用于利用对称除去部分无用的匹配顺序
void Graph::orderCutRemain()
{
    //labelEdgeOfRemainingQ是我们要处理的主角
    //label(ui) - label(uj) - ui - adjs 
    unordered_map<int,unordered_map<int,unordered_map<int,set<int>>>> newLabelEdgeOfRemainingQ = labelEdgeOfRemainingQ;
    for(auto [labelUi,luj] : labelEdgeOfRemainingQ)
    {
        for(auto [labelUj,uiAdj]:luj)
        {
            for(auto [ui,ujs] : uiAdj)
            {
                for(auto uj : ujs)
                {
                    if(!isLegalEdge_Remain(ui,uj))
                    {
                        newLabelEdgeOfRemainingQ[labelUi][labelUj][ui].erase(uj);
                        if(newLabelEdgeOfRemainingQ[labelUi][labelUj][ui].empty())
                        {
                            //std::cout<<"Error: symmetry.cpp:orderCutRemain() 删除无用，按理不会进入这里"<<endl;
                            //std::cout<<"labelUi"<<labelUi<<" LabelUj:"<<labelUj<< "ui:"<<ui<<" uj:"<<uj<<endl;
                            newLabelEdgeOfRemainingQ[labelUi][labelUj].erase(ui);
                            if(newLabelEdgeOfRemainingQ[labelUi][labelUj].empty())
                            {
                                newLabelEdgeOfRemainingQ[labelUi].erase(labelUj);
                                if(newLabelEdgeOfRemainingQ[labelUi].empty())
                                {
                                    newLabelEdgeOfRemainingQ.erase(labelUi);
                                }
                            
                            }
                        } 
                    }
                }
            }
        }
    }
    labelEdgeOfRemainingQ = newLabelEdgeOfRemainingQ;
}
bool Graph::isLegalEdge_Comm(int ui,int uj)
{
    int q_id = Q[ui].q_id;
    if(QSym[q_id].matchNum == 1) return true;
    int offset = QSym[q_id].rangeS;
    const SymmetryRecord& sr = QSym[q_id];
    bool iSymV = isSymmetryVertex(ui);
    bool jSymV = isSymmetryVertex(uj);
    if(!iSymV && !jSymV)return true;
    else if(iSymV && jSymV)
    {
        int representer_i = sr.symmetryRepresenter[ui-offset];
        int representer_j = sr.symmetryRepresenter[uj-offset];
        if(representer_i!=ui && representer_j!=uj)return false;
        else if(representer_i == ui && representer_j == uj)return true;
        else
        {
            if(representer_i == representer_j)return ui==representer_i;
            int smaller_representer = min(representer_i,representer_j);
            if(ui == smaller_representer||uj==smaller_representer)
            {
                if(ui == smaller_representer)//ui是代表点，则需要其与uj代表点不相邻
                {
                    if(Q[ui].q_neis.find(representer_j)!=Q[ui].q_neis.end())return false;
                    else return true;
                }
                else
                {
                    if(Q[uj].q_neis.find(representer_i)!=Q[uj].q_neis.end())return false;
                    else return true;
                }
            }
            else return false;
        }
    }
    else
    {
        int commPoint,symPoint;
        if(iSymV)
        {
            commPoint = uj;
            symPoint = ui;
        }
        else
        {
            commPoint = ui;
            symPoint = uj;
        }
        if(sr.symmetryRepresenter[symPoint-offset]!=symPoint)//此对称点必须是代表点，不是返回false
        {
            return false;
        }
        else return true;
    }
    return true;
}
bool Graph::isLegalEdge_Remain(int ui, int uj)//注意一个致命点，在Graph剪枝edge时，对于同label数据点搜两遍没有过滤能力
{
    //试图提出定理，两对称点的邻居，要不然是同一点，要不然就是两个对称点
    //规则
    //都不是对称，安全
    //都对称
        //必须存在至少一个Representer
            //都是R true
            //其一是R ，则对于两点，这个R必须是两点的R的小的R，且和大R不相邻
    //其一对称--》推理另一个点一定是公共点
        //必须是Representer
            //两点的边不在公共部分
    int q_id = Q[ui].q_id;
    if(QSym[q_id].matchNum == 1) return true;
    int offset = QSym[q_id].rangeS;
    const SymmetryRecord& sr = QSym[q_id];
    bool iSymV = isSymmetryVertex(ui);
    bool jSymV = isSymmetryVertex(uj);
    if(!iSymV && !jSymV)return true;
    else if(iSymV && jSymV)
    {
        int representer_i = sr.symmetryRepresenter[ui-offset];
        int representer_j = sr.symmetryRepresenter[uj-offset];
        if(representer_i!=ui && representer_j!=uj)return false;
        else if(representer_i == ui && representer_j == uj)return true;
        else
        {
            if(representer_i == representer_j)return ui==representer_i;
            int smaller_representer = min(representer_i,representer_j);
            if(ui ==smaller_representer||uj==smaller_representer)
            {
                if(ui == smaller_representer)
                {
                    if(Q[ui].q_neis.find(representer_j)!=Q[ui].q_neis.end())return false;
                    else return true;
                }
                else
                {
                    if(Q[uj].q_neis.find(representer_i)!=Q[uj].q_neis.end())return false;
                    else return true;
                }
            }
            else return false;
        }
    }
    else
    {
        int commPoint,symPoint;
        if(iSymV)
        {
            commPoint = uj;
            symPoint = ui;
        }
        else
        {
            commPoint = ui;
            symPoint = uj;
        }
        if(sr.symmetryRepresenter[symPoint-offset]!=symPoint)
        {
            return false;
        }
        
        else
        {
            if(isCommEdge(commPoint,symPoint))
            {
                return false;
                //是代表点，但是本代表点和非对称点是comm中的边，则不可以
                std::cout<<"是代表点，但是本代表点和非对称点是comm中的边，则不可以  的情况按理来说不会发生吧，但还是发生了"<<endl;
            }
            else return true;
        }
    }
    std::cout<<"似乎有漏掉的判断逻辑条件"<<endl;
    return true;
}
void Graph::symProcess()
{
    //generateQSymVInComm();
    orderCutRemain();
    prepareSymData();
}
void Graph::prepareSymData()
{
    symGroupData.reserve(QGNum);
    for(int i = 0 ; i < Q_adjList.size();i++)
    {
        SymGroupData newData;
        newData.rangeS = QSym[i].rangeS;
        newData.rangeE = QSym[i].rangeE;
        newData.ngbrGroup = QSym[i].ngbrGroup;
        newData.matchNum = QSym[i].matchNum;
        newData.startMulti = QSym[i].startMulti;
        newData.forbidden = QSym[i].forbidden;
        symGroupData.push_back(newData);
    }
    // std::vector<SymmetryRecord>().swap(QSym);
}
// //本函数用于提取每个在comm中的对称点
// void Graph::generateQSymVInComm()
// {
// 	int numQForAComm = Q_adjList.size() / (Q.end() - 1)->comm_id;
// 	int numNodesForAQ = Q_adjList.begin()->second.size();
//     std::vector<std::set<int>> QSymVInComm;
//     QSymVInComm.resize(Q_adjList.size());
//     for(int i = 0 ; i < Q_adjList.size(); i+=numQForAComm)
//     {
//         const u_map& theCommSubGraph = commSubgraph.at(i/numQForAComm+1);//仅考虑了公共部分为每组第一个图的
//         for(int j = 0; j<numQForAComm; j++)//i即初始变量，j是偏移量
//         {
//             QNgbrSymVInComm.push_back(std::set<int>());
//             if(QSym[i+j].matchNum==1)continue;
//             for(auto each : theCommSubGraph)
//             {
//                 if(isNgbrSymmetryVertex(each.first+numNodesForAQ*j))
//                 {
//                     QNgbrSymVInComm[i+j].insert(each.first+numNodesForAQ*j);
//                 }
//                 if(isSymmetryVertex(each.first+numNodesForAQ*j))
//                 {
//                     QSymVInComm[i+j].insert(each.first+numNodesForAQ*j);
//                 }
//             }
//         }
//     }
//     for(int i = 0 ; i< QNgbrSymVInComm.size(); i++)
//     {
//         if(QNgbrSymVInComm.empty())continue;
//         int offset = QSym[i].range.first;
//         for( int each : QNgbrSymVInComm[i])
//         {
//             const int groupLeader = QSym[i].ngbrSymRepresenter[each-offset];
//             if(QSym[i].symNgbrGroupVInComm.find(groupLeader) != QSym[i].symNgbrGroupVInComm.end())
//             {
//                 QSym[i].symNgbrGroupVInComm[groupLeader].insert(each);
//             }
//             else
//             {
//                 QSym[i].symNgbrGroupVInComm.emplace(groupLeader,std::set<int>({each}));
//             }


//             const int nonNgbrSymLeader = QSym[i].symmetryRepresenter[each-offset];
//             if(QSym[i].symGroupVInComm.find(nonNgbrSymLeader) != QSym[i].symGroupVInComm.end())
//             {
//                 QSym[i].symGroupVInComm[nonNgbrSymLeader].insert(each);
//             }
//             else
//             {
//                 QSym[i].symGroupVInComm.emplace(nonNgbrSymLeader,std::set<int>({each}));
//             }
            
//         }
//     }
// }





bool Graph::isCommEdge(int ui,int uj)
{
    //std::cout<<"In isCommEdge"<<endl;
    int q_ui_label = Q[ui].q_label;
    int q_uj_label = Q[uj].q_label;
    if(q_ui_label>q_uj_label){swap(ui,uj);swap(q_ui_label,q_uj_label);}
    if(q_ui_label == q_uj_label)
    {
        if(ui>uj){swap(ui,uj);}
    }
    int q_id = Q[ui].q_id;
    int comm_id = Q[ui].comm_id;
    int numQForAComm = Q_adjList.size() / (Q.end() - 1)->comm_id;
	int numNodesForAQ = Q_adjList.begin()->second.size();
    int offset = (q_id - (comm_id-1)*numQForAComm)*numNodesForAQ;

    //TODO 使用ABSL：：absl::flat_hash_map 或 boost::container::flat_map
    auto it1 = labelEdgeOfCommSub.find(q_ui_label);
    if (it1 == labelEdgeOfCommSub.end()) return false;
    auto it2 = it1->second.find(q_uj_label);
    if (it2 == it1->second.end()) return false;
    auto it3 = it2->second.find(ui - offset);
    if (it3 == it2->second.end()) return false;
    auto it4 = it3->second.find(uj - offset);
    return it4 != it3->second.end();
    
    // if(labelEdgeOfCommSub.find(Q[ui].q_label) == labelEdgeOfCommSub.end())return false;
    // if(labelEdgeOfCommSub[Q[ui].q_label].find(Q[uj].q_label) == labelEdgeOfCommSub[Q[ui].q_label].end())return false;
    // if(labelEdgeOfCommSub[Q[ui].q_label].at(Q[uj].q_label).find(ui-offset)==labelEdgeOfCommSub[Q[ui].q_label].at(Q[uj].q_label).end())return false;
    // if(labelEdgeOfCommSub[Q[ui].q_label].at(Q[uj].q_label).at(ui-offset).find(uj-offset)==labelEdgeOfCommSub[Q[ui].q_label].at(Q[uj].q_label).at(ui-offset).end())return false;
    // return true;
}
