/* Homework 2: Implement Dijkstra's Algorithm
 * Author: Oleh Pomazan <oleh.pomazan@gmail.com>
 *
 * Description
 *
 * The graph ADT was implemented both using adjacency lists and matrix
 * in two classes inherited from AbstractGraph: AdjListGraph and MatrixGraph.
 * Neighbor helper class is used to store vertex and weight of the edge pointed into it.
 *
 * The Dijkstra algorithm is in ShortestPath class and it uses std::priority_queue
 * as min-heap. Priority queue stores the pair of minimal distance and vertex number,
 * also there's "previous" vector to store the path of visited nodes which is used to
 * obtain the shortest path.
 *
 * Random graphs are produced in the MonteCarloSimulation class, average path length is calculated
 * in random_graph method.
 *
 * Also there're two test functions to check the correctness of the implementation of Dijkstra algorithm
 * for both list and matrix based graphs.
*/

#include <iostream>
#include <vector>
#include <string>
#include <list>
#include <queue>
#include <limits>
#include <utility>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <random>

/* Store the end of the edge and weight of the edge */
template <typename weight_type>
struct Neighbor
{
    int vertex;
    weight_type weight;
    Neighbor(int v) : vertex(v) {}
    Neighbor(int v, weight_type w) : vertex(v), weight(w) {}
};

/* Abstract graph class provides general interface for derived list
 * and matrix based classes. Template's argument "vertex_type" is used
 * as node's value (number, string), "weight_type" is used as edge distance (double)
*/
template <typename vertex_type, typename weight_type>
class AbstractGraph
{
private:
    void operator=(const AbstractGraph&) = delete; // protect assignment
    AbstractGraph(const AbstractGraph&) = delete;  // protect copy constructor

public:
    AbstractGraph() {}
    virtual ~AbstractGraph() {}

    virtual int V() const =0; // returns the number of vertices in the graph
    virtual int E() const =0; // returns the number of edges in the graph

    virtual bool adjacent(int x, int y) const =0; // tests whether there is an edge from node "x" to node "y"
    virtual std::vector< Neighbor<weight_type> > neighbors(int x) const =0; // return list of all vertices which have edge with "x"

    virtual void addEdge(int x, int y) =0; // adds to graph the edge from x to y, if it is not there
    virtual void deleteEdge(int x, int y) =0; // removes the edge from x to y, if it is there

    virtual vertex_type get_node_value(int x) const =0; // returns the value associated with the node x
    virtual void set_node_value(int x, vertex_type value) =0; // sets the value associated with the node x to a

    virtual weight_type get_edge_value(int x, int y) =0; // returns the value associated to the edge (x,y)
    virtual void set_edge_value(int x, int y, weight_type v) =0; // sets the value associated to the edge (x,y) to v
};

/* Adjacency list-based implementation of graph ADT */
template <typename vertex_type, typename weight_type>
class AdjListGraph : public AbstractGraph<vertex_type, weight_type>
{
private:
    int numVertex, numEdge;
    // stores vector of lists of neighbors (using std::list would be less time consuming for insertion operations,
    // but it not so imporatant because we only insert edges at the end of vector and never in the middle and never delete edges in this task)
    std::vector< std::vector<Neighbor<weight_type> > > AdjacencyList;
    std::vector<vertex_type> vertices; // store nodes values

public:
    AdjListGraph() {}
    AdjListGraph(int numVert) : numVertex(numVert), numEdge(0)
    {
        vertices.reserve(numVertex);
        AdjacencyList.reserve(numVertex);

        for (int i = 0; i < numVertex; i++)
        {
            AdjacencyList.push_back(std::vector< Neighbor<weight_type> >());
            AdjacencyList[i].reserve(numVertex);
        }
    }
    ~AdjListGraph() { }

    int V() const { return numVertex; }
    int E() const { return numEdge; }

    bool adjacent(int x, int y) const
    {
        // return true if adjacency list for "x" node contains "y" node
        // lambda function checks every element of list for condition
        // std::find_if returns iterator which is equal to end() if "y" was not found
        return std::find_if(AdjacencyList[x].begin(), AdjacencyList[x].end(), [&y](Neighbor<weight_type> elem){
            return elem.vertex == y;
        }) != AdjacencyList[x].end();
    }

