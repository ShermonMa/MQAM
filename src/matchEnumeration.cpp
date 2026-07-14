#include "match.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <filesystem>
#define Get_Time() std::chrono::high_resolution_clock::now()
#define Duration(start) std::chrono::duration_cast<\
    std::chrono::microseconds>(Get_Time() - start).count()/(float)1000

//无对称的查询图仍然使用之前的方法
//从公共区域开始的匹配
long long Match::searchMatchedEdgeFromCommon(int vi, int vj)
{
	//std::cout<<"search from common"<<endl;
	// if(coutComm && !(currentvi == vj && currentvj == vi) )
	// {

	// 	string path = commpath + "/commInsertion.graph";
	// 	std::ofstream outFile(path, std::ios_base::app);
    // 	if (!outFile) return 1;

    // 	outFile << "e "<<vi<<" "<<vj<<" 0"<<endl;
    // 	outFile.close();
	// }
	// return 0;
	long long result = 0; 
	std::set<int> used_v;
	used_v.emplace(vi);
	used_v.emplace(vj);
	currentvi = vi;
	currentvj = vj;
	for (auto& i : CommSub.at(initG[vi].label).at(initG[vj].label))
	{
		for (auto& j : i.second)     //ui = i.first, uj = j
		{
			bool flag = checkBidirectionalEdgeOnIndex(vi, vj, i.first, j);
			if (flag)
			{
				if(symRepresenter.forbidStart.find(i.first) != symRepresenter.forbidStart.end())
				{
					if(symRepresenter.forbidStart.at(i.first).find(j) != symRepresenter.forbidStart.at(i.first).end())
					{
						continue;
					}
				}
				u_map cand = cmpInitCand(vi, vj, i.first, j); 
				//std::cout<<"Try Print InitCand at New Edge"<<vi<<"<->"<<vj<<"||"<<i.first<<"<->"<<j<<endl;
				//printMatchCand(cand);
				if (!cand.empty())
				{
					//enumeration starts here
					//clear SymGroup?
					//i j如果是对称点，那么cmpInitCand已经排除了vi vj这时候我们要加回去
					//同时在searchRemain的时候也要考虑是不是已经对称过了，也许就不需要再search一遍了
					std::unordered_map<int, int> m;
					m.emplace(i.first, vi);
					m.emplace(j, vj);
					int sum = orderCommSub.at(i.first).at(j).size();
					result += countCommSubMatch(i.first, j, 0, sum, cand, m, used_v, 0); 
					continue;
				}
			}
			int bg_q_id = (q[i.first].comm_id - 1) * numQForAComm;
			int ed_q_id = bg_q_id + numQForAComm;
			for (int k = bg_q_id + 1; k < ed_q_id; k++)
			{
				int dif = (k - bg_q_id) * numNodesForAQ;//这里疑似就是不同查询图的偏移量
				bool check = checkBidirectionalEdgeOnIndex(vi, vj, i.first + dif, j + dif);
				if (check)
				{
					u_map cand = cmpInitCand(vi, vj, i.first + dif, j + dif);
					if (!cand.empty())
					{
						std::unordered_map<int, int> m;
						m.emplace(i.first + dif, vi);
						m.emplace(j + dif, vj);
						int sum = orderCommSub.at(i.first).at(j).size();
						result += countCommSubMatch(i.first, j, 0, sum, cand, m, used_v, dif);
						break;
					}
				}
			}
		}
	}
	
	//join 剩余查询图
	if (Auxiliary.find(vi) != Auxiliary.end())
	{
		if (Auxiliary.at(vi).find(vj) != Auxiliary.at(vi).end())
		{
			int end_size = Auxiliary.at(vi).at(vj).candidate.size();//可以看出vi vj边会寻得很多可用的部分匹配来匹配remain
			for (int i = 0; i < end_size; i++)
			{
				u_map cand = Auxiliary.at(vi).at(vj).candidate.at(i);
				set<int> used_v = Auxiliary.at(vi).at(vj).used_v.at(i);
				unordered_map<int, int> m = Auxiliary.at(vi).at(vj).match.at(i); 
				int bg_q_id = (q[cand.begin()->first].comm_id - 1) * numQForAComm;
				int ed_q_id = bg_q_id + numQForAComm;
				int sum = orderRemainQ.at(bg_q_id).size();
				//大概在这里插入symGroupAuxCheck
				//std::cout<<"Ready To Match Joint by Auxiliary"<<endl;
				if (Auxiliary.at(vi).at(vj).comm_q_id.at(i) == bg_q_id)
				{   //生成公共子图的匹配结果是该组查询图的首个查询图

                    if(symGroupData[bg_q_id].matchNum == 1){result += countJoinedMatch(bg_q_id, 0, sum, cand, m, used_v, 0);}
                    else
                    {
						pair<int,int> startUiUj = Auxiliary.at(vi).at(vj).startUiUj.at(i);
						bool isForbidden = false;
						if(symGroupData[bg_q_id].forbidden.find(startUiUj.first) != symGroupData[bg_q_id].forbidden.end())
						{
							if(symGroupData[bg_q_id].forbidden.at(startUiUj.first).find(startUiUj.second) != symGroupData[bg_q_id].forbidden.at(startUiUj.first).end())
							{
								isForbidden = true;
							}
						}
						if(!isForbidden )
						{
							if(isSymmetryVertexSameNgbr(startUiUj.first,startUiUj.second))
								isForbidden = !(startUiUj.first<startUiUj.second&&vi<vj);
						}
						if(!isForbidden)
						{
							int multi = symGroupData[bg_q_id].startMulti.at(startUiUj.first).at(startUiUj.second);
							eachQManager.at(bg_q_id).symmetryUnit.clear();
							eachQManager.at(bg_q_id).unitVertices.clear();
							// marchG mg(numNodesForAQ,bg_q_id);
							// multi*=analyseInitCand_Joint_ver2(bg_q_id,cand,0,mg);
							multi*=analyseInitCand_Joint(bg_q_id,cand,0);
							if(multi<0)continue;
							result += newSym_countJoinedMatch(bg_q_id, 0, sum, cand, m, used_v, 0,multi);

							// result += newSym_countJoinedMatch(bg_q_id, 0, sum, cand, m, used_v, 0,multi,mg);
						}

                    }

					for (int j = bg_q_id + 1; j < ed_q_id; j++)
					{
						int dif = (j - bg_q_id) * numNodesForAQ;
						u_map other_cand = cmpJoinedInitCand(j, m, used_v, dif);
						if (!other_cand.empty())
						{
                            if(symGroupData[j].matchNum == 1)result += countJoinedMatch(j, 0, sum, other_cand, m, used_v, dif);
                            else
                            {
								pair<int,int> startUiUj = Auxiliary.at(vi).at(vj).startUiUj.at(i);
								int startEdgeDif = startUiUj.first - startUiUj.first%numNodesForAQ;
								startEdgeDif = j*numNodesForAQ - startEdgeDif;
								startUiUj.first += startEdgeDif;
								startUiUj.second += startEdgeDif;
								bool isForbidden = false;
								if(symGroupData[j].forbidden.find(startUiUj.first) != symGroupData[j].forbidden.end())
								{
									if(symGroupData[j].forbidden.at(startUiUj.first).find(startUiUj.second) != symGroupData[j].forbidden.at(startUiUj.first).end())
									{
										isForbidden = true;
									}
								}
								if(!isForbidden )
								{
									if(isSymmetryVertexSameNgbr(startUiUj.first,startUiUj.second))
										isForbidden = !(startUiUj.first<startUiUj.second&&vi<vj);
								}
								if(!isForbidden)
								{
									int multi = symGroupData[j].startMulti.at(startUiUj.first).at(startUiUj.second);
									eachQManager.at(j).symmetryUnit.clear();
									eachQManager.at(j).unitVertices.clear();
									// marchG mg(numNodesForAQ,j);
									// multi*=analyseInitCand_Joint_ver2(j,other_cand,dif,mg);
									multi*=analyseInitCand_Joint(j,other_cand,dif);
									if(multi<0)continue;
									result += newSym_countJoinedMatch(j, 0, sum, other_cand, m, used_v, dif,multi);

									// result += newSym_countJoinedMatch_ver2(j, 0, sum, other_cand, m, used_v, dif,multi,mg);
								}		
							}		
						}
					}
				}
				else
				{ 
					for (int j = Auxiliary.at(vi).at(vj).comm_q_id.at(i); j < ed_q_id; j++)
					{
						int dif = (j - Auxiliary.at(vi).at(vj).comm_q_id.at(i)) *numNodesForAQ;
						u_map other_cand = cmpJoinedInitCand(j, m, used_v, dif);
						if (!other_cand.empty())
						{
                            if(symGroupData[j].matchNum == 1)result += countJoinedMatch(j, 0, sum, other_cand, m, used_v, dif);
                            else
                            {
								pair<int,int> startUiUj = Auxiliary.at(vi).at(vj).startUiUj.at(i);
								int startEdgeDif = startUiUj.first - startUiUj.first%numNodesForAQ;
								startEdgeDif = j*numNodesForAQ - startEdgeDif;
								startUiUj.first += startEdgeDif;
								startUiUj.second += startEdgeDif;
								bool isForbidden = false;
								if(symGroupData[j].forbidden.find(startUiUj.first) != symGroupData[j].forbidden.end())
								{
									if(symGroupData[j].forbidden.at(startUiUj.first).find(startUiUj.second) != symGroupData[j].forbidden.at(startUiUj.first).end())
									{
										isForbidden = true;
									}
								}
								if(!isForbidden )
								{
									if(isSymmetryVertexSameNgbr(startUiUj.first,startUiUj.second))
										isForbidden = !(startUiUj.first<startUiUj.second&&vi<vj);
								}
								if(!isForbidden)
								{
									int multi = symGroupData[j].startMulti.at(startUiUj.first).at(startUiUj.second);
									eachQManager.at(j).symmetryUnit.clear();
									eachQManager.at(j).unitVertices.clear();
									// marchG mg(numNodesForAQ,j);
									// multi*=analyseInitCand_Joint_ver2(j,other_cand,dif,mg);
									multi*=analyseInitCand_Joint(j,other_cand,dif);
									if(multi<0)continue;
									result += newSym_countJoinedMatch(j, 0, sum, other_cand, m, used_v, dif,multi);

									// result += newSym_countJoinedMatch_ver2(j, 0, sum, other_cand, m, used_v, dif,multi,mg);
								}	
                            }
						}
					}
				}
			}
		}
	}
	return result;
}

