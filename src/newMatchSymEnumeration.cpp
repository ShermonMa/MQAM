#include "match.h"
#include <iostream>
#include <algorithm>

#include <chrono>
#define Get_Time() std::chrono::high_resolution_clock::now()
#define Duration(start) std::chrono::duration_cast<\
    std::chrono::microseconds>(Get_Time() - start).count()/(float)1000
//我们试图解耦开辅助的数据结构，重新思考为何在DFS过程中有对称剪枝，以及如何利用
// inline int numCombination(int a, int b) {//计算组合数
//     if (b > a) return 0;
//     if (b == 0 || b == a) return 1;
//     b = std::min(b, a - b); // Take advantage of symmetry
//     int result = 1;
//     for (int i = 0; i < b; ++i) {
//         result *= (a - i);
//         result /= (i + 1);
//     }
//     return result;
// }
inline int numCombination(int a, int b) {//计算组合数
    if(a<BasicComputation::MAX_COMBINATION)return BasicComputation::combination(a,b);
    if (b > a) return 0;
    if (b == 0 || b == a) return 1;
    b = std::min(b, a - b); // Take advantage of symmetry
    int result = 1;
    for (int i = 0; i < b; ++i) {
        result *= (a - i);
        result /= (i + 1);
    }
    return result;
}
inline int numPermutation(int n, int r) {//计算排列数
    if (r > n || n < 0 || r < 0) {
        throw invalid_argument("PERMUTATION Invalid arguments: Ensure n >= r >= 0");
    }
    int result = 1;
    for (int i = n; i > n - r; --i) {
        result *= i;
    }
    return result;
}