    std::vector<Neighbor<weight_type> > neighbors(int x) const
    {
        return AdjacencyList[x];
    }

    void addEdge(int x, int y)
    {
        assert(x >= 0 || y >= 0);
        addEdge(x,y,weight_type()); // cannot predict default value for "weight_type", use possible default constructor
                                    // it's assumed weight_type = double, it's just 0.0
    }

    // overloaded version with edge value for 3rd argument
    void addEdge(int x, int y, weight_type w)
    {
        assert(x >= 0 || y >= 0 || w >= 0);
        for (auto &it : AdjacencyList[x])
        {
            if (it.vertex == y)
            {
                it.weight = w;
                return;
            }
        }
        numEdge++;
        AdjacencyList[x].push_back(Neighbor<weight_type>(y,w));
    }

    void deleteEdge(int x, int y)
    {
        assert(x >= 0 || y >= 0);
        // look for the "y" vertex in the adj list
        auto nit = std::find_if(AdjacencyList[x].begin(), AdjacencyList[x].end(), [&y](Neighbor<weight_type> elem){
                return elem.vertex == y;
        });
        if (nit != AdjacencyList[x].end()) // if find - erase
        {
            numEdge--;
            AdjacencyList[x].erase(nit);
        }
    }

    vertex_type get_node_value(int x) const
    {
        assert(x >= 0);
        return vertices[x];
    }

    void set_node_value(int x, vertex_type value)
    {
        if (x >= 0) return;
        vertices[x] = value;
    }

    weight_type get_edge_value(int x, int y)
    {
        assert(x >= 0 || y >= 0);
        auto nit = std::find_if(AdjacencyList[x].begin(), AdjacencyList[x].end(), [&y](Neighbor<weight_type> elem){
                return elem.vertex == y;
        });
        if (nit != AdjacencyList[x].end())
            return nit->weight;
        return weight_type(); // return 0.0 if there's no edge
    }

    void set_edge_value(int x, int y, weight_type w)
    {
        assert(x >= 0 || y >= 0);
        auto nit = std::find_if(AdjacencyList[x].begin(), AdjacencyList[x].end(), [&y](Neighbor<weight_type> elem){
                return elem.vertex == y;
        });
        if (nit != AdjacencyList[x].end())
            nit->weight = w;
    }

    void print()
    {
        std::cout << "Adjacency list graph\n";
        std::cout << "Number of vertices: " << numVertex << std::endl;
        std::cout << "Number of edges: " << numEdge << std::endl;
        for (int i = 0; i < numVertex; i++)
        {
            std::cout << "V" << i << ": ";
            for (auto &elem : AdjacencyList[i])
                std::cout << "[" << elem.vertex << ", w" << elem.weight << "] ";
            std::cout << std::endl;
        }
    }
};

/* Matrix based graph class */
template <typename vertex_type, typename weight_type>
class MatrixGraph : public AbstractGraph<vertex_type, weight_type>
{
private:
    int numVertex, numEdge;
    std::vector< std::vector<weight_type> > matrix; // adjacency matrix
    std::vector<vertex_type> vertices; // store nodes values
public:
    MatrixGraph(int numVert) : numVertex(numVert), numEdge(0)
    {
        matrix.reserve(numVertex);
        for (int i = 0; i < numVertex; i++)
        {
            matrix.push_back(std::vector<weight_type>());
            matrix[i].reserve(numVertex);
            for (int j = 0; j < numVertex; j++)
            {
                matrix[i].push_back(0.0); // initialize by zero
            }
        }
    }
    ~MatrixGraph() {}

    int V() const { return numVertex; }
    int E() const { return numEdge; }

    bool adjacent(int x, int y) const
    {
        assert(x >= 0 || y >= 0);
        return matrix[x][y] != 0.0;
    }

    std::vector<Neighbor<weight_type> > neighbors(int x) const
    {
        assert(x >= 0);
        std::vector<Neighbor<weight_type> > nbrs;
        for (int i = 0; i < numVertex; i++)
            if (matrix[x][i] != 0)
                nbrs.push_back(Neighbor<weight_type>(i,matrix[x][i]));
        return nbrs;
    }

