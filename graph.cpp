#include "graph.h"
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits.h>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

// constructor, empty graph
// directionalEdges defaults to true
Graph::Graph(bool directionalEdges) { dirEdges = directionalEdges; }

// destructor
Graph::~Graph() {
  for (auto &vert : vertGraph) {
    delete vert;
  }
  vertGraph.clear();
}

// @return total number of vertices
int Graph::verticesSize() const { return vertGraph.size(); }

// @return total number of edges
int Graph::edgesSize() const {
  int size = 0;
  for (auto *edge : vertGraph) {
    size += edge->edges.size();
  }
  if (!dirEdges) {
    return size / 2;
  }
  return size;
}

// helper to find a vertex by label
int Graph::findHelper(const string &label) const {
  if (vertGraph.empty()) {
    return -1;
  }
  for (int i = 0; i < vertGraph.size(); i++) {
    if (vertGraph[i]->label == label) {
      return i;
    }
  }
  return -1;
}

// @return number of edges from given vertex, -1 if vertex not found
int Graph::vertexDegree(const string &label) const {
  int found = findHelper(label);
  if (found != -1) {
    return vertGraph[found]->edges.size();
  }
  return found;
}

// @return true if vertex added, false if it already is in the graph
bool Graph::add(const string &label) {
  if (!contains(label)) {
    Vertex *newVert = new Vertex(label);
    for (int i = 0; i < vertGraph.size(); i++) {
      if (vertGraph[i]->label > label) {
        vertGraph.insert(vertGraph.begin() + i, newVert);
        return true;
      }
    }
    vertGraph.insert(vertGraph.end(), newVert);
    return true;
  }
  return false;
}

/** return true if vertex already in graph */
bool Graph::contains(const string &label) const {
  return (findHelper(label) != -1);
}

// @return string representing edges and weights, "" if vertex not found
// A-3->B, A-5->C should return B(3),C(5)
string Graph::getEdgesAsString(const string &label) const {
  int index = findHelper(label);
  string edgeStr;
  if (index != -1 && !vertGraph[index]->edges.empty()) {
    for (auto &edge : vertGraph[index]->edges) {
      edgeStr += (edge.first->label + "(" + to_string(edge.second) + "),");
    }
    edgeStr.erase(edgeStr.end() - 1);
  }
  return edgeStr;
}

// helper to find if a vertex has a neighbor
int Graph::neighborHelper(const string &vert, const string &label) const {
  if (vertGraph.empty()) {
    return -1;
  }
  int vertPos = findHelper(vert);
  if (vertPos != -1) {
    for (int i = 0; i < vertGraph[vertPos]->edges.size(); i++) {
      if (vertGraph[vertPos]->edges[i].first->label == label) {
        return i;
      }
    }
  }
  return -1;
}

// @return true if successfully connected
bool Graph::connect(const string &from, const string &to, int weight) {
  if (!this->contains(from)) {
    this->add(from);
  }
  if (from == to) {
    return false;
  }

  if (!this->contains(to) && from != to) {
    this->add(to);
  }

  int fromPos = findHelper(from);
  int toPos = findHelper(to);
  if (neighborHelper(from, to) == -1) {
    bool insertedFrom = false;
    for (int i = 0; i < vertGraph[fromPos]->edges.size() && !insertedFrom;
         i++) {
      if (vertGraph[fromPos]->edges[i].first->label > to) {
        vertGraph[fromPos]->edges.insert(
            vertGraph[fromPos]->edges.begin() + i,
            pair<Vertex *, int>(vertGraph[toPos], weight));
        insertedFrom = true;
      }
    }
    if (!insertedFrom) {
      vertGraph[fromPos]->edges.insert(
          vertGraph[fromPos]->edges.end(),
          pair<Vertex *, int>(vertGraph[toPos], weight));
    }
    if (!dirEdges) {
      bool insertedTo = false;
      for (int i = 0; i < vertGraph[toPos]->edges.size() && !insertedTo; i++) {
        if (vertGraph[toPos]->edges[i].first->label > from) {
          vertGraph[toPos]->edges.insert(
              vertGraph[toPos]->edges.begin() + i,
              pair<Vertex *, int>(vertGraph[fromPos], weight));
          insertedTo = true;
        }
      }
      if (!insertedTo) {
        vertGraph[toPos]->edges.insert(
            vertGraph[toPos]->edges.end(),
            pair<Vertex *, int>(vertGraph[fromPos], weight));
      }
    }
    return true;
  }
  return false;
}

bool Graph::disconnect(const string &from, const string &to) {
  if (from == to) {
    return false;
  }
  if (!this->contains(from) || !this->contains(to)) {
    return false;
  }
  int fromPos = findHelper(from);
  int toPos = findHelper(to);
  for (int i = 0; i < vertGraph[fromPos]->edges.size(); i++) {
    if (vertGraph[fromPos]->edges[i].first->label == to) {
      vertGraph[fromPos]->edges.erase(vertGraph[fromPos]->edges.begin() + i);
      if (!dirEdges) {
        for (int j = 0; j < vertGraph[toPos]->edges.size(); j++) {
          if (vertGraph[toPos]->edges[j].first->label == from) {
            vertGraph[toPos]->edges.erase(vertGraph[toPos]->edges.begin() + j);
          }
        }
      }
      return true;
    }
  }
  return false;
}