// bool Match::symCheckFromRemainSimplified(int q_id, int ui, int uj,int vi,int vj)
// {
//     //countRemainMatch开始前的合法性检查
//     if(QSym[q_id].matchNum==1)return true;
//     if(initG[vi].label!=initG[vj].label)return true;
//     //isSymVertex应该可以用sr的数组直接获得吧，按理来说不需要这么复杂的寻址
// 	vec& rr = symRepresenter.symmetryRepresenter;
// 	int representer_i = rr[ui];
// 	int representer_j = rr[uj];
//     if(representer_i!=-1 && representer_j!=-1)
//     {
//         if(representer_i == representer_j)return ui==representer_i&&vi<vj;
//     }
//     return true;
// }
bool Match::symCheckFromRemainSimplified(int q_id, int ui, int uj,int vi,int vj)
{
    //countRemainMatch开始前的合法性检查
    if(QSym[q_id].matchNum==1)return true;
    if(initG[vi].label!=initG[vj].label)return true;
    //isSymVertex应该可以用sr的数组直接获得吧，按理来说不需要这么复杂的寻址
    if(isSymmetryVertex(ui) && isSymmetryVertex(uj))
    {
        const SymmetryRecord& sr = QSym[q_id];
        int offset = QSym[q_id].rangeS;
        int representer_i = sr.symmetryRepresenter[ui-offset];
        int representer_j = sr.symmetryRepresenter[uj-offset];
        if(representer_i == representer_j)return ui==representer_i&&vi<vj;
    }
    return true;
}
int Match::analyseInitCand_Joint(int q_id,const u_map& cand,int dif)
{
	// std::cout<<"ana_Joint_start::"<<endl;
	int multi = 1;
	unordered_map<int,set<int>> groups;
	const SymmetryRecord& sr = QSym[q_id];
	for(auto& each : cand)
	{	
		// int eachAfterDif = each.first+dif;
		int eachAfterDif = each.first;
		if(sr.ngbrSymRepresenter[eachAfterDif - sr.rangeS ] == -1)continue;
		else 
		{
			groups[sr.ngbrSymRepresenter[eachAfterDif - sr.rangeS]].insert(eachAfterDif);
		}
	}
	for(auto&& each : groups)
	{
		if(each.second.size()<2)continue;
		vector<bool> flags(each.second.size(),false);
		vector<int> sameNgbrNodes(each.second.begin(),each.second.end());
		//暴力遍历数组找到NbrLi相同的同组结点，说明此时它们还处于共同选择中
		for(int i = 0;i<sameNgbrNodes.size();i++)
		{
			if(flags[i])continue;
			const set<int>& thisCand = cand.at(sameNgbrNodes[i]);
			set<int> sameChoicesNodes;
			for(int j = i+1;j<sameNgbrNodes.size();j++)
			{
				if(cand.at(sameNgbrNodes[j]) == thisCand)
				{
					flags[j] = true;
					sameChoicesNodes.insert(sameNgbrNodes[j]);
				}
			}
			if(sameChoicesNodes.size()<thisCand.size())//加上下面的insertion至少要等于
			{
				//新组别，更新对称组
				sameChoicesNodes.insert(sameNgbrNodes[i]);
				multi*=addSymGroup(sameNgbrNodes[i],cand.at(sameNgbrNodes[i]),sameChoicesNodes);
			}
			else return -1;
		}
	}
	// std::cout<<"ana_Joint_end::"<<endl;
	return multi;
}
// int Match::analyseInitCand_Joint(int q_id,const u_map& cand,int dif)
// {
// 	int multi = 1;
// 	unordered_map<int,set<int>> groups;
// 	vec& rr = symRepresenter.symmetryNgbrRepresenter;
// 	// const SymmetryRecord& sr = QSym[q_id];
// 	for(auto& each : cand)
// 	{	
// 		// int eachAfterDif = each.first+dif;
// 		int eachAfterDif = each.first;
// 		// if(!(eachAfterDif>=range[q_id*2]&&eachAfterDif<=range[q_id*2+1]))
// 		// {
// 		// 	std::cout<<"ERROR in analyseInitCand_Joint, eachAfterDif not in range"<<eachAfterDif<<" in <"
// 		// 	<<range[q_id*2]<<","<<range[q_id*2+1]<<">"<<endl;
// 		// }
// 		if(rr[eachAfterDif] == -1)continue;
// 		else 
// 		{
// 			groups[rr[eachAfterDif]].insert(eachAfterDif);
// 		}
// 	}
// 	for(auto&& each : groups)
// 	{
// 		if(each.second.size()<2)continue;
// 		vector<bool> flags(each.second.size(),false);
// 		vector<int> sameNgbrNodes(each.second.begin(),each.second.end());
// 		//暴力遍历数组找到NbrLi相同的同组结点，说明此时它们还处于共同选择中
// 		for(int i = 0;i<sameNgbrNodes.size();i++)
// 		{
// 			if(flags[i])continue;
// 			set<int> thisCand = cand.at(sameNgbrNodes[i]);
// 			set<int> sameChoicesNodes;
// 			for(int j = i+1;j<sameNgbrNodes.size();j++)
// 			{
// 				if(cand.at(sameNgbrNodes[j]) == thisCand)
// 				{
// 					flags[j] = true;
// 					sameChoicesNodes.insert(sameNgbrNodes[j]);
// 				}
// 			}
// 			if(sameChoicesNodes.size()>0)
// 			{
// 				//新组别，更新对称组
// 				sameChoicesNodes.insert(sameNgbrNodes[i]);
// 				multi*=addSymGroup(sameNgbrNodes[i],cand.at(sameNgbrNodes[i]),sameChoicesNodes);
// 			}
// 		}
// 	}
// 	return multi;
// }
tuple<int,set<int>,set<int>> Match::analyseUpdateCand(int q_id,const u_map& cand,const set<int>& newNodes)
{
    unordered_set<int> checked;
    int multi = 1;
    set<int> repres;
	set<int> changed;
    const SymmetryRecord& sr = QSym[q_id];
    for(auto each : newNodes)
    {
        if(checked.find(each)!=checked.end())continue;
        if(sr.ngbrSymRepresenter[each - sr.rangeS] == -1)continue;
        if(eachQManager.at(q_id).symmetryUnit.find(each)!=eachQManager.at(q_id).symmetryUnit.end())
        {
            SymmetryUnit &su = *eachQManager.at(q_id).symmetryUnit.at(each);
            if(su.candidates.size()==cand.at(each).size())continue;
			else if(su.vertices.size()>cand.at(each).size() ){
				return make_tuple(-1,std::set<int>(), std::set<int>());
			}
            else
            {
                su.candidates = std::vector<int>(cand.at(each).begin(),cand.at(each).end());
				changed.insert(each);
				for(auto each : su.vertices)
				{
					if(newNodes.find(each)!=newNodes.end())checked.insert(each);
				}
				continue;
            }
        }
        set<int> group;
        group.insert(each);
        for(auto eachInGroup : sr.ngbrGroup.at(sr.ngbrSymRepresenter[each - sr.rangeS])->vertices)
        {
            if(newNodes.find(eachInGroup)!=newNodes.end())
            {
                checked.insert(eachInGroup);
                group.insert(eachInGroup);
				if(group.size()>cand.at(each).size())return make_tuple(-1,std::set<int>(), std::set<int>());
            }
        }
        if(group.size() == 1)continue;
		multi*=addSymGroup(each,cand.at(each),group);
        repres.insert(each);
    }
    return make_tuple(multi,repres,changed);
}