    void addEdge(int x, int y)
    {
        assert(x >= 0 || y >= 0);
        addEdge(x,y,weight_type());
    }

    void addEdge(int x, int y, weight_type w)
    {
        assert(x >= 0 || y >= 0);
        if (matrix[x][y] == 0.0)
            numEdge++;
        matrix[x][y] = w;
    }

    void deleteEdge(int x, int y)
    {
        assert(x >= 0 || y >= 0);
        if (matrix[x][y] != 0.0)
            numEdge--;
        matrix[x][y] = 0.0;
    }

    weight_type get_edge_value(int x, int y)
    {
        assert(x >= 0 || y >= 0);
        return matrix[x][y];
    }

    void set_edge_value(int x, int y, weight_type w)
    {
        assert(x >= 0 || y >= 0 || w >= 0);
        matrix[x][y] = w;
    }

    vertex_type get_node_value(int x) const
    {
        assert(x >= 0);
        return vertices[x];
    }

    void set_node_value(int x, vertex_type value)
    {
        assert(x >= 0);
        vertices[x] = value;
    }

    void print()
    {
        std::cout << "Matrix-based graph\n";
        std::cout << "Number of vertices: " << numVertex << std::endl;
        std::cout << "Number of edges: " << numEdge << std::endl;
        for (int i = 0; i < numVertex; i++)
        {
            std::cout << "V" << i << ": ";
            for (int j = 0; j < numVertex; j++)
            {
                if (matrix[i][j] != 0)
                    std::cout << "[" << j << ",w" << matrix[i][j] << "] ";
            }
            std::cout << std::endl;
        }
    }
};

/* Class containing implementation of Dijkstra algorithm */
template <typename vertex_type, typename weight_type>
class ShortestPath
{
    AbstractGraph<vertex_type, weight_type> *g;
    std::vector<weight_type> min_distance; // store distances from source veritice
    std::vector<int> previous; // store history of visiting, helps to restore the shortest path
    std::vector<int> _path; // store shortest path
public:
    ShortestPath(AbstractGraph<vertex_type, weight_type> *_g) : g(_g) {}

    // Dijkstra algorithm implementation similarly to http://rosettacode.org/wiki/Dijkstra%27s_algorithm#C.2B.2B
    std::vector< int > path(int source, int target)
    {
        int n = g->V(); // number of vertices
        min_distance.clear();
        min_distance.resize(n, std::numeric_limits<double>::infinity()); // set distances to infinity
        min_distance[source] = 0;

        previous.clear();
        previous.resize(n, -1); // store vertices to reproduce the shortest path
        _path.clear();

        std::priority_queue< std::pair<weight_type, int>,
                std::vector<std::pair<weight_type, int> >,
                std::greater<std::pair<weight_type, int> > > vertex_queue; // min-heap
        vertex_queue.push(std::make_pair(min_distance[source], source)); // put the start vertice to queue

        // iterate until queue is empty
        while (!vertex_queue.empty())
        {
            weight_type dist = vertex_queue.top().first; // distance
            int u = vertex_queue.top().second; // vertex
            vertex_queue.pop();

            if (dist < min_distance[u])
                continue;

            const std::vector<Neighbor<weight_type>> &neighbors = g->neighbors(u); // get all neighbors of "u"
            for (auto &neighbor : neighbors) // iterate over neighbors
            {
                int v = neighbor.vertex;
                weight_type weight = neighbor.weight;
                weight_type distance_through_u = dist + weight; // calcualate the cost
                if (distance_through_u < min_distance[v])
                {
                    min_distance[v] = distance_through_u;
                    previous[v] = u;
                    vertex_queue.push(std::make_pair(min_distance[v], v)); // push on top of heap the most "effective" vertex
                }
            }
        }
        // retrieve the path vertices
        int targetVertex = target;
        for (; target != -1; target = previous[target])
        {
            _path.push_back(target);
        }
        std::reverse(_path.begin(), _path.end()); // set correct order for path vertices
        if (_path.size() < 2)
            std::cout << "There's no path to " << targetVertex << ".\n";
        return _path;
    }

