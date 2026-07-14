#include "graph.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <string.h>
//query graph
void Graph::inputQ(const std::string& path)
{
    ifstream qg(path);
    if (!qg) cerr << "Fail to open query file." << endl;

    char type = 'n';
    int comm_sub_num = 0, gen_q_num = 0, comm_thr_e_num = 0, cnt_commSub = 0, cnt_commE = 0;
    bool store_comm = false;
    int query_id = 0, nodes_num = 0, num = 0;
    int id = 0, lb = 0;
    int id1 = 0, id2 = 0, weight = 0;
    vertex_Q u;
    while (qg >> type) {
        if (type == 'c'){
            qg >> comm_sub_num >> gen_q_num >> comm_thr_e_num;
        }
        else if (type == 't'){
            // std::cout<<"new QG"<<endl;
            qg >> query_id >> nodes_num;
            range[2*QGNum] = num;
            num += nodes_num;
            range[2*QGNum+1] = num - 1;
            QGNum++;
            u.q_id = query_id;
            if (query_id % gen_q_num == 0){
                store_comm = true;  //存一次公共部分
                cnt_commSub += 1;   //!! 第几个公共部分,从1作为起始公共部分
                cnt_commE = 0;      //公共部分的第几条边
                u_map temp;
                commSubgraph.emplace(cnt_commSub, temp);
            }
            u.comm_id = cnt_commSub;
        }
        else if (type == 'v'){
            qg >> id >> lb;
            u.q_label = lb;
            u.id = id;
            Q.emplace_back(u);
        }
        else{
            qg >> id1 >> id2 >> weight;
            Q[id1].q_neis.insert(id2);
            Q[id2].q_neis.insert(id1);
            Q[id1].q_neiLabels.insert(Q[id2].q_label);
            Q[id2].q_neiLabels.insert(Q[id1].q_label);            
            
            if (store_comm)
            {
                if (cnt_commE < comm_thr_e_num){
                    storeCommSubgraph(cnt_commSub, id1, id2);
                    storeCommSubgraph(cnt_commSub, id2, id1);
                    storeLabelEdge(true, id1, id2);
                    cnt_commE += 1;
                }
                else{
                    store_comm = false;
                }
            }
        }
    }
    qg.close();
    // std::cout<<"QGNUM"<<QGNum<<endl;
    //存Q_adjList和q_labelEdge
    for (int Qi = 0 ; Qi < QGNum; Qi++)
    {
        u_map temp;
        Q_adjList.emplace(Qi, temp);
        int bg = range[2*Qi];
        int en = range[2*Qi+1];

        for (auto i = bg; i <= en; i++){
            for (auto& u_nbr : Q[i].q_neis){
                //计算同label对应的邻接点集合
                int uNbr_Label = Q[u_nbr].q_label;
                if (Q[i].q_neiSet.find(uNbr_Label) != Q[i].q_neiSet.end()){
                    Q[i].q_neiSet.at(uNbr_Label).insert(u_nbr);
                }
                else{
                    set<int> temp;
                    temp.insert(u_nbr);
                    Q[i].q_neiSet.emplace(uNbr_Label, temp);
                }


                //计算(Label(ui),Label(uj))对应的查询图集合
                int ui_Label = Q[i].q_label;
                if (q_labelEdge.find(ui_Label) != q_labelEdge.end()){
                    if (q_labelEdge[ui_Label].find(uNbr_Label) != q_labelEdge[ui_Label].end()){
                        bool unStore = true;  //该查询图未存储过(Label(ui),Label(uj))对应的vector
                        for (auto   beg = q_labelEdge[ui_Label].lower_bound(uNbr_Label), 
                                    end = q_labelEdge[ui_Label].upper_bound(uNbr_Label); beg != end; ++beg)
                        {
                            if (beg->second.front() == Qi){
                                beg->second.emplace_back(i);
                                unStore = false;
                                break;
                            }
                        }
                        if (unStore){
                            vec temp;
                            temp.emplace_back(Qi);
                            temp.emplace_back(i);
                            q_labelEdge[ui_Label].emplace(uNbr_Label, temp);
                        }
                    }
                    else{
                        vec temp;
                        temp.emplace_back(Qi);
                        temp.emplace_back(i);
                        q_labelEdge[ui_Label].emplace(uNbr_Label, temp);
                    }
                }
                else{
                    vec temp;
                    temp.emplace_back(Qi);
                    temp.emplace_back(i);
                    std::multimap<int, vec> temp2;
                    temp2.insert(make_pair(uNbr_Label, temp));
                    q_labelEdge.emplace(ui_Label, temp2);
                }
            }
            Q_adjList.at(Qi).emplace(i, Q[i].q_neis);
        }
    }
    // std::cout<<"InputQComplete"<<endl;

    /*cout << "----------------------commSubgraph---------------------" << endl;
    print_two_umap(commSubgraph);
    cout << "-------------------------------------------------------" << endl;*/
    ReInitiateQueryVertices();

}

void Graph::storeCommSubgraph(int commSubId,int u1, int u2)
{
    if (commSubgraph.at(commSubId).find(u1) != commSubgraph.at(commSubId).end()){
        commSubgraph.at(commSubId).at(u1).emplace(u2);
    }
    else{
        std::set<int> temp;
        temp.emplace(u2);
        commSubgraph.at(commSubId).emplace(u1, temp);
    }
}

void Graph::storeLabelEdge(bool flag, int u1, int u2)
{
    if (Q[u1].q_label < Q[u2].q_label){
        addLabelEdge(flag, u1, u2);
    }
    else if (Q[u1].q_label == Q[u2].q_label)
    {
        if (u1 < u2){
            addLabelEdge(flag, u1, u2);
        }
        else{
            addLabelEdge(flag, u2, u1);
        }
    }
    else if (Q[u1].q_label > Q[u2].q_label){
        addLabelEdge(flag, u2, u1);
    }
}

