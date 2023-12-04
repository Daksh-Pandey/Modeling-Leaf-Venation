#include "veinnode.h"

float euclidDistance(VeinNode* node, float aux_x, float aux_y){
    return euclidDistance(node->getX(), node->getY(), aux_x, aux_y);
}

void unitVector(float& x, float& y){
    float mag = pow(x*x + y*y, 0.5);
    x /= mag;
    y /= mag;
}

void VeinNode::addNewAuxinSrc(float aux_x, float aux_y){
    this->auxinSrcs.push_back({aux_x, aux_y});
}

void VeinNode::placeNewChildNode(float D){
    float sum_x = 0.0f, sum_y = 0.0f;
    for (auto i : auxinSrcs){
        float aux_x = i.first - this->x;
        float aux_y = i.second - this->y;
        unitVector(aux_x, aux_y);
        sum_x += aux_x;
        sum_y += aux_y;
    }
    unitVector(sum_x, sum_y);
    VeinNode* newChild = new VeinNode(x + D * sum_x, y + D * sum_y);
    this->children.push_back(newChild);
}

VeinNode* findNearestNode(VeinNode* root, float& aux_x, float& aux_y){
    if (!root) return nullptr;
    if (!root->hasChildren()){
        return root;
    }
    VeinNode* nearestNode = root;
    float near_dist = euclidDistance(root, aux_x, aux_y);
    for (VeinNode* nbr : root->getChildren()){
        VeinNode* tmp = findNearestNode(nbr, aux_x, aux_y);
        float tmp_dist = euclidDistance(tmp, aux_x, aux_y);
        if (tmp_dist < near_dist){
            nearestNode = tmp;
            near_dist = tmp_dist;
        }
    }
    return nearestNode;
}

void flattenTree(VeinNode* root, std::vector<float>& nodePos){
    if (!root) return;
    if (!root->hasChildren()){
        return;
    }
    for (VeinNode* nbr : root->getChildren()){
        nodePos.push_back(root->getX());
        nodePos.push_back(root->getY());
        nodePos.push_back(0.0f);
        nodePos.push_back(nbr->getX());
        nodePos.push_back(nbr->getY());
        nodePos.push_back(0.0f);
        flattenTree(nbr, nodePos);
    }
}

void placeNewNodes(VeinNode* root, float newNodeDist){
    if (!root) return;
    if (!root->hasAuxinSrcs() && !root->hasChildren()){
        return;
    }
    if (root->hasAuxinSrcs()){
        root->placeNewChildNode(newNodeDist);
        root->getAuxinSrcs().clear();
    }
    if (!root->hasChildren()){
        return;
    }
    for (VeinNode* nbr : root->getChildren()){
        placeNewNodes(nbr, newNodeDist);
    }
}