// tuple<int,set<int>,set<int>> Match::analyseUpdateCand(int q_id,const u_map& cand,const set<int>& newNodes)
// {
//     unordered_set<int> checked;
// 	checked.reserve(newNodes.size());
//     int multi = 1;
//     set<int> repres;
// 	set<int> changed;
// 	vec& rr = symRepresenter.symmetryNgbrRepresenter;
//     // const SymmetryRecord& sr = QSym[q_id];
//     for(auto each : newNodes)
//     {
//         if(checked.find(each)!=checked.end())continue;
//         if(rr[each] == -1)continue;
//         if(eachQManager.at(q_id).symmetryUnit.find(each)!=eachQManager.at(q_id).symmetryUnit.end())
//         {
//             SymmetryUnit &su = *eachQManager.at(q_id).symmetryUnit.at(each);
//             if(su.candidates.size()==cand.at(each).size())continue;
//             else
//             {
//                 su.candidates = std::vector<int>(cand.at(each).begin(),cand.at(each).end());
// 				changed.insert(each);
// 				for(auto each : su.vertices)
// 				{
// 					if(newNodes.find(each)!=newNodes.end())checked.insert(each);
// 				}
// 				continue;
//             }
//         }
//         set<int> group;
//         group.insert(each);
//         for(auto eachInGroup : QSym[q_id].ngbrGroup.at(rr[each])->vertices)
//         {
//             if(newNodes.find(eachInGroup)!=newNodes.end())
//             {
//                 checked.insert(eachInGroup);
//                 group.insert(eachInGroup);
//             }
//         }
//         if(group.size() == 1)continue;
// 		multi*=addSymGroup(each,cand.at(each),group);
//         repres.insert(each);
//     }
//     return make_tuple(multi,repres,changed);
// }
int Match::addSymGroup(int repre,const set<int>& cand,const set<int>& vertices)
{
	// if(cand.size()<vertices.size())return -1;
	SymmetryUnit newUnit;
	int q_id = q[repre].q_id;
	newUnit.vertices = vertices;
	newUnit.candidates  = std::vector<int>(cand.begin(),cand.end());
	newUnit.current.push(-1);
	std::shared_ptr<SymmetryUnit> unitPtr = make_shared<SymmetryUnit>(newUnit);
	for(auto eachSymV : vertices)
	{
		eachQManager.at(q_id).symmetryUnit.emplace(eachSymV,unitPtr);
	}
	eachQManager.at(q_id).unitVertices.emplace(repre,vertices);
	int multi=numPermutation(vertices.size(),vertices.size());
	return multi;
}
void Match::deleteSymGroup(const set<int>& repres,int q_id,const set<int>& changed,const u_map& cand)
{
    for(auto each : repres)
    {
        const set<int>& keys = eachQManager.at(q_id).unitVertices.at(each);
        for(auto eachKey : keys)
        {
            eachQManager.at(q_id).symmetryUnit.erase(eachKey);
        }
        eachQManager.at(q_id).unitVertices.erase(each);
    }
	for(auto each : changed)
	{
		eachQManager.at(q_id).symmetryUnit.at(each)-> candidates = std::vector<int>(cand.at(each).begin(),cand.at(each).end());
	}
}
pair<u_map,set<int>> Match::newSym_updateCand(bool joinOperation, int v, int u, u_map cand, std::unordered_map<int, int> match, std::set<int> used_v, int dif)
{
	//cand中删除其他u候选集中的v
	//selfLI中按照查询图分类地存储了诸多该数据点对应地查询点，所以现在我们要在已有的Match下
 	bool flag = false;
    set<int> newNodes;
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
		return make_pair(cand,newNodes);
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
							if(temp != cand.at(i))
							{
								if(isNgbrSymmetryVertex(i))newNodes.insert(i);
							}
							cand.erase(i);
							cand.emplace(i, temp);
						}
					}
					else
					{
						cand.erase(i);
						cand.emplace(i, rest_c_set);
						newNodes.insert(i);
					}
				}
                //newNodes可能是全新的点，也可能是被再次约减的老未匹配点
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
	return make_pair(cand,newNodes);
}