void Graph::addLabelEdge(bool flag, int u1, int u2)
{
    if (flag)
    {   //添加到labelEdgeOfCommSub
        if (labelEdgeOfCommSub.find(Q[u1].q_label) != labelEdgeOfCommSub.end())
        {
            if (labelEdgeOfCommSub.at(Q[u1].q_label).find(Q[u2].q_label) != labelEdgeOfCommSub.at(Q[u1].q_label).end())
            {
                if (labelEdgeOfCommSub.at(Q[u1].q_label).at(Q[u2].q_label).find(u1) != labelEdgeOfCommSub.at(Q[u1].q_label).at(Q[u2].q_label).end())
                {
                    labelEdgeOfCommSub.at(Q[u1].q_label).at(Q[u2].q_label).at(u1).emplace(u2);
                }
                else{
                    std::set<int> temp;
                    temp.emplace(u2);
                    labelEdgeOfCommSub.at(Q[u1].q_label).at(Q[u2].q_label).emplace(u1, temp);
                }
            }
            else{
                std::set<int> temp;
                temp.emplace(u2);
                u_map temp2;
                temp2.emplace(u1, temp);
                labelEdgeOfCommSub.at(Q[u1].q_label).emplace(Q[u2].q_label, temp2);
            }
        }
        else{
            std::set<int> temp;
            temp.emplace(u2);
            u_map temp2;
            temp2.emplace(u1, temp);
            two_u_map temp3;
            temp3.emplace(Q[u2].q_label, temp2);
            labelEdgeOfCommSub.emplace(Q[u1].q_label, temp3);
        }
    }
    else
    {   //添加到labelEdgeOfRemainingQ
        if (labelEdgeOfRemainingQ.find(Q[u1].q_label) != labelEdgeOfRemainingQ.end())
        {
            if (labelEdgeOfRemainingQ.at(Q[u1].q_label).find(Q[u2].q_label) != labelEdgeOfRemainingQ.at(Q[u1].q_label).end())
            {
                if (labelEdgeOfRemainingQ.at(Q[u1].q_label).at(Q[u2].q_label).find(u1) != labelEdgeOfRemainingQ.at(Q[u1].q_label).at(Q[u2].q_label).end())
                {
                    labelEdgeOfRemainingQ.at(Q[u1].q_label).at(Q[u2].q_label).at(u1).emplace(u2);
                }
                else{
                    std::set<int> temp;
                    temp.emplace(u2);
                    labelEdgeOfRemainingQ.at(Q[u1].q_label).at(Q[u2].q_label).emplace(u1, temp);
                }
            }
            else{
                std::set<int> temp;
                temp.emplace(u2);
                u_map temp2;
                temp2.emplace(u1, temp);
                labelEdgeOfRemainingQ.at(Q[u1].q_label).emplace(Q[u2].q_label, temp2);
            }
        }
        else{
            std::set<int> temp;
            temp.emplace(u2);
            u_map temp2;
            temp2.emplace(u1, temp);
            two_u_map temp3;
            temp3.emplace(Q[u2].q_label, temp2);
            labelEdgeOfRemainingQ.emplace(Q[u1].q_label, temp3);
        }
    }
}

void Graph::traverseRemainingQ(u_map remain_Q)
{
    for (auto& i: remain_Q)
    {
        for (auto& j: i.second)
        {
            if (Q[i.first].q_label <= Q[j].q_label)
            {
                storeLabelEdge(false, i.first, j);
            }
        }
    }
}


//data graph
void Graph::inputG(const std::string& path)
{
    ifstream dg(path);
    if (!dg) cerr << "Fail to open graph file." << endl;

    char c;
    int v_num, e_num, id, lb, id1, id2, weight;
    vertex_G v;
    int id_c = 0;
    while (dg >> c) {
        if (c == 't'){
            dg >> v_num >> e_num;
        }
        else if (c == 'v'){
            dg >> id >> lb;
            if (id_c != id) {
                for (; id_c < id; ++id_c) {
                    v.label = -1;
                    G.emplace_back(v);
                }
            }
            /* if (id_c != id) {
                 std::cout << "---" << id_c << " " << id << std::endl;
                 exit(0);
             }*/
            ++id_c;
            if (q_labelEdge.find(lb) != q_labelEdge.end()){
                v.label = lb;
            }
            else{
                v.label = -1;  // 与查询结点无关的数据结点可以跳过
            }
            G.emplace_back(v);
        }
        else{
            dg >> id1 >> id2 >> weight;
            if (q_labelEdge.find(G[id1].label) != q_labelEdge.end())
            {
                if (q_labelEdge.at(G[id1].label).find(G[id2].label) != q_labelEdge.at(G[id1].label).end())
                {
                    //只存储匹配查询边的数据边
                    if (G[id1].nei.find(G[id2].label) != G[id1].nei.end()){
                        G[id1].nei.at(G[id2].label).emplace(id2);
                    }
                    else{
                        set<int> temp;
                        temp.emplace(id2);
                        G[id1].nei.emplace(G[id2].label, temp);
                    }

                    if (G[id2].nei.find(G[id1].label) != G[id2].nei.end()){
                        G[id2].nei.at(G[id1].label).emplace(id1);
                    }
                    else{
                        set<int> temp;
                        temp.emplace(id1);
                        G[id2].nei.emplace(G[id1].label, temp);
                    }
                }
            }
            break;
        }
    }
    while (dg >> c) {
        dg >> id1 >> id2 >> weight;

        if (q_labelEdge.find(G[id1].label) != q_labelEdge.end()){
            if (q_labelEdge.at(G[id1].label).find(G[id2].label) != q_labelEdge.at(G[id1].label).end()){
                //只存储匹配查询边的数据边
                if (G[id1].nei.find(G[id2].label) != G[id1].nei.end()){
                    G[id1].nei.at(G[id2].label).emplace(id2);
                }
                else{
                    set<int> temp;
                    temp.emplace(id2);
                    G[id1].nei.emplace(G[id2].label, temp);
                }

                if (G[id2].nei.find(G[id1].label) != G[id2].nei.end()){
                    G[id2].nei.at(G[id1].label).emplace(id1);
                }
                else{
                    set<int> temp;
                    temp.insert(id1);
                    G[id2].nei.emplace(G[id1].label, temp);
                }
            }
        }
    }
    dg.close();
}

// stream
void Graph::inputUpdate(const std::string& path)
{
    // 读取更新流数据，存储时加边v1、v2是正值，删边v1、v2是负值
    int capacity = Config::UPDATE_BLOCK_SIZE;
    int v1, v2, weight;
    update = new int[capacity];
    ifstream infile(path);
    string c;
    int andCnt = 0;
    while (infile >> c >> v1 >> v2 >> weight) {
        if (updateSize * 2 + 2 > capacity) {
            capacity += Config::UPDATE_BLOCK_SIZE*(andCnt+1);
            andCnt++;
            int* temp = new int[capacity];
            memcpy(temp, update, updateSize * 2 * sizeof(int));
            delete[] update;
            update = temp;
        }
        if (c == "e"){
            // std::cout<<"NEWEDGE:"<<v1<<" "<<v2<<endl;
            update[updateSize*2] = v1;
            update[updateSize*2+1] = v2;
            updateSize++;
        }
        else if (c == "-e"){
            // std::cout<<"NEWEDGE:"<<-v1<<" "<<-v2<<endl;
            update[updateSize*2] = -v1;
            update[updateSize*2+1] = -v2;
            updateSize++;
        }

    } 
}