int Match::countCommSubMatch(int ui, int uj, int index_u, int sum, u_map cand, unordered_map<int, int> match, set<int> used_v, int dif)
{//其实就是对图0的search，全部复制即可，注意辅助结构更新的方式可能会影响SymGroup存储
//甚至需要我们给SymGroup一个辅助结构配合使用
//	std::cout<<"countCommSubMatch"<<std::endl;
//	std::cout<<"ui uj index and sum : "<<ui<<" "<<uj<<" "<<index_u<<" "<<sum<<std::endl;
	int result = 0;
	if (index_u < sum - 1)
	{
		int u = orderCommSub.at(ui).at(uj)[index_u][0] + dif;
		if (cand.find(u) != cand.end())
		{
			for (auto& v : cand.at(u))
			{
				std::unordered_map<int, int> copy_match = match;
				copy_match.emplace(u, v);
				u_map copy_cand = cand;
				copy_cand.erase(u);
				std::set<int> copy_usedV = used_v;
				copy_usedV.emplace(v);
				copy_cand = updateCand(false, v, u, copy_cand, copy_match, copy_usedV, dif);
				int copy_index_u = index_u;
				if (!copy_cand.empty())
				{
					copy_index_u += 1;
					result += countCommSubMatch(ui, uj, copy_index_u, sum, copy_cand, copy_match, copy_usedV, dif);
				}
				else
				{
					partialMatch_num += 1;
				}
			}	
		}
	}
	else if (index_u == sum - 1)//即将来到公共部分匹配的尾巴
	{
		int u = orderCommSub.at(ui).at(uj)[index_u][0] + dif;
		if (cand.find(u) != cand.end())
		{
			for (auto& v : cand.at(u))
			{
				std::unordered_map<int, int> copy_match = match;
				copy_match.emplace(u, v);
				u_map copy_cand = cand;
				copy_cand.erase(u);
				std::set<int> copy_usedV = used_v;
				copy_usedV.emplace(v);
				copy_cand = updateCand(false, v, u, copy_cand, copy_match, copy_usedV, dif); 
				if (!copy_cand.empty())
				{
					//在这里加入了辅助结构，方便以后进行remain部分匹配的时候拿到已经匹配好的cand还是什么东西？
					//哦哦，看起来是把图0的匹配结果映射到其余的图上，存储在Auxiliary中
					addAuxiliary(q[u].q_id, copy_match.at(ui+dif), copy_match.at(uj+dif), 
						copy_match, copy_cand, copy_usedV,make_pair(ui,uj));
					//Adjust All Q's SymGroup()
				}
				else
				{
					if (copy_match.size() == numNodesForAQ)
					{   //公共子图是涵盖整个查询图的顶点
						result += 1;
						//Result+=Sym*1;
						//print_unordered_map(copy_match);
					}
					else
					{
						partialMatch_num += 1;
					}
				}
			}
		}
	}

	return result;
}