pair<u_map,set<int>> Match::newSym_cmpJoinedInitCand(int q_id, std::unordered_map<int, int> match, std::set<int> used_v, int dif)
{
	u_map cand;
	set<int> newNodes;
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
							else
							{
								flag = true;
								break;
							}
						}
						else
						{
							newNodes.insert(i.first);
							cand.emplace(i.first, rest_c_set);
						}
					}
					else
					{
						flag = true;
						break;
					}
				}
				else
				{
					flag = true;
					break;
				}
			}
			if (flag)
			{
				cand.clear();
				break;
			}
		}
	}

	return make_pair(cand,newNodes);
}


long long Match::newSym_countJoinedMatch(int q_id, int index_u, int sum, 
u_map cand, unordered_map<int, int> match, set<int> used_v, int dif,int isoph)
{
	long long result = 0;
	if (index_u < sum - 1)
	{
		int u = orderRemainQ.at(q_id)[index_u][0];//新的匹配结点u
        int q_id = q[u].q_id;
        bool isSymmetry = false;
        if(eachQManager.at(q_id).symmetryUnit.find(u)!=eachQManager.at(q_id).symmetryUnit.end())
        {
            const SymmetryUnit& unit = *eachQManager.at(q_id).symmetryUnit.at(u);
            if(sum - index_u == unit.vertices.size() - unit.matchedNum&& QSym[q_id].ngbrGroup.at(QSym[q_id].ngbrSymRepresenter[u-QSym[q_id].rangeS])->isSelfConnected == false)
            {
                //只剩本组未匹配
                int combinationNum;
                vec dif;
                set_difference(cand.at(u).begin(), cand.at(u).end(), used_v.begin(), used_v.end(), back_inserter(dif));

                combinationNum = numCombination(dif.size(),sum - index_u);
				result += combinationNum*isoph;
				if(coutAble)
                {
                    std::cout<<"JointOutcome Result: "<<result<<" ";print_unordered_map(match);
                    std::cout<<"<";
                    for(auto each : unit.vertices)std::cout<<each<<",";std::cout<<">";
                    for (auto& v : dif)
                    {
                        std::cout<<"["<<v<<"]";
                    }
					std::cout<<"\t\t"<<combinationNum<<" "<<isoph;
					std::cout<<endl;
                }
                return combinationNum*isoph;
            }
            isSymmetry = true;
            int lastChoice = unit.current.top();

			//const SymmetryUnit& unit = *eachQManager.at(q_id).symmetryUnit.at(u);
			auto iter = upper_bound(unit.candidates.begin(),unit.candidates.end(),lastChoice);
			auto enditer = unit.candidates.end();
			if(iter!=enditer)
			{
				std::advance(enditer,-(unit.vertices.size() - unit.matchedNum-1));
				set<int>& newCand = cand.at(u);
				newCand.clear();
				std::set_difference(iter,enditer,used_v.begin(),used_v.end(),inserter(newCand,newCand.begin()));
			}
			else
			{
				cand.at(u).clear();
			}
            
        }
		if (cand.find(u) != cand.end())
		{
            for (auto& v : cand.at(u))
            {
                if(isSymmetry)
                {
                    eachQManager.at(q_id).symmetryUnit.at(u)->current.push(v);
                    eachQManager.at(q_id).symmetryUnit.at(u)->matchedNum++;
                }
                std::unordered_map<int, int> copy_match = match;         u_map copy_cand = cand;
                copy_match.emplace(u, v);		copy_cand.erase(u);
                std::set<int> copy_usedV = used_v;set<int> newNodes;
                copy_usedV.emplace(v);
                //copy_cand = updateCand(true, v, u, copy_cand, copy_match, copy_usedV, dif);//在这里选择然后更新了一下candidate
                int copy_index_u = index_u;
                auto [returnCand,returnSet] = newSym_updateCand(true, v, u, copy_cand, copy_match, copy_usedV, dif);//在这里选择然后更新了一下candidate
                copy_cand = returnCand;
                newNodes = returnSet;
                if ( !copy_cand.empty())
                {
					if(!newNodes.empty())
					{
						auto[ multi, representers ,changed]= analyseUpdateCand(q_id,copy_cand,newNodes);
						if(multi>0)
						{
							copy_index_u += 1;
							long long newResult = newSym_countJoinedMatch(q_id, copy_index_u, sum, copy_cand, copy_match, copy_usedV, dif,isoph*multi);
							result += newResult;
							// matchFromComm+=newResult;
						}
						else
						{
							partialMatch_num+=1;
						}
						deleteSymGroup(representers,q_id,changed,cand);
					}
					else
					{
						copy_index_u += 1;
						result += newSym_countJoinedMatch(q_id, copy_index_u, sum, copy_cand, copy_match, copy_usedV, dif,isoph);
					}
                }
                else
                {	//这里是部分匹配，我们需要在这里去记录失败点
                    partialMatch_num += 1;
                }
                if(isSymmetry)
                {
                    eachQManager.at(q_id).symmetryUnit.at(u)->current.pop();
                    eachQManager.at(q_id).symmetryUnit.at(u)->matchedNum--;
                }
            }
        }
        
    }
	
	else if (index_u == sum - 1)
	{
		int u = orderRemainQ.at(q_id)[index_u][0];
		
		if (cand.find(u) != cand.end())
		{	
			vec dif;
			vec newDif;
			int cnt;
			set_difference(cand.at(u).begin(), cand.at(u).end(), used_v.begin(), used_v.end(), back_inserter(dif));
			if(eachQManager.at(q_id).symmetryUnit.find(u)!=eachQManager.at(q_id).symmetryUnit.end())
			{
				const SymmetryUnit& unit = *eachQManager.at(q_id).symmetryUnit.at(u);
				cnt = std::distance(upper_bound(unit.candidates.begin(),unit.candidates.end(),unit.current.top()),unit.candidates.end());
				if (coutAble)newDif = std::vector<int>(upper_bound(unit.candidates.begin(),unit.candidates.end(),unit.current.top()),unit.candidates.end());
			}
			else
			{
				cnt = dif.size();
			}
			result += cnt*isoph;
			// matchFromComm+= cnt*isoph;
			if(coutAble)
			{
				std::cout<<"JointOutcome Result: "<<result<<" ";print_unordered_map(match);
				std::cout<<"<"<<u<<",";
				for (auto& v : (newDif.empty()?dif:newDif))
				{
					std::cout<<"["<<v<<"]";
				}
				std::cout<<">"<<endl;
			}
			//print match
			// if(useNoInitSuccessProcess)
			// {
			// 	//std::cout<<"Result:"<<result<<endl;
			// 	return result;
			// }
			// for (auto& v : (newDif.empty()?dif:newDif))
			// {
			// 	std::unordered_map<int, int> copy_match = match;
			// 	copy_match.emplace(u, v); 
			// 	std::cout<<"CommOutcome";print_unordered_map(copy_match);
			// }
			// std::cout<<"Result:"<<result<<endl;
		}
		else
		{
			partialMatch_num += 1;
		}
	}
	return result;
}