//order
void Graph::genOrder()
{
    //计算公共区域的所有匹配顺序
    cmpOrderOfCommSub();

    // cout << "----------------------OrderOfCommSub---------------------" << endl;
    // print_two_umap_twovec(OrderOfCommSub);
    // cout << "-------------------------------------------------------" << endl;


    //计算除去公共区域后，每个查询图的匹配顺序
    int s = Q_adjList.size() / commSubgraph.size();
    int comm_id = 0;
    int sum_q = Q_adjList.size();
    for (int i = 0; i < sum_q; i++)
    {
        comm_id = i / s + 1;  //第comm_id个公共子图, q_id = i
        u_map remain_Q = removeCommSubFromQ(comm_id, i);
        traverseRemainingQ(remain_Q);

        int dif = cmpDifBtw_ActualId_And_IdInCommSub(comm_id, i);
        std::set<int> matched;
        for (auto& i : commSubgraph.at(comm_id))
        {
            int actual_id = i.first + dif;
            matched.emplace(actual_id);
        }
        cmpOrderOfRemainingQ(comm_id, remain_Q, matched);
    }
    
    // cout << "--------------------OrderOf Remaining Q------------------" << endl;
    // print_umap_twovec(OrderOfRemainQ);
    // cout << "-------------------------------------------------------" << endl;


    //计算匹配边在非公共区域时的匹配顺序
    cmpOrderOfEdgeOfRemainQ();
    //提前结束 关闭输出 要点！
    //return;
    // cout << "---------------------- OrderOfEdgeOfRemainQ ------------------" << endl;
    // print_two_umap_twovec(OrderOfEdgeOfRemainQ);
    // cout << "-------------------------------------------------------" << endl;

    // cout << "---------------------- odTree_startNodes ---------------------" << endl;
    // print_two_umap(odTree_startNodes);
    // cout << "-------------------------------------------------------" << endl;

    // cout << "---------------------- orderTree ---------------------" << endl;
    // print_tri_u_map(orderTree);
    // cout << "-------------------------------------------------------" << endl;

    // cout << "---------------------- od_node ---------------------" << endl;
    // for (int i=0; i< odTree_nodes.size(); i++)
    // {
    //     cout << "od_node = " << i << ", u_set: ";
    //     print_vec(odTree_nodes[i].u_set);
    //     cout << endl;
    // }
    // cout << "-------------------------------------------------------" << endl;
}



//compute order of common subgraph
void Graph::cmpOrderOfCommSub()
{
    for (auto& i: commSubgraph)      //commSubID=i.first 
    {
        for (auto& j: i.second)      //ui=j.first
        {
            for (auto& k : j.second) //uj = k;
            {
                //看起来这里在生成匹配顺序的时候是按照升序的，第一序Label 第二序ID，其余过滤
                if (Q[j.first].q_label < Q[k].q_label)
                {
                    computeStartMulti(true, i.first, j.first, k);
                    std::set<int> matched;
                    matched.emplace(j.first);
                    matched.emplace(k);
                    u_map adj = i.second;
                    cmpOrderOf_UiUj(true, j.first, k, 0, adj, matched);
                }
                
                if (Q[j.first].q_label == Q[k].q_label && j.first < k)
                {
                    computeStartMulti(true, i.first, j.first, k);
                    std::set<int> matched;
                    matched.emplace(j.first);
                    matched.emplace(k);
                    u_map adj = i.second;
                    cmpOrderOf_UiUj(true, j.first, k, 0, adj, matched);
                }
            }
        }
    }
}

void Graph::cmpOrderOf_UiUj(bool start, int ui, int uj, int u, u_map adj, std::set<int> matched)
{
    if (start)
    {
        u_map updateAdj = removeAnEdgeFromQ(ui, uj, adj);
        vec major1 = cmpMajor(ui, updateAdj, matched);
        vec major2 = cmpMajor(uj, updateAdj, matched);
        vec finalMajor = cmpFinalMajor(major1, major2);
        if (!finalMajor.empty())
        {
            u_map updatedAdj1 = removeMatchedNodeFromQ(ui, adj);
            u_map updatedAdj2 = removeMatchedNodeFromQ(uj, updatedAdj1);
            int next_u = finalMajor[0];
            matched.emplace(next_u);
            addOrderOfCommSub(true, ui, uj, next_u);
            cmpOrderOf_UiUj(false, ui, uj, next_u, updatedAdj2, matched);
        }
        else
        {
            addOrderOfCommSub(false, ui, uj, 0);   // false表示需要往order里添加minor顶点
        }
    }
    else
    {
        vec major = cmpMajor(u, adj, matched);
        if (!major.empty())
        {
            u_map updateAdj = removeMatchedNodeFromQ(u, adj);
            int next_u = major[0];
            matched.emplace(next_u);
            addOrderOfCommSub(true, ui, uj, next_u);
            cmpOrderOf_UiUj(false, ui, uj, next_u, updateAdj, matched);
        }
        else
        {
            addOrderOfCommSub(false, ui, uj, 0);
        }
    }
}

vec Graph::cmpMajor(int u, u_map adj, std::set<int> matched)
{
    vec major;
    int max_degree = 0;
    int maxDegreeNode = 0;
    if (adj.find(u) != adj.end())
    {
        for (auto& i : adj.at(u))
        {
            vec dif;
            set_difference(adj.at(i).begin(), adj.at(i).end(), matched.begin(), matched.end(), back_inserter(dif));
            if (!dif.empty())
            {
                if (dif.size() > max_degree)
                {
                    max_degree = dif.size();
                    maxDegreeNode = i;
                }
            }
        }
    }
    if (max_degree != 0)
    {
        major.emplace_back(maxDegreeNode);   //major[0] = next_u, major[1] = max_degree
        major.emplace_back(max_degree);
    }
    return major;
}

vec Graph::cmpFinalMajor(vec major1, vec major2)
{
    vec result;
    if (major1.empty())
    {
        if(!major2.empty())
        {
            result.emplace_back(major2[0]);
        }
    }
    else
    {
        if (!major2.empty())
        {
            if (major1[1] < major2[1])
            {
                result.emplace_back(major2[0]);
            }
            else
            {
                result.emplace_back(major1[0]);
            }
        }
        else
        {
            result.emplace_back(major1[0]);
        }
    }

    return result;
}

void Graph::addOrderOfCommSub(bool flag, int ui, int uj, int u)
{
    if (flag)
    {   //添加major顶点
        if (OrderOfCommSub.find(ui) != OrderOfCommSub.end())
        {
            if (OrderOfCommSub.at(ui).find(uj) != OrderOfCommSub.at(ui).end())
            {
                for (auto& i: OrderOfCommSub.at(ui).at(uj))
                {
                    if (i[1] == 0)          
                    {
                        i[0] = u;
                        i[1] = 1;
                        break;
                    }
                }
            }
            else
            {
                int num_u = commSubgraph.at(Q[ui].comm_id).size() - 2;
                two_vec tv(num_u, vec(2));
                tv[0][0] = u;
                tv[0][1] = 1;
                OrderOfCommSub.at(ui).emplace(uj, tv);
            }
        }
        else
        {
            int num_u = commSubgraph.at(Q[ui].comm_id).size() - 2;
            two_vec tv(num_u, vec(2));
            tv[0][0] = u;
            tv[0][1] = 1;
            std::unordered_map<int, two_vec> temp;
            temp.emplace(uj, tv);
            OrderOfCommSub.emplace(ui, temp);
        }
    }
    else
    {   //添加minor顶点
        if (OrderOfCommSub.find(ui) != OrderOfCommSub.end())
        {
            if (OrderOfCommSub.at(ui).find(uj) != OrderOfCommSub.at(ui).end())
            {
                cmpMinorForCommSub(true, ui, uj);
            }
            else
            {   
                int num_u = commSubgraph.at(Q[ui].comm_id).size() - 2;
                two_vec tv(num_u, vec(2));
                OrderOfCommSub.at(ui).emplace(uj, tv);
                cmpMinorForCommSub(false, ui, uj);
            }
        }
        else
        {
            int num_u = commSubgraph.at(Q[ui].comm_id).size() - 2;
            two_vec tv(num_u, vec(2));
            std::unordered_map<int, two_vec> temp;
            temp.emplace(uj, tv);
            OrderOfCommSub.emplace(ui, temp);
            cmpMinorForCommSub(false, ui, uj);
        }
    }
}

