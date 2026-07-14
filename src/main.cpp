#include <chrono>
#include <string>
#include <vector>
#include <iostream>
#include "graph.h"
#include "match.h"
#include <time.h>
#include <string.h>
#include <cstring>
#include "type.h"
#include <sys/resource.h>

#ifdef WIN32
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <direct.h>
#include <process.h>
#else
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#define VMRSS_LINE 22

using namespace std;

long getMemoryUse() {
    int who = RUSAGE_SELF;
    struct rusage usage;
    getrusage(who, &usage);
    return usage.ru_maxrss;
}

// get current process pid
inline int GetCurrentPid()
{
    return getpid();
}

// get specific process physical memeory occupation size by pid (MB)
inline float GetMemoryUsage(int pid)
{
#ifdef WIN32
    uint64_t mem = 0, vmem = 0;
    PROCESS_MEMORY_COUNTERS pmc;

    // get process hanlde by pid
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (GetProcessMemoryInfo(process, &pmc, sizeof(pmc)))
    {
        mem = pmc.WorkingSetSize;
        vmem = pmc.PagefileUsage;
    }
    CloseHandle(process);

    // use GetCurrentProcess() can get current process and no need to close handle

    // convert mem from B to MB
    return mem / 1024.0 / 1024.0;

#else
    char file_name[64] = { 0 };
    FILE* fd;
    char line_buff[512] = { 0 };
    sprintf(file_name, "/proc/%d/status", pid);

    fd = fopen(file_name, "r");
    if (nullptr == fd)
        return 0;

    char name[64];
    int vmrss = 0;
    for (int i = 0; i < VMRSS_LINE - 1; i++)
        fgets(line_buff, sizeof(line_buff), fd);

    fgets(line_buff, sizeof(line_buff), fd);
    sscanf(line_buff, "%s %d", name, &vmrss);
    fclose(fd);

    // cnvert VmRSS from KB to MB
    return vmrss / 1024.0;
#endif
}

namespace mem
{
    /**
     * get peak virtual memory space of the current process
     * https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process#answer-64166
     */
    inline int parseLine(char *line)
    {
        // This assumes that a digit will be found and the line ends in " Kb".
        int i = strlen(line);
        const char *p = line;
        while (*p < '0' || *p > '9')
            p++;
        line[i - 3] = '\0';
        i = atoi(p);
        return i;
    }

    inline int getValue()
    { // Note: this value is in KB!
        FILE *file = fopen("/proc/self/status", "r");
        int result = -1;
        char line[128];

        while (fgets(line, 128, file) != NULL)
        {
            if (strncmp(line, "VmPeak:", 7) == 0)
            {
                result = parseLine(line);
                break;
            }
        }
        fclose(file);
        return result;
    }

}

// Time counting
#define Get_Time() std::chrono::high_resolution_clock::now()
#define Duration(start) std::chrono::duration_cast<                        \
                            std::chrono::microseconds>(Get_Time() - start) \
                                .count() /                                 \
                            (float)1000
#define Print_Time(str, start) std::cout << str << Duration(start) << " " << Duration(start) / 1000 << std::endl