int Match::newSym_countRemainQMatch(int ui, int uj, int index_u, int sum, u_map cand,  unordered_map<int, int> match, set<int> used_v,int isoph)
{
	// std::cout<<"newsym countRemainQMatch"<<std::endl;
	int result = 0;
	if (index_u < sum - 1)
	{
		int u = orderEdgeOfRemainQ.at(ui).at(uj)[index_u][0];
        int q_id = q[u].q_id;
        bool isSymmetry = false;
        if(eachQManager.at(q_id).symmetryUnit.find(u)!=eachQManager.at(q_id).symmetryUnit.end())
        {
            const SymmetryUnit& unit = *eachQManager.at(q_id).symmetryUnit.at(u);
            if(sum - index_u == unit.vertices.size() - unit.matchedNum && QSym[q_id].ngbrGroup.at(QSym[q_id].ngbrSymRepresenter[u-QSym[q_id].rangeS])->isSelfConnected == false)
            {
                //只剩本组未匹配
				//std::cout<<"bool"<<QSym[q_id].ngbrGroup.at(u)->isSelfConnected<<endl;
                int combinationNum;
                vec dif;
			    set_difference(cand.at(u).begin(), cand.at(u).end(), used_v.begin(), used_v.end(), back_inserter(dif));

                combinationNum = numCombination(dif.size(),sum - index_u);
				if(coutAble)
			    {
                    std::cout<<"RemainOutcome Result: "<<combinationNum*isoph<<" ";print_unordered_map(match);
                    std::cout<<"<";
                    for(auto each : unit.vertices)std::cout<<u<<",";std::cout<<">";
                    for (auto& v : dif)
                    {
                        std::cout<<"["<<v<<"]";
                    }
					std::cout<<endl;
			    }
                return combinationNum*isoph;
            }
            isSymmetry = true;
            int lastChoice = unit.current.top();

			//const SymmetryUnit& unit = *eachQManager.at(q_id).symmetryUnit.at(u);
			auto iter = upper_bound(unit.candidates.begin(),unit.candidates.end(),lastChoice);
			auto enditer = unit.candidates.end();
			if(iter!=enditer)
			{
				std::advance(enditer,-(unit.vertices.size() - unit.matchedNum-1));
				set<int>& newCand = cand.at(u);
				newCand.clear();
				std::set_difference(iter,enditer,used_v.begin(),used_v.end(),inserter(newCand,newCand.begin()));
			}
			else
			{
				cand.at(u).clear();
			}

        }
		if (cand.find(u) != cand.end())
		{
            for (auto& v : cand.at(u))
            {
                if(isSymmetry)
                {
                    eachQManager.at(q_id).symmetryUnit.at(u)->current.push(v);
                    eachQManager.at(q_id).symmetryUnit.at(u)->matchedNum++;
                }
                std::unordered_map<int, int> copy_match = match;
                copy_match.emplace(u, v);
                u_map copy_cand = cand;
                copy_cand.erase(u);
                set<int> newNodes;
                std::set<int> copy_usedV = used_v;
                copy_usedV.emplace(v);
                auto [returnCand,returnSet] = newSym_updateCand(false, v, u, copy_cand, copy_match, copy_usedV, 0);//在这里选择然后更新了一下candidate
                copy_cand = returnCand;
                newNodes = returnSet;

                
                int copy_index_u = index_u;
                if (!copy_cand.empty())
                {
					if(!newNodes.empty())
					{
						auto[ multi, representers ,changed]= analyseUpdateCand(q_id,copy_cand,newNodes);
						if(multi>0)
						{
							copy_index_u += 1;
							int newResult = newSym_countRemainQMatch(ui, uj, copy_index_u, sum, copy_cand, copy_match,
											copy_usedV,isoph*multi);
							result += newResult;
						}
						else
						{
							partialMatch_num+=1;
						}

						// matchFromRemain+=newResult;
						deleteSymGroup(representers,q_id,changed,cand);
					}
					else
					{
						copy_index_u += 1;
						result += newSym_countRemainQMatch(ui, uj, copy_index_u, sum, copy_cand, copy_match,
										copy_usedV,isoph);
					}
                }
                else
                {	//这里是部分匹配，我们需要在这里去记录失败点
                    partialMatch_num += 1;
                }
                if(isSymmetry)
                {
                    eachQManager.at(q_id).symmetryUnit.at(u)->current.pop();
                    eachQManager.at(q_id).symmetryUnit.at(u)->matchedNum--;
                }
            }
        }
    }
	else if (index_u == sum - 1)
	{
		int u = orderEdgeOfRemainQ.at(ui).at(uj)[index_u][0];
		if (cand.find(u) != cand.end())
		{
			vec dif;
			vec newDif;
			int cnt,q_id = q[u].q_id;
			set_difference(cand.at(u).begin(), cand.at(u).end(), used_v.begin(), used_v.end(), back_inserter(dif));
			if(eachQManager.at(q_id).symmetryUnit.find(u)!=eachQManager.at(q_id).symmetryUnit.end())
			{
				const SymmetryUnit& unit = *eachQManager.at(q_id).symmetryUnit.at(u);
				cnt = std::distance(upper_bound(unit.candidates.begin(),unit.candidates.end(),unit.current.top()),unit.candidates.end());
				if (coutAble)newDif = std::vector<int>(upper_bound(unit.candidates.begin(),unit.candidates.end(),unit.current.top()),unit.candidates.end());
			}
			else
			{
				cnt = dif.size();
			}
			result += cnt*isoph;
			// matchFromRemain += cnt*isoph;
			//result += dif.size()*isoph;
			if(coutAble)
			{
				std::cout<<"RemainOutcome Result: "<<result<<" ";print_unordered_map(match);
				std::cout<<"<"<<u<<",";
				for (auto& v : (newDif.empty()?dif:newDif))
				{
					std::cout<<"["<<v<<"]";
				}
				std::cout<<">"<<endl;
			}
		}
        else
        {
            partialMatch_num+=1;
        }
	
	}
	return result;
}