u_map Graph::removeMatchedNodeFromQ(int u, u_map adj)
{
    
    if (adj.find(u) != adj.end())
    {
        if(adj.at(u).find(u)!=adj.at(u).end())
        {
            adj.at(u).erase(u);
        }
        for (auto& i : adj.at(u))
        {   
            adj.at(i).erase(u);
            if (adj.at(i).size() == 0)
            {
                adj.erase(i);
            }
        }
        adj.erase(u);
    }
    return adj;
}

u_map Graph::removeAnEdgeFromQ(int ui, int uj, u_map adj)
{
    adj.at(ui).erase(uj);
    adj.at(uj).erase(ui);
    if (adj.at(ui).empty())
    {
        adj.erase(ui);
    }
    if (adj.at(uj).empty())
    {
        adj.erase(uj);
    }

    return adj;
}

void Graph::cmpMinorForCommSub(bool exist, int ui, int uj)
{
        if (exist)    //order中有major顶点
    {
        set<int> matched;
        for (auto& i : OrderOfCommSub.at(ui).at(uj))
        {
            if (i[1] == 1)
            {
                matched.emplace(i[0]);
            }
        }
        matched.emplace(ui);
        matched.emplace(uj);
        u_set unmatched;
        for (auto& j : commSubgraph.at(Q[ui].comm_id))
        {
            if (matched.find(j.first) == matched.end())
            {
                unmatched.insert(j.first);
            }
        }
        int bg_minor = 0;
        for (auto& k : OrderOfCommSub.at(ui).at(uj))
        {
            bg_minor += 1;
            if (k[1] == 0)
            {
                break;
            }
        }
        int end_size = OrderOfCommSub.at(ui).at(uj).size();
        for (int i = bg_minor - 1; i < end_size; i++)
        {
            auto it = unmatched.begin();
            while (it != unmatched.end()){
                int a = *it;
                const auto& neis = commSubgraph.at(Q[ui].comm_id).at(*it);
                vector<int> result;
                set_intersection(neis.begin(), neis.end(), matched.begin(), matched.end(), back_inserter(result));
                if (!result.empty()){
                    OrderOfCommSub.at(ui).at(uj)[i][0] = *it;
                    OrderOfCommSub.at(ui).at(uj)[i][1] = 0;
                    matched.insert(*it);
                    unmatched.erase(it);
                    break;
                }
                else{
                    it++;
                }
            }
        }
    }
    else   //order中无major顶点
    { 
        vec unmatched;
        for (auto& i : commSubgraph.at(Q[ui].comm_id))
        {
            if (i.first != ui && i.first != uj)
            {
                unmatched.emplace_back(i.first);
            }
        }
        int num_minor = commSubgraph.at(Q[ui].comm_id).size() - 2;
        for (int i = 0; i < num_minor; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                if (j == 0)
                {
                    OrderOfCommSub.at(ui).at(uj)[i][j] = unmatched[0];
                    unmatched.erase(unmatched.begin());
                }
                else
                {
                    OrderOfCommSub.at(ui).at(uj)[i][j] = 0;
                }
            }
        }
    }
}

u_map Graph::removeCommSubFromQ(int comm_id, int q_id)
{
    u_map remain_q = Q_adjList.at(q_id);
    int dif = cmpDifBtw_ActualId_And_IdInCommSub(comm_id, q_id);
    for (auto& i: commSubgraph.at(comm_id))
    {
        for (auto& j: i.second)
        {
            int actual_i = i.first + dif;
            int actual_j = j + dif;
            if (remain_q.find(actual_i) != remain_q.end())
            {
                remain_q.at(actual_i).erase(actual_j);
                if (remain_q.at(actual_i).size() == 0)
                {
                    remain_q.erase(actual_i);
                }
            }
        }
    }
    
    //存储每个查询图的除去公共部分的剩余图
    remainQ_adjList.emplace(q_id, remain_q);

    //计算每个剩余查询图计算匹配时的初始候选集合,若初始候选集为空,即不需要进行join了
    for (auto& i: remain_q)
    {
        u_map temp;
        remainQ_initialCandU.emplace(q_id, temp);
        set<int> u_set;
        for (auto& j: i.second)
        {
            int comm_j = j - dif;
            if (commSubgraph.at(comm_id).find(comm_j) != commSubgraph.at(comm_id).end())
            {
                u_set.emplace(j);
            }
        }
        if (!u_set.empty())
        {
            remainQ_initialCandU.at(q_id).emplace(i.first, u_set);
        }
    }
    
    return remain_q;
}

int Graph::cmpDifBtw_ActualId_And_IdInCommSub(int comm_id, int q_id)
{
    int bg_q_id = Q[commSubgraph.at(comm_id).begin()->first].q_id;
    int dif = (q_id - bg_q_id) * Q_adjList.at(q_id).size();
    return dif;
}


//compute order of the remaining query graph
void Graph::cmpOrderOfRemainingQ(int comm_id, u_map adj, std::set<int> matched)
{
    std::multimap<int, int> major = cmpNextMajorFromRemainingQ(comm_id, adj, matched);//在剩余图上,与公共区域连接的major顶点
    if (!major.empty())
    {
        int next_u = major.rbegin()->second;  // 取度最大且为major
        matched.emplace(next_u);
        u_map updateAdj = removeMatchedNodeFromQ(next_u, adj);
        addOrderOfRemainingQ(true, next_u, matched);
        cmpOrderOfRemainingQ(comm_id, updateAdj, matched);
    }
    else
    {
        addOrderOfRemainingQ(false, *matched.begin(), matched);
    }
}

std::multimap<int, int> Graph::cmpNextMajorFromRemainingQ(int comm_id, u_map adj, std::set<int> matched)
{
    std::multimap<int, int> sort;    // 领域中未匹配顶点数量 - u
    for (auto& i: adj)
    {
        if (matched.find(i.first) == matched.end())
        {
            vec dif;
            set_difference(i.second.begin(), i.second.end(), matched.begin(), matched.end(), back_inserter(dif));
            vec same;
            set_intersection(Q[i.first].q_neis.begin(), Q[i.first].q_neis.end(), matched.begin(), matched.end(), back_inserter(same));
            if (!same.empty() && !dif.empty())
            {   //既与匹配过的顶点是连接的，又是major顶点
                sort.emplace(dif.size(), i.first);  //key升序
            }
        }
    }
    return sort;
}