    // Calculate the cost as sum of distances between path vertices
    weight_type path_size(int source, int target)
    {
        path(source, target);
        weight_type sum = 0;

        for (int i = 0; i < _path.size() - 1; i++)
        {
            sum += g->get_edge_value(_path[i], _path[i+1]);
        }
        if (sum == 0) return -1;
        return sum;
    }
};

// Test for graph as used in http://rosettacode.org/wiki/Dijkstra%27s_algorithm#C.2B.2B
// Correct path is 0 2 5 4, cost is 20
void test1()
{
    AdjListGraph<int,double> g(6);
    // 0
    g.addEdge(0,1,7);
    g.addEdge(0,2,9);
    g.addEdge(0,5,14);
    // 1
    g.addEdge(1,0,7);
    g.addEdge(1,2,10);
    g.addEdge(1,3,15);
    // 2
    g.addEdge(2,0,9);
    g.addEdge(2,1,10);
    g.addEdge(2,3,11);
    g.addEdge(2,5,2);
    // 3
    g.addEdge(3,1,15);
    g.addEdge(3,2,11);
    g.addEdge(3,4,6);
    // 4
    g.addEdge(4,3,6);
    g.addEdge(4,5,9);
    // 5
    g.addEdge(5,0,14);
    g.addEdge(5,2,2);
    g.addEdge(5,4,9);

    g.print();

    ShortestPath<int, double> sp(&g);
    auto path = sp.path(0, 4);
    std::cout << "Path: ";
    std::copy(path.begin(), path.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
    std::cout << "Path cost: " << sp.path_size(0, 4) << std::endl;
}

void test2()
{
    MatrixGraph<int,double> g(6);
    // 0
    g.addEdge(0,1,7);
    g.addEdge(0,2,9);
    g.addEdge(0,5,14);
    // 1
    g.addEdge(1,0,7);
    g.addEdge(1,2,10);
    g.addEdge(1,3,15);
    // 2
    g.addEdge(2,0,9);
    g.addEdge(2,1,10);
    g.addEdge(2,3,11);
    g.addEdge(2,5,2);
    // 3
    g.addEdge(3,1,15);
    g.addEdge(3,2,11);
    g.addEdge(3,4,6);
    // 4
    g.addEdge(4,3,6);
    g.addEdge(4,5,9);
    // 5
    g.addEdge(5,0,14);
    g.addEdge(5,2,2);
    g.addEdge(5,4,9);

    g.print();

    ShortestPath<int, double> sp(&g);
    auto path = sp.path(0, 4);
    std::cout << "Path: ";
    std::copy(path.begin(), path.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
    std::cout << "Path cost: " << sp.path_size(0, 4) << std::endl;
}

/* Graph of given size and edge density is created using random numbers generator.
 * The average path length is calculated and printed out.
 */
class MonteCarloSimulation
{
public:
    void random_graph(int graph_size, double edge_dens, double min_dist, double max_dist)
    {
        AdjListGraph<int, double> g(graph_size);
        // http://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> distance_distribution(min_dist, max_dist);
        std::uniform_real_distribution<> edge_distribution(0.0, 1.0);
        for (int i = 0; i < graph_size; ++i)
        {
            for (int j = 0; j < graph_size; j++)
            if (edge_distribution(gen) <= edge_dens)
            {
                if (!(g.adjacent(i,j) && g.adjacent(j,i)))
                {
                    double dist = distance_distribution(gen);
                    g.addEdge(i,j, dist); // set two directed edges as one undirected
                    g.addEdge(j,i, dist);
                }
            }
        }
        std::cout << "Number of edges: " << g.E()/2 << std::endl; // half of edges due to graph is undirected

        ShortestPath<int,double> sp(&g);
        double sum = 0;
        int count = 0;
        for (int i = 1; i < graph_size; i++)
        {
            int path_len = sp.path_size(0,i);
            if (path_len != -1)
            {
                sum += path_len;
                count++;
            }
        }
        std::cout << "Average path length: " << sum / count << std::endl;
    }
};

int main()
{
//    test1();
//    test2();

    MonteCarloSimulation ms;
    ms.random_graph(50, 0.2, 1.0, 10.0); // graph size, edge density, min distance, max distance
    ms.random_graph(50, 0.4, 1.0, 10.0);

    return 0;
}