// depth-first traversal starting from given startLabel
void Graph::dfs(const string &startLabel, void visit(const string &label)) {
  if (!this->contains(startLabel)) {
    return;
  }
  unordered_set<string> visited;
  stack<string> s;
  s.push(startLabel);

  while (!s.empty()) {
    string current = s.top();
    s.pop();

    if (visited.find(current) == visited.end()) {
      visit(current);
      visited.insert(current);
    }

    vector<string> neighbors;
    for (auto &neighbor : vertGraph[findHelper(current)]->edges) {
      neighbors.push_back(neighbor.first->label);
    }

    sort(neighbors.rbegin(), neighbors.rend());
    for (auto &neighbor : neighbors) {
      if (visited.find(neighbor) == visited.end()) {
        s.push(neighbor);
      }
    }
  }
}

// breadth-first traversal starting from startLabel
void Graph::bfs(const string &startLabel, void visit(const string &label)) {
  if (!this->contains(startLabel)) {
    return;
  }

  unordered_set<string> visited;
  queue<string> q;
  q.push(startLabel);

  while (!q.empty()) {
    string current = q.front();
    q.pop();

    if (visited.find(current) == visited.end()) {
      visit(current);
      visited.insert(current);
    }

    vector<string> neighbors;
    for (auto &neighbor : vertGraph[findHelper(current)]->edges) {
      neighbors.push_back(neighbor.first->label);
    }

    for (auto &neighbor : neighbors) {
      if (visited.find(neighbor) == visited.end()) {
        q.push(neighbor);
      }
    }
  }
}

// store the weights in a map
// store the previous label in a map
pair<map<string, int>, map<string, string>>
Graph::dijkstra(const string &startLabel) const {
  map<string, int> weights;
  map<string, string> previous;
  if (this->contains(startLabel)) {
    if (vertGraph[findHelper(startLabel)] == vertGraph.back()) {
      return make_pair(weights, previous);
    }
    weights[startLabel] = 0;

    auto comp = [&weights](const string &a, const string &b) {
      return weights[a] > weights[b];
    };
    priority_queue<string, vector<string>, decltype(comp)> pq(comp);
    pq.push(startLabel);

    while (!pq.empty()) {
      string curr = pq.top();
      pq.pop();

      for (auto &neighbor : vertGraph[findHelper(curr)]->edges) {
        int altWeight = weights[curr] + neighbor.second;
        if (weights.find(neighbor.first->label) == weights.end() ||
            altWeight < weights[neighbor.first->label]) {
          weights[neighbor.first->label] = altWeight;
          previous[neighbor.first->label] = curr;
          pq.push(neighbor.first->label);
        }
      }
    }
    weights.erase(startLabel);
  }
  return make_pair(weights, previous);
}

// helper for prim
void Graph::primHelper(Vertex *current,
                       map<Vertex *, pair<vector<string>, int>> &edgeMap,
                       int dist, vector<string> path,
                       map<Vertex *, int> &distMap, int covered) const {
  if (edgeMap.count(current) == 0 || edgeMap.at(current).second > dist) {
    path.emplace_back(current->label);

    if (edgeMap.count(current) == 0) {
      edgeMap.emplace(current, make_pair(path, dist));
    } else {
      edgeMap.at(current) = make_pair(path, dist);
    }

    if (distMap.count(current) == 0) {
      distMap.emplace(current, covered);
    } else {
      distMap.at(current) = covered;
    }

    for (auto &edge : current->edges) {
      primHelper(edge.first, edgeMap, dist + edge.second, path, distMap,
                 edge.second);
    }
  }
}

// minimum spanning tree using Prim's algorithm
int Graph::mstPrim(const string &startLabel,
                   void visit(const string &from, const string &to,
                              int weight)) const {
  if (this->contains(startLabel)) {
    vector<string> startVec;
    map<Vertex *, pair<vector<string>, int>> edgeMap;
    map<Vertex *, int> distMap;

    int weight = 0;
    primHelper(vertGraph[findHelper(startLabel)], edgeMap, 0, startVec, distMap,
               0);

    while (!edgeMap.empty()) {
      int low = INT_MAX;
      int lowPos = 0;
      for (int i = 0; i < vertGraph.size(); i++) {
        if (edgeMap.count(vertGraph[i]) != 0 &&
            edgeMap.at(vertGraph[i]).second < low) {
          low = edgeMap.at(vertGraph[i]).second;
          lowPos = i;
        }
      }

      if (edgeMap.at(vertGraph[lowPos]).first.size() > 1) {
        visit(edgeMap.at(vertGraph[lowPos])
                  .first.at(edgeMap.at(vertGraph[lowPos]).first.size() - 2),
              edgeMap.at(vertGraph[lowPos])
                  .first.at(edgeMap.at(vertGraph[lowPos]).first.size() - 1),
              distMap.at(vertGraph[lowPos]));
        weight += distMap.at(vertGraph[lowPos]);
      }
      edgeMap.erase(vertGraph[lowPos]);
    }
    return weight;
  }
  return -1;
}

// minimum spanning tree using Kruskal's algorithm
int Graph::mstKruskal(void visit(const string &from, const string &to,
                                 int weight)) const {
  return -1;
}

// read a text file and create the graph
bool Graph::readFile(const string &filename) {
  ifstream myfile(filename);
  if (!myfile.is_open()) {
    cerr << "Failed to open " << filename << endl;
    return false;
  }
  int edges = 0;
  int weight = 0;
  string fromVertex;
  string toVertex;
  myfile >> edges;
  for (int i = 0; i < edges; ++i) {
    myfile >> fromVertex >> toVertex >> weight;
    connect(fromVertex, toVertex, weight);
  }
  myfile.close();
  return true;
}