void Graph::addOrderOfRemainingQ(bool flag, int u, std::set<int> matched)
{
    if (OrderOfRemainQ.find(Q[u].q_id) == OrderOfRemainQ.end())
    {
        int num_u = Q_adjList.at(Q[u].q_id).size() - commSubgraph.at(Q[u].comm_id).size();
        two_vec tv(num_u, vec(2));
        OrderOfRemainQ.emplace(Q[u].q_id, tv);
    }

    if (flag)
    {   //添加major顶点
        for (auto& i: OrderOfRemainQ.at(Q[u].q_id))
        {
            if (i[1] == 0)
            {
                i[0] = u;
                i[1] = 1;
                break;
            }
        }
    }
    else
    {   //添加minor顶点
        int dif = cmpDifBtw_ActualId_And_IdInCommSub(Q[u].comm_id, Q[u].q_id);
        vec unmatched;
        for (auto& i : Q_adjList.at(Q[u].q_id))
        {
            int actual_i = i.first - dif;
            if (commSubgraph.at(Q[u].comm_id).find(actual_i) == commSubgraph.at(Q[u].comm_id).end() && matched.find(i.first) == matched.end())
            {
                unmatched.emplace_back(i.first);
            }
        }
        int num_u = Q_adjList.at(Q[u].q_id).size() - commSubgraph.at(Q[u].comm_id).size();
        for (int i = num_u - unmatched.size(); i < num_u; i++)
        {
            OrderOfRemainQ.at(Q[u].q_id)[i][0] = unmatched[0];
            OrderOfRemainQ.at(Q[u].q_id)[i][1] = 0;
            unmatched.erase(unmatched.begin());
        }
    }
}


//compute order of starting edge in the remaining query graph
void Graph::cmpOrderOfEdgeOfRemainQ()
{
    for (auto& i: labelEdgeOfRemainingQ)  
    {
        for (auto& j: i.second)
        {   //!!!! 特殊情况，当 j.second.size() == 1, k.second.size() == 1时, 即该label边对应的边数量只有一条时(也就是该边在所有查询图中非公共边), 
        //匹配顺序里，没有选取最大度的major作为下一个匹配顶点,这里没有进行判断. 而是按照有公共情况的方式处理的
            for (auto& k: j.second)          //ui = k.first , uj = h
            {
                for (auto& h : k.second)
                {
                    // if( i.first == 3 && j.first == 9)
                    // {
                    //     std::cout<<"Find it"<<endl;
                    //     std::cout<<"Find it"<<endl;
                    // }
                    if(!isLegalEdge_Remain(k.first, h))continue;
                    computeStartMulti(false, k.first, k.first, h);
                    updateAuxiliary_Matched(true, k.first, h, h);
                    u_map adj = Q_adjList.at(Q[k.first].q_id);
                    u_map updatedAdj = removeAnEdgeFromQ(k.first, h, adj);
                    updateAuxiliary_Adj(true, k.first, h, updatedAdj);
                    cmpMajorAndMinor(k.first, h, k.first);// major中每个label只留度最大的顶点, label - u
                    cmpMajorAndMinor(k.first, h, h);
                    u_map updateAdj2 = removeMatchedNodeFromQ(k.first, updatedAdj);
                    u_map updateAdj3 = removeMatchedNodeFromQ(h, updateAdj2);
                    updateAuxiliary_Adj(true, k.first, h, updateAdj3);
                }
            }
            
            //(i.first, j.first)对应的所有查询边, key为major/minor集合中中包含该label的查询边数量, value为label
            multimap<int, int> nextLabelOfMajor = divideNextByLabel("major");
            multimap<int, int> nextLabelOfMinor = divideNextByLabel("minor");
            generateOrderOfEdgeOfRemainQ(nextLabelOfMajor, nextLabelOfMinor);//对每个查询边生成一个order
            summaryAllOrder(i.first, j.first); //生成查询树
            Auxiliary.clear();
        }
    }
}

void Graph::updateAuxiliary_Matched(bool start, int ui, int uj, int u)
{
    if (start)
    {   //start为真表示起始匹配边
        set<int> matched;
        matched.emplace(ui);
        matched.emplace(uj);
        if (Auxiliary.find(ui) != Auxiliary.end())
        {
            if (Auxiliary.at(ui).find(uj) != Auxiliary.at(ui).end())
            {
                Auxiliary.at(ui).at(uj).matched = matched;
            }
            else
            {
                Aux temp;
                temp.matched = matched;
                Auxiliary.at(ui).emplace(uj, temp);
            }
        }
        else
        {
            Aux temp;
            temp.matched = matched;
            unordered_map<int, Aux> temp2;
            temp2.emplace(uj, temp);
            Auxiliary.emplace(ui, temp2);
        }
    }
    else
    {
        Auxiliary.at(ui).at(uj).matched.emplace(u);
    }
}

void Graph::updateAuxiliary_Adj(bool start, int ui, int uj, u_map adj)
{
    if (start)
    {   //start为真表示起始匹配边
        if (Auxiliary.find(ui) != Auxiliary.end())
        {
            if (Auxiliary.at(ui).find(uj) != Auxiliary.at(ui).end())
            {
                Auxiliary.at(ui).at(uj).adj = adj;
            }
            else
            {
                Aux temp;
                temp.adj = adj;
                Auxiliary.at(ui).emplace(uj, temp);
            }
        }
        else
        {
            Aux temp;
            temp.adj = adj;
            unordered_map<int, Aux> temp2;
            temp2.emplace(uj, temp);
            Auxiliary.emplace(ui, temp2);
        }
    }
    else
    {
        Auxiliary.at(ui).at(uj).adj = adj;
    }
}

void Graph::cmpMajorAndMinor(int ui,int uj,int u)
{
    if (Auxiliary.at(ui).at(uj).adj.find(u) != Auxiliary.at(ui).at(uj).adj.end())
    {
        for (auto& i : Auxiliary.at(ui).at(uj).adj.at(u))
        {
            vec dif;
            set_difference(Auxiliary.at(ui).at(uj).adj.at(i).begin(), Auxiliary.at(ui).at(uj).adj.at(i).end(), Auxiliary.at(ui).at(uj).matched.begin(), Auxiliary.at(ui).at(uj).matched.end(), back_inserter(dif));
            if (!dif.empty())
            {
                bool flag = compareDegree(ui, uj, i);
                if (flag)
                {
                    addMajorInAuxiliary(ui, uj, i);
                }
            }
            else
            {
                addMinorInAuxiliary(ui, uj, i);
            }
        }
    }
}

