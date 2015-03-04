#include <cmath>

#include "scene.h"
#include "light.h"
#include "../ui/TraceUI.h"

using namespace std;

bool Geometry::intersect(ray& r, isect& i) const {
	double tmin, tmax;
	if (hasBoundingBoxCapability() && !(bounds.intersect(r, tmin, tmax))) return false;
	// Transform the ray into the object's local coordinate space
	Vec3d pos = transform->globalToLocalCoords(r.p);
	Vec3d dir = transform->globalToLocalCoords(r.p + r.d) - pos;
	double length = dir.length();
	dir /= length;
	Vec3d Wpos = r.p;
	Vec3d Wdir = r.d;
	r.p = pos;
	r.d = dir;
	bool rtrn = false;
	if (intersectLocal(r, i))
	{
		// Transform the intersection point & normal returned back into global space.
		i.N = transform->localToGlobalCoordsNormal(i.N);
		i.t /= length;
		rtrn = true;
	}
	r.p = Wpos;
	r.d = Wdir;
	return rtrn;
}

bool Geometry::hasBoundingBoxCapability() const {
	// by default, primitives do not have to specify a bounding box.
	// If this method returns true for a primitive, then either the ComputeBoundingBox() or
    // the ComputeLocalBoundingBox() method must be implemented.

	// If no bounding box capability is supported for an object, that object will
	// be checked against every single ray drawn.  This should be avoided whenever possible,
	// but this possibility exists so that new primitives will not have to have bounding
	// boxes implemented for them.
	return false;
}

Scene::~Scene() {
    giter g;
    liter l;
    tmap::iterator t;
    for( g = objects.begin(); g != objects.end(); ++g ) delete (*g);
    for( l = lights.begin(); l != lights.end(); ++l ) delete (*l);
    for( t = textureCache.begin(); t != textureCache.end(); t++ ) delete (*t).second;
}

// Get any intersection with an object.  Return information about the 
// intersection through the reference parameter.
bool Scene::intersect(ray& r, isect& i) const {
    
    
    
    
	double tmin = 0.0;
	double tmax = 0.0;
	bool have_one = false;
    
    //kdTree!!
    std::vector<Geometry*> result;
    kdtree->searchTree(kdtree->root, r, result);
    
	typedef vector<Geometry*>::const_iterator iter;
	for(iter j = result.begin(); j != result.end(); ++j) {
		isect cur;
		if( (*j)->intersect(r, cur) ) {
			if(!have_one || (cur.t < i.t)) {
				i = cur;
				have_one = true;
			}
		}
	}
	if(!have_one) i.setT(1000.0);
	// if debugging,
	if (TraceUI::m_debug) intersectCache.push_back(std::make_pair(new ray(r), new isect(i)));
	return have_one;
}



TextureMap* Scene::getTexture(string name) {
	tmap::const_iterator itr = textureCache.find(name);
	if(itr == textureCache.end()) {
		textureCache[name] = new TextureMap(name);
		return textureCache[name];
	} else return (*itr).second;
}



KDNode::KDNode(){
}

bool KdTree::compare0(Geometry *a, Geometry *b){
    return a->getBoundingBox().getMin()[0] < b->getBoundingBox().getMin()[0];
}
bool KdTree::compare1(Geometry *a, Geometry *b){
    return a->getBoundingBox().getMin()[1] < b->getBoundingBox().getMin()[1];
}
bool KdTree::compare2(Geometry *a, Geometry *b){
    return a->getBoundingBox().getMin()[2] < b->getBoundingBox().getMin()[2];
}


void Scene::buildKdTree(){
    //printf("Building Mesh KD tree...\n");
    
    kdtree = new KdTree();
    KDNode* root = kdtree->root;
    root->objects = objects;
    
    typedef vector<Geometry*>::const_iterator iter;
    for(iter j = objects.begin(); j != objects.end(); ++j) {
        //printf("TMB %f\n",(*j)->getBoundingBox().getMin()[0]);
        root->node_bounds.merge((*j)->getBoundingBox());
    }
    
    kdtree->buildTree(root, 0);
    //printf("Done!\n");
    return;
}



void KdTree::buildTree(KDNode *node, int depth){
 //   printf("Scene KD depth- size:%d - %d\n", depth, node->objects.size());
    if(node->objects.size() <= 1 || depth == 10)
        return;
    
    typedef vector<Geometry*>::const_iterator iter;
    
    double len;
    
    node->axis = node->node_bounds.longestAxis();
    
    typedef vector<Geometry*>::const_iterator iter;
    
    switch(node->axis){
        case 0:
            std::sort(node->objects.begin(), node->objects.end(), compare0);
            break;
        case 1:
            std::sort(node->objects.begin(), node->objects.end(), compare1);
            break;
        case 2:
            std::sort(node->objects.begin(), node->objects.end(), compare2);
            break;
    }
    
    node->mid = node->objects[node->objects.size()/2]->getBoundingBox().getMin()[node->axis];
    
    //find the cutting plane
    /*
		   double mid = 0;
		   for(iter j = faces.begin(); j != faces.end(); ++j) {
		   mid += (*j)->getBoundingBox().getMin()[dimension];
		   }
     
		   mid /= faces.size();
     */
    node->left = new KDNode();
    node->right = new KDNode();
    
    for(iter j = node->objects.begin(); j != node->objects.end(); ++j) {
        if((*j)->getBoundingBox().getMax()[node->axis] >= node->mid){//right
            node->right->objects.push_back(*j);
            node->right->node_bounds.merge((*j)->getBoundingBox());
        }else if((*j)->getBoundingBox().getMin()[node->axis] <= node->mid ){//left
            node->left->objects.push_back(*j);
            node->left->node_bounds.merge((*j)->getBoundingBox());
        }else{
            printf("Error: KdTree split??\n");
        }
    }
    
    
    //printf("right vs left: %d vs %d\n", node->right->faces.size(), node->left->faces.size());
    
    buildTree(node->right,  depth + 1);
    buildTree(node->left, depth + 1);
    return;
}

//dfs
void KdTree::searchTree(KDNode *node, ray &r, std::vector<Geometry*> &result){
    
    
    typedef vector<Geometry*>::const_iterator iter;
    if(node->right == NULL && node->left == NULL){//leaf
        for(iter j = node->objects.begin(); j != node->objects.end(); ++j)
            result.push_back(*j);
        return;
    }
    
    double tTMP = 100;
    if(node->right != NULL && node->right->objects.size() != 0 && node->right->node_bounds.intersect(r, tTMP, tTMP)){
        searchTree(node->right, r, result);
    }
    
    if(node->left != NULL && node->right->objects.size() != 0 && node->left->node_bounds.intersect(r, tTMP, tTMP)){
        searchTree(node->left, r, result);
    }
    
    
    
    //  double tMax = 100;
    //  double tMin = 0;
    //  if (node->node_bounds.inters(r, tMin, tMax)) {
    
    /*
     
     if (node->mid > tMax) searchTree(node->left, r, result);
     else if (node->mid < tMin) searchTree(node->right, r, result);
     else {
     searchTree(node->left, r, result);
     searchTree(node->right, r, result);
     }
     }
     
     */
    
}