int main(int argc, char *argv[])
{
    std::chrono::high_resolution_clock::time_point start, start_index;

    // 数据集地址
// string d_path = "/root/autodl-tmp/mxm/dataset/email-dnc-corecipient/ins0.3/data.graph";
// string q_path = "/root/autodl-tmp/mxm/querys/exp3/dense/ol0.4/sr04.txt";
// string u_path = "/root/autodl-tmp/mxm/dataset/email-dnc-corecipient/ins0.3/insertion.graph";
string d_path = "/root/autodl-tmp/mzy/mzy-data/datasets/youtube/ins_0.6/data.graph";
string q_path = "/root/autodl-tmp/mxm/experiment/exp3/querys/youtube/ol60.txt";
string u_path = "/root/autodl-tmp/mzy/mzy-data/datasets/youtube/ins_0.6/insertion.graph";
// string d_path = "/root/autodl-tmp/mzy/mzy-data/datasets/wikipedia/ins_0.6/data.graph";
// string q_path = "/root/autodl-tmp/mxm/experiment/exp5/querys/wikipedia/sr60.txt";
// string u_path = "/root/autodl-tmp/mzy/mzy-data/datasets/wikipedia/ins_0.6/insertion.graph";
// string comm_path;
// bool coutComm = false;
// string d_path = "/home/mxm/MQMatchExtension/Data/BigTest/data.graph";
// string q_path = "/home/mxm/MQMatchExtension/Data/BigTest/Q_0_49";
// string u_path = "/home/mxm/MQMatchExtension/Data/BigTest/insertion.graph";

        for (int i = 1; i < argc && argc>1; i++) {
            //cout << "change datasets path" << endl;
            if (string(argv[i]) == "-q")
                q_path = argv[i + 1];
            else if (string(argv[i]) == "-d")
                d_path = argv[i + 1];
            else if (string(argv[i]) == "-u")
                u_path = argv[i + 1];
            // else if (string(argv[i]) == "-commedge")
            // {
            //     coutComm = true;
            //     std::cout<<"coutComm = true"<<endl;
            //     comm_path = argv[i+1];
            // }
            //else if (string(argv[i]) == "-c")  // 表示选取数据流前面的多少行，默认是所有
                //cnum = atoi(argv[i + 1]);
        }
    /*printf("argc=%d\n", argc);
    for (int i = 0; i < argc; ++i)
    printf("argv[%d]=%s\n", i, argv[i]);*/
    
    // 读取数据
    Graph graph{};
    graph.inputQ(q_path);
    graph.inputG(d_path);
    graph.inputUpdate(u_path);
    // BasicComputation::printCombinationTable();
    std::cout<<"input Complete"<<endl;
    graph.genOrder();
    std::cout << "Order Generation Complete" << endl;
    // return 0;
    // 构建索引，计算数据顶点的候选信息
    int bg_curr_pid = GetCurrentPid();
    float bg_memory_usage = GetMemoryUsage(bg_curr_pid);
    start_index = Get_Time();
    graph.constructIndex();
    std::cout << "Index Construction Complete" << endl;
    graph.symProcess();
    Print_Time("processing time of initial index (ms s): ", start_index);
    int ed_cur_pid = GetCurrentPid();
    float ed_memory_usage = GetMemoryUsage(ed_cur_pid);
    std::cout << "memory usage of initial index (MB): " << ed_memory_usage - bg_memory_usage << std::endl;

    /* #region  */
    // 获取数据
    std::vector<vertex_Q> &qg = graph.getQ();
    two_u_map &Q_adjList = graph.getQAdjList();
    std::unordered_map<int, std::multimap<int, vec>> &qg_edges = graph.getQueryEdges();
    std::vector<vertex_G> &initialG = graph.getG();
    int* updateStream = graph.getStream();
    int* range = graph.getRange();
    int updateStreamSize = graph.getStreamNum();
    tri_u_map &commSub = graph.getLabelEdgeOfCommSub();
    tri_u_map &remainQ = graph.getLabelEdgeOfRemainingQ();
    two_umap_twovec &od_commSub = graph.getOrderOfCommSub();
    umap_twovec &od_remainQ = graph.getOrderOfRemainQ();
    two_umap_twovec &od_edgeOfRemainQ = graph.getOrderOfEdgeOfRemainQ();
    two_u_map &remainQ_adjList = graph.getRemainQ_adjList();
    two_u_map &remainQ_initialCandU = graph.getRemainQ_initialCandU();
    std::vector<Graph::od_node> &ot_node = graph.getOdTreeNodes();
    two_u_map &ot_bgNode = graph.getOdTreeStartNodes();
    tri_u_map &ot = graph.getOrderTree();
    std::vector<SymGroupData>& sd = graph.getSymGroupData();
    SymmetryRepresenter& symRepresenter = graph.getSymRepresenter();
    std::vector<SymmetryRecord> QSym = graph.getQSym();
    /* #endregion */

    // 更新和匹配
    Match match = Match(Q_adjList, qg, qg_edges, initialG, updateStream, commSub, remainQ,
                        remainQ_adjList, remainQ_initialCandU,
                        od_commSub, od_remainQ, od_edgeOfRemainQ, ot_node, ot_bgNode, ot, sd, 
                        symRepresenter,range,updateStreamSize,QSym);
    graph.clear();
    start = Get_Time(); // clock_t time = clock();
    match.coutAble = false;
    // if(coutComm)
    // {
    //     match.coutComm = true;
    //     match.commpath = comm_path;
    // }
    match.updateAndMatch();
    auto timeCost = Duration(start);
    std::cout << "Incremental Matching (ms s): " << timeCost << " " << timeCost / 1000 << std::endl;
    std::cout << "Peak Virtual Memory (KB): " << mem::getValue() << std::endl;
    cout << "Index Maintenance Time: " << match.getTime_indexUpdate() << endl;
    cout << "Search Time: " << match.getTime_search() << endl;
    cout << "Partial Matches Number= " << match.getPartialMatchNum() << endl;
    cout << "EPS= " << fixed << setprecision(6) << float(match.allMatch)*1000/timeCost << endl;
    cout << "Memory usage : " << getMemoryUse() << "KB" << endl;
    cout << "Memory usage : " << getMemoryUse() / 1024 << "MB" << endl;
    cout << "--------------end---------------" << endl;
}