void Graph::updateMajorAndMinor(int ui, int uj)
{
    set<int> major_set;//收集和匹配过顶点连接的major顶点
    for (auto& i: Auxiliary.at(ui).at(uj).matched)
    {
        for (auto& j: Q[i].q_neis)
        {
            if (Auxiliary.at(ui).at(uj).adj.find(j) != Auxiliary.at(ui).at(uj).adj.end() && Auxiliary.at(ui).at(uj).matched.find(j) == Auxiliary.at(ui).at(uj).matched.end())
            {
                major_set.emplace(j);
            }
        }
    }
    for (auto& i : major_set)
    {
        vec dif;
        set_difference(Auxiliary.at(ui).at(uj).adj.at(i).begin(), Auxiliary.at(ui).at(uj).adj.at(i).end(), Auxiliary.at(ui).at(uj).matched.begin(), Auxiliary.at(ui).at(uj).matched.end(), back_inserter(dif));
        if (!dif.empty())
        {
            bool flag = compareDegree(ui, uj, i);
            if (flag)
            {
                addMajorInAuxiliary(ui, uj, i);
            }
        }
        else
        {
            addMinorInAuxiliary(ui, uj, i);
        }
    }
}

void Graph::addMajorInAuxiliary(int ui, int uj, int u)
{
    if (Auxiliary.at(ui).at(uj).major.find(Q[u].q_label) != Auxiliary.at(ui).at(uj).major.end())
    {
        Auxiliary.at(ui).at(uj).major.at(Q[u].q_label) = u;
    }
    else
    {
        Auxiliary.at(ui).at(uj).major.emplace(Q[u].q_label, u);
    }
}

void Graph::addMinorInAuxiliary(int ui, int uj, int u)
{
    if (Auxiliary.at(ui).at(uj).minor.find(Q[u].q_label) != Auxiliary.at(ui).at(uj).minor.end())
    {
        Auxiliary.at(ui).at(uj).minor.at(Q[u].q_label).emplace(u);
    }
    else
    {
        set<int> temp;
        temp.emplace(u);
        Auxiliary.at(ui).at(uj).minor.emplace(Q[u].q_label, temp);
    }
}

void Graph::deleteMinorInAuxiliary(int ui, int uj, int u)
{
    Auxiliary.at(ui).at(uj).minor.at(Q[u].q_label).erase(u);
    if (Auxiliary.at(ui).at(uj).minor.at(Q[u].q_label).empty())
    {
        Auxiliary.at(ui).at(uj).minor.erase(Q[u].q_label);
    }
}

bool Graph::compareDegree(int ui, int uj, int u)
{
    if (Auxiliary.at(ui).at(uj).major.find(Q[u].q_label) != Auxiliary.at(ui).at(uj).major.end())
    {
        int ori_u = Auxiliary.at(ui).at(uj).major.at(Q[u].q_label);
        if (Auxiliary.at(ui).at(uj).adj.at(ori_u).size() < Auxiliary.at(ui).at(uj).adj.at(u).size())
        {
            return true;   //major顶点需要被替换
        }
        else
        {
            return false;
        }
    }
    else
    {
        return true;
    }
}

void Graph::generateOrderOfEdgeOfRemainQ(multimap<int, int> nextLabelOfMajor, multimap<int, int> nextLabelOfMinor)
{
    cmpNextForAllEdge(nextLabelOfMajor, nextLabelOfMinor);
    multimap<int, int> updatedNextLabelOfMajor = divideNextByLabel("major");
    multimap<int, int> updatedNextLabelOfMinor = divideNextByLabel("minor");
    if (!updatedNextLabelOfMajor.empty() || !updatedNextLabelOfMinor.empty())
    {
        generateOrderOfEdgeOfRemainQ(updatedNextLabelOfMajor, updatedNextLabelOfMinor);
    }
}

multimap<int, int> Graph::divideNextByLabel(string flag)
{
    multimap<int, int> sortLabel; 
    unordered_map<int, int> cntLabel; // label - the number of edges including the label 
    for (auto& i : Auxiliary)
    {
        for (auto& j : i.second)
        {
            if (flag == "major")
            {
                for (auto& k : j.second.major)
                {
                    if (cntLabel.find(k.first) != cntLabel.end())
                    {
                        cntLabel.at(k.first) += 1;
                    }
                    else
                    {
                        cntLabel.emplace(k.first, 1);
                    }
                }
            }
            else
            {
                for (auto& k : j.second.minor)
                {
                    if (cntLabel.find(k.first) != cntLabel.end())
                    {
                        cntLabel.at(k.first) += 1;
                    }
                    else
                    {
                        cntLabel.emplace(k.first, 1);
                    }
                }
            }
        }
    }
    sortLabel = sortLabelByNum(cntLabel);

    return sortLabel;
}

multimap<int, int> Graph::sortLabelByNum(unordered_map<int, int> cntLabel)
{
    multimap<int, int> sortLabel;
    for (auto& i: cntLabel)
    {
        sortLabel.emplace(i.second, i.first);
    }
    return sortLabel;
}

void Graph::cmpNextForAllEdge(multimap<int, int> nextLabelOfMajor, multimap<int, int> nextLabelOfMinor)
{
    for (auto& i: Auxiliary)     // ui = i.first
    {
        for (auto& j: i.second)  // uj = j.first
        {
            multimap<int, int>::reverse_iterator iter;
            if (!j.second.major.empty())
            {
                bool flag = false;   //下一个顶点不是公共的顶点(就是一个找到下一个匹配的顶点后提前停止循环的操作)
                for (iter = nextLabelOfMajor.rbegin(); iter != nextLabelOfMajor.rend(); iter++)
                {
                    for (auto& l : j.second.major)
                    {
                        if (l.first == iter->second)   //label相同    next_u = l.second
                        {
                            int next_u = l.second;
                            addOrderOfEdgeOfRemainQ(true, i.first, j.first, next_u);
                            updateAuxiliary_Matched(false, i.first, j.first, next_u);
                            j.second.major.clear();
                            updateMajorAndMinor(i.first, j.first);
                            u_map adj = j.second.adj;
                            u_map updatedAdj = removeMatchedNodeFromQ(next_u, adj);
                            updateAuxiliary_Adj(false, i.first, j.first, updatedAdj);
                            flag = true; 
                            break;
                        }
                    }
                    if (flag)
                    {
                        break;
                    }
                }
            }
            else
            {
                bool flag = false;
                for (iter = nextLabelOfMinor.rbegin(); iter != nextLabelOfMinor.rend(); iter++) 
                {
                    for (auto& l : j.second.minor)
                    {
                        if (l.first == iter->second)
                        {
                            int next_u = *(l.second).begin();
                            addOrderOfEdgeOfRemainQ(false, i.first, j.first, next_u);
                            updateAuxiliary_Matched(false, i.first, j.first, next_u);
                            deleteMinorInAuxiliary(i.first, j.first, next_u);
                            u_map adj = j.second.adj;
                            u_map updatedAdj = removeMatchedNodeFromQ(next_u, adj);
                            updateAuxiliary_Adj(false, i.first, j.first, updatedAdj);
                            flag = true;
                            break;
                        }
                    }
                    if (flag)
                    {
                        break;
                    }
                }
            }
        }
    }
}