void Match::addAuxiliary(int q_id, int vi, int vj, std::unordered_map<int, int> match, 
		u_map cand, std::set<int> used_v,pair<int,int> uiuj)
{
	if (Auxiliary.find(vi) != Auxiliary.end())
	{
		if (Auxiliary.at(vi).find(vj) != Auxiliary.at(vi).end())
		{
			Auxiliary.at(vi).at(vj).match.emplace_back(match);
			Auxiliary.at(vi).at(vj).candidate.emplace_back(cand);
			Auxiliary.at(vi).at(vj).used_v.emplace_back(used_v);
			Auxiliary.at(vi).at(vj).comm_q_id.emplace_back(q_id);
			Auxiliary.at(vi).at(vj).startUiUj.emplace_back(uiuj);
		}
		else
		{
			Aux temp;
			temp.match.emplace_back(match);
			temp.candidate.emplace_back(cand);
			temp.used_v.emplace_back(used_v);
			temp.comm_q_id.emplace_back(q_id);
			temp.startUiUj.emplace_back(uiuj);
			Auxiliary.at(vi).emplace(vj, temp);
		}
	}
	else
	{
		Aux temp;
		temp.match.emplace_back(match);
		temp.candidate.emplace_back(cand);
		temp.used_v.emplace_back(used_v);
		temp.comm_q_id.emplace_back(q_id);
		temp.startUiUj.emplace_back(uiuj);
		unordered_map<int, Aux> temp2;
		temp2.emplace(vj, temp);
		Auxiliary.emplace(vi, temp2);
	}
}
//我们需要两个部分功能
//第一部分：对于数据结点，已经被选择为某一结点的Candi后，不再作为查询图点的对称点的候选
//第二部分：完成一次成功匹配后，记录使用点；每次更新Candi后，若一个点有对称点，需要查询对称点使用的Candi是什么，并且及时排除防止重复计算

