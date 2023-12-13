#ifndef VEIN_NODE_H
#define VEIN_NODE_H

#include <utility>
#include <vector>
#include <cmath>

class VeinNode{
    private:
        float x;
        float y;
        VeinNode* parent = nullptr;
        std::vector<VeinNode*> children;
        std::vector<std::pair<float,float>> auxinSrcs;
    public:
        explicit VeinNode(float pos_x, float pos_y): x(pos_x), y(pos_y){}
        float getX(){
            return x;
        }
        float getY(){
            return y;
        }
        std::vector<VeinNode*> getChildren(){
            return children;
        }
        std::vector<std::pair<float,float>> getAuxinSrcs(){
            return auxinSrcs;
        }
        bool hasChildren(){
            return !children.empty();
        }
        bool hasAuxinSrcs(){
            return !auxinSrcs.empty();
        }
        void addNewAuxinSrc(float aux_x, float aux_y);
        void placeNewChildNode(float D);
};

float euclidDistance(float x1, float y1, float x2, float y2);
float euclidDistance(VeinNode* node, float aux_x, float aux_y);
void unitVector(float& x, float& y);
VeinNode* findNearestNode(VeinNode* root, float& aux_x, float& aux_y);
void flattenTree(VeinNode* root, std::vector<float>& nodePos);
void placeNewNodes(VeinNode* root, float newNodeDist);
bool relativeNeighbourCheck(VeinNode* root, float& vein_x, float& vein_y, float& aux_x, float& aux_y);

#endif