void Graph::addOrderOfEdgeOfRemainQ(bool flag, int ui, int uj, int u)
{
    vec temp(2);
    temp[0] = u;
    if (flag)
    {  //major
        temp[1] = 1;
    }
    else
    {
        temp[1] = 0;
    }
    if (OrderOfEdgeOfRemainQ.find(ui) != OrderOfEdgeOfRemainQ.end())
    {
        if (OrderOfEdgeOfRemainQ.at(ui).find(uj) != OrderOfEdgeOfRemainQ.at(ui).end())
        {       
            OrderOfEdgeOfRemainQ.at(ui).at(uj).emplace_back(temp);
        }
        else
        {
            two_vec temp2;
            temp2.emplace_back(temp);
            OrderOfEdgeOfRemainQ.at(ui).emplace(uj, temp2);
        }
    }
    else
    {
        two_vec temp2;
        temp2.emplace_back(temp);
        umap_twovec temp3;
        temp3.emplace(uj, temp2);
        OrderOfEdgeOfRemainQ.emplace(ui, temp3);
    }
}

void Graph::summaryAllOrder(int label1, int label2)
{ 
    two_u_map dividedL; // label(first matched u) - ui - uj
    for (auto& j : OrderOfEdgeOfRemainQ)
    {
        for (auto& k : j.second)
        {
            if (Q[j.first].q_label == label1 && Q[k.first].q_label == label2)
            {
                dividedL = classfiyNodes(dividedL, j.first, k.first, k.second[0][0]);
            }
        }
    }
    //要点！ 关闭了输出
    // cout << endl;
    // cout << "lable1 = " << label1 << ", label2 = " << label2 << endl;
    // cout << "---------------------------------------------- begin 0" << endl;
    // cout << "------- dividedL:" << endl;
    // print_two_umap(dividedL);

    for (auto& i: dividedL)
    {
        
        od_node n;
        n.label = i.first;
        odTree_nodes.emplace_back(n);
        int ot_id = odTree_nodes.size() - 1;
        update_odNode_uSet(i.second, 0, ot_id);
        addOdTree_startNodes(label1, label2, ot_id);
        addOdTree_adjKey(label1, label2, ot_id);
        combineAGroupOfOrders(label1, label2, i.second, 1, ot_id);
    }
}

two_u_map Graph::classfiyNodes(two_u_map dividedL, int ui, int uj, int u)
{
    if (dividedL.find(Q[u].q_label) != dividedL.end())
    {
        if (dividedL.at(Q[u].q_label).find(ui) != dividedL.at(Q[u].q_label).end())
        {
            dividedL.at(Q[u].q_label).at(ui).emplace(uj);
        }
        else
        {
            set<int> temp;
            temp.emplace(uj);
            dividedL.at(Q[u].q_label).emplace(ui, temp);
        }
    }
    else
    {
        set<int> temp;
        temp.emplace(uj);
        u_map temp2;
        temp2.emplace(ui, temp);
        dividedL.emplace(Q[u].q_label, temp2);
    }

    return dividedL;
}

void Graph::update_odNode_uSet(u_map edge, int id, int orderTree_id)
{
    for (auto& i: edge)
    {
        for (auto& j: i.second)
        {
            odTree_nodes[orderTree_id].u_set.emplace_back(OrderOfEdgeOfRemainQ.at(i.first).at(j)[id][0]);
        }
    }
}

void Graph::addOdTree_startNodes(int label1, int label2, int orderTree_id)
{
    if (odTree_startNodes.find(label1) != odTree_startNodes.end())
    {
        if (odTree_startNodes.at(label1).find(label2) != odTree_startNodes.at(label1).end())
        {
            odTree_startNodes.at(label1).at(label2).emplace(orderTree_id);
        }
        else
        {
            set<int> temp;
            temp.emplace(orderTree_id);
            odTree_startNodes.at(label1).emplace(label2, temp);
        }
    }
    else
    {
        set<int> temp;
        temp.emplace(orderTree_id);
        u_map temp2;
        temp2.emplace(label2, temp);
        odTree_startNodes.emplace(label1, temp2);
    }
}

void Graph::addOdTree_adjKey(int label1, int label2, int adj_key)
{
    //邻接表中添加新的key
    set<int> temp;
    u_map temp1;
    temp1.emplace(adj_key, temp);
    if (orderTree.find(label1) != orderTree.end())
    {
        if (orderTree.at(label1).find(label2) != orderTree.at(label1).end())
        {
            orderTree.at(label1).at(label2).emplace(adj_key, temp);
        }
        else
        {
            orderTree.at(label1).emplace(label2, temp1);
        }
    }
    else
    {
        two_u_map temp2;
        temp2.emplace(label2, temp1);
        orderTree.emplace(label1, temp2);
    }
}

void Graph::update_OdTree_adj(int label1, int label2, int adj_key, int adj_value)
{
    orderTree.at(label1).at(label2).at(adj_key).emplace(adj_value);
}

void Graph::combineAGroupOfOrders(int label1, int label2, u_map edge, int anEdgeOd_id, int adj_key)
{
    int thr_size = Q_adjList.at(Q[edge.begin()->first].q_id).size() - 2;
    two_u_map dividedL; // label(first matched u) - ui - uj
    for (auto& i: edge)
    {
        for (auto& j: i.second)
        {
            dividedL = classfiyNodes(dividedL, i.first, j, OrderOfEdgeOfRemainQ.at(i.first).at(j)[anEdgeOd_id][0]);
        }
    }

    for (auto& i : dividedL) 
    {
        od_node n;
        n.label = i.first;
        odTree_nodes.emplace_back(n);
        int ot_id = odTree_nodes.size() - 1;
        update_odNode_uSet(i.second, anEdgeOd_id, ot_id);
        update_OdTree_adj(label1, label2, adj_key, ot_id);
        if (anEdgeOd_id < thr_size-1)
        {
            addOdTree_adjKey(label1, label2, ot_id);
            combineAGroupOfOrders(label1, label2, i.second, anEdgeOd_id + 1, ot_id);
        }
    }

}