int Match::countJoinedMatch(int q_id, int index_u, int sum, u_map cand, unordered_map<int, int> match, set<int> used_v, int dif)
{
	//std::cout<<"countJoinedMatch"<<std::endl;
	int result = 0;
	if (index_u < sum - 1)
	{
		int u = orderRemainQ.at(q_id)[index_u][0];//新的匹配结点u
		if (cand.find(u) != cand.end())
		{
            for (auto& v : cand.at(u))
            {

                std::unordered_map<int, int> copy_match = match;
                copy_match.emplace(u, v);
                u_map copy_cand = cand;
                copy_cand.erase(u);
                std::set<int> copy_usedV = used_v;
                copy_usedV.emplace(v);
                copy_cand = updateCand(true, v, u, copy_cand, copy_match, copy_usedV, dif);//在这里选择然后更新了一下candidate
                int copy_index_u = index_u;

                if ( !copy_cand.empty())
                {
                    copy_index_u += 1;
                    result += countJoinedMatch(q_id, copy_index_u, sum, copy_cand, copy_match, copy_usedV, dif);
                }
                else
                {	//这里是部分匹配，我们需要在这里去记录失败点
                    partialMatch_num += 1;
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
			set_difference(cand.at(u).begin(), cand.at(u).end(), used_v.begin(), used_v.end(), back_inserter(dif));
			result += dif.size();
			if(coutAble)
			{
				std::cout<<"NonSymJoint Result: "<<result<<" ";print_unordered_map(match);
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

//从剩余区域开始的匹配
int Match::searchMatchedEdgeFromRemainQ(int vi, int vj)
{
	//symGroupReflash();
	// std::cout<<"In searchMatchedEdgeFromRemainQ:"<<vi<<" "<<vj<<endl;
	// if(coutComm && !(currentvi == vj && currentvj == vi) )
	// {
	// 	string path = commpath + "/remainInsertion.graph";
	// 	std::ofstream outFile(path, std::ios_base::app);
    // 	if (!outFile) return 1;

    // 	outFile << "e "<<vi<<" "<<vj<<" 0"<<endl;
    // 	outFile.close();
	// }
	// return 0;
	int result = 0; 
	std::set<int> used_v;
	used_v.emplace(vi);
	used_v.emplace(vj);
	currentvi = vi;
	currentvj = vj;
	u_map cand;
	bool flag = false;
	std::unordered_map<int, int> match;
	// 对应下面第二个countRemainQMatch()
	for (auto& i : RemainQ.at(initG[vi].label).at(initG[vj].label))
	{    // ui = i.first, uj = j
		for (auto& j : i.second)
		{
			//std::cout<<"try"<<i.first<<" "<<j<<endl;
			//if(!symGroupCheckSearchFromRemainQ(q[i.first].q_id,i.first,j))continue;
			if(!symCheckFromRemainSimplified(q[i.first].q_id,i.first,j,vi,vj))continue;
			bool flag = checkBidirectionalEdgeOnIndex(vi, vj, i.first, j);
			//std::cout<<"checkThrough"<<endl;
			if (flag)
			{
				u_map cand = cmpInitCand(vi, vj, i.first, j);
				if (!cand.empty())
				{
					std::unordered_map<int, int> m;
					m.emplace(i.first, vi);
					m.emplace(j, vj);
					currentviToUi = i.first;
					currentvjToUj = j;
					//symGroupInitFromRemain(q[i.first].q_id,make_pair(vi,vj),make_pair(i.first,j));

					int sum = orderEdgeOfRemainQ.at(i.first).at(j).size();
                    if(symGroupData.at(q[i.first].q_id).matchNum == 1)result += countRemainQMatch(i.first, j, 0, sum, cand, m, used_v);
                    else
                    {
						//int multi = computeStartNum(i.first,j);
                        //result += symCountRemainQMatch(i.first, j, 0, sum, cand, m, used_v);
						int multi = symGroupData[q[i.first].q_id].startMulti.at(i.first).at(j);
						eachQManager.at(q[i.first].q_id).symmetryUnit.clear();
						eachQManager.at(q[i.first].q_id).unitVertices.clear();
						multi*=analyseInitCand_Joint(q[i.first].q_id,cand,0);
						if(multi<0)continue;
						// marchG mg(numNodesForAQ,q[i.first].q_id);
						// multi*=analyseInitCand_Joint_ver2(q[i.first].q_id,cand,0,mg);
						// result += symRemainMatch(i.first, j, 0, sum, cand, m, used_v,multi,mg);
						result += newSym_countRemainQMatch(i.first, j, 0, sum, cand, m, used_v,multi);
                    }
				}
			}
		}
	}

	return result;
}


int Match::countRemainQMatch(int ui, int uj, int index_u, int sum, u_map cand, unordered_map<int, int> match, set<int> used_v)
{
	int result = 0;
	if (index_u < sum - 1)
	{
		int u = orderEdgeOfRemainQ.at(ui).at(uj)[index_u][0];
		if (cand.find(u) != cand.end())
		{
			int q_id = q[u].q_id;
            for (auto& v : cand.at(u))
            {
                std::unordered_map<int, int> copy_match = match;
                copy_match.emplace(u, v);
                u_map copy_cand = cand;
                copy_cand.erase(u);

                std::set<int> copy_usedV = used_v;
                copy_usedV.emplace(v);
                copy_cand = updateCand(false, v, u, copy_cand, copy_match, copy_usedV, 0);//在这里选择然后更新了一下candidate
                int copy_index_u = index_u;

                if (!copy_cand.empty())
                {
                    copy_index_u += 1;
                    result += countRemainQMatch(ui, uj, copy_index_u, sum, copy_cand, copy_match, copy_usedV);
                }
                else
                {	//这里是部分匹配，我们需要在这里去记录失败点
                    partialMatch_num += 1;
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
			set_difference(cand.at(u).begin(), cand.at(u).end(), used_v.begin(), used_v.end(), back_inserter(dif));

			result += dif.size();
			if(coutAble)
			{
				std::cout<<"NonSymRemainOutcome Result: "<<result<<" ";print_unordered_map(match);
				std::cout<<"<"<<u<<",";
				for (auto& v : (newDif.empty()?dif:newDif))
				{
					std::cout<<"["<<v<<"]";
				}
				std::cout<<">"<<endl;
			}
		}
	
	}

	return result;
}