//index
void Graph::constructIndex()
{
    int G_size = G.size();
    for (int vi = 0; vi < G_size; vi++) {
        int lb = G[vi].label;//vi的label
        if (lb >= 0 && G[vi].nei.size() != 0) {
            for (auto& mulmap : q_labelEdge[lb]) {//对q边筛选以vi起始的边，ui——》uj
                int uj_lb = mulmap.first;//uj的label
                int q_id = mulmap.second.front();
                int thr = mulmap.second.size();//多少个ui
                for (auto i = 1; i < thr; i++)
                {
                    int ui = mulmap.second.at(i);
                    int ui_ujLbNeiNum = Q[ui].q_neiSet.at(uj_lb).size();

                    int vi_ujLbNeiNum;
                    if (G[vi].nei.find(uj_lb) != G[vi].nei.end())
                    {
                        vi_ujLbNeiNum = G[vi].nei.at(uj_lb).size();
                    }
                    else
                    {
                        vi_ujLbNeiNum = 0;
                    }

                    if (ui_ujLbNeiNum <= vi_ujLbNeiNum)
                    {    //vi邻域中该标签顶点数量多于ui邻域中该标签邻接点数量
                        if (G[vi].cand.find(q_id) != G[vi].cand.end())
                        {
                            if (G[vi].cand.at(q_id).find(ui) != G[vi].cand.at(q_id).end())
                            {
                                G[vi].cand.at(q_id).at(ui).insert(uj_lb);
                            }
                            else
                            {
                                set<int> temp;
                                temp.insert(uj_lb);
                                G[vi].cand.at(q_id).insert(make_pair(ui, temp));
                            }
                        }
                        else
                        {
                            set<int> temp;
                            temp.insert(uj_lb);
                            u_map temp2;
                            temp2.insert(make_pair(ui, temp));
                            G[vi].cand.insert(make_pair(q_id, temp2));
                        }


                        //计算self_LI
                        if (G[vi].cand.at(q_id).at(ui).size() == Q[ui].q_neiLabels.size())
                        {
                            if (G[vi].self_LI.find(q_id) != G[vi].self_LI.end())
                            {
                                G[vi].self_LI.at(q_id).insert(ui);
                            }
                            else
                            {
                                set<int> temp;
                                temp.insert(ui);
                                G[vi].self_LI.emplace(q_id, temp);
                            }
                        }
                    }
                }
            }
        }
    }

    //计算nbr_LI
    for (auto vi = 0; vi < G_size; vi++)
    {
        if (G[vi].self_LI.size())
        {
            for (auto& match : G[vi].self_LI)
            {
                int Qi = match.first;//查询图ID
                for (auto& ui : match.second)//每个查询图的selfLI的set
                {
                    for (auto& uiNbr : Q[ui].q_neis)//对每个ui的邻居
                    {
                        int uiNbr_label = Q[uiNbr].q_label;
                        for (auto& viNbr : G[vi].nei.at(uiNbr_label)) {//对每个vi的邻居，更新ui的邻居
                            update_nbr_LI(vi, viNbr, Qi, uiNbr);
                        }
                    }
                }
            }
        }
    }

}

void Graph::update_nbr_LI(int v, int v_nbr, int Qi, int u_nbr)
{
    if (G[v_nbr].self_LI.find(Qi) != G[v_nbr].self_LI.end())//VNbr必须有Qi以及 Unbr的LI
    {
        if (G[v_nbr].self_LI.at(Qi).find(u_nbr) != G[v_nbr].self_LI.at(Qi).end())
        {
            //v_nbr匹配u_nbr，更新v的nbr_LI
            if (G[v].nbr_LI.find(u_nbr) != G[v].nbr_LI.end())//v -> ui{vj,vj,vj}
            {
                G[v].nbr_LI.at(u_nbr).emplace(v_nbr);//插入一个新vj
            }
            else
            {
                set<int> temp;
                temp.insert(v_nbr);
                G[v].nbr_LI.emplace(u_nbr, temp);
            }
        }
    }
}




//print
void Graph::print_Q()
{
    int end_q = Q.size();
    for (int i = 0; i < end_q; i++)
    {
        print_u(i);
    }
}

void Graph::print_u(int i)
{
    std::cout << "------------------------------ u" << i <<":" << std::endl;
    std::cout << "Q[i].q_id = " << Q[i].q_id << ", Q[i].comm_id = " << Q[i].comm_id << "Q[i].q_label = "<<Q[i].q_label<<endl;
    std::cout << std::endl;
}

void Graph::print_umap(u_map m)
{
    std::cout << "------------------------------" << std::endl;
    for (auto& i : m)
    {
        cout << "second key = " << i.first << ": [";
        for (auto& j : i.second)
        {
        	cout << j << ", ";
        }
        cout <<"]" << endl;
    }
}

void Graph::print_two_umap(two_u_map tm)
{
    for (auto& i: tm)
    {
        cout << "---------------- first key = " << i.first << endl;
        print_umap(i.second);
    }
}

void Graph::print_two_umap_twovec(two_umap_twovec tmv)
{
    cout << "--------------- two_umap_twovec -------------" << endl;
    for (auto& i : tmv)
    {
        cout << "---------------- first key = " << i.first << endl;
        for (auto& j : i.second)
        {
            cout << "second key = " << j.first << ": { ";
            for (auto& k : j.second)
            {
                cout << "[";
                for (auto& t: k)
                {
                    cout << t << ", ";
                }
                cout<< "], ";
            }
            cout << "}" << endl;
        }
    }
    cout << "-------------------------------------------------------" << endl;
}

void Graph::print_vec(vec ve)
{
    cout << "[";
    for (auto& k : ve)
    {
        cout << k << ", ";
    }
    cout << "], ";
}

void Graph::print_set(set<int> ve)
{
    cout << "[";
    for (auto& k : ve)
    {
        cout << k << ", ";
    }
    cout << "]" << endl;
}

void Graph::print_umap_twovec(umap_twovec utv)
{
    for (auto& i : utv)
    {
        cout << "first key = " << i.first << ": { ";
        for (auto& j: i.second)
        {
            print_vec(j);
        }
        cout << " }" << endl;
    }
}

void Graph::print_tri_u_map(tri_u_map tum)
{
    for (auto& i: tum)
    {
        cout << "------------------------------------ first first key = " << i.first << endl;
        print_two_umap(i.second);
    }
}

void Graph::print_Auxiliary(unordered_map<int, unordered_map<int, Aux>> Auxiliary)
{
    cout << "----here------------------Auxiliary------------------------" << endl;
    for (auto& i : Auxiliary)
    {
        cout << "---------------- first key = " << i.first << endl;
        for (auto& j : i.second)
        {
            cout << "second key = " << j.first << ": " << endl;
            if (i.first == 1 && j.first == 4)
            {
                cout << "---- Adj " << endl;
                print_umap(j.second.adj);
                cout << "---- Major " << endl;
                for (auto& k : j.second.major)
                {
                    cout << "label = " << k.first << ", u = " << k.second << endl;
                }
                cout << "---- Minor " << endl;
                print_umap(j.second.minor);
                cout << "---- Matched " << endl;
                print_set(j.second.matched);
            }
            
        }
    }
    cout << "-------------------------------------------------------" << endl;
}

void Graph::print_multimap(std::multimap<int, int> p)
{
    cout << "-------------print_multimap ---------" << endl;
    for (auto& k : p)
    {
        cout << "(" << k.first << " - " << k.second << ")";
    }
    cout << endl;
}
