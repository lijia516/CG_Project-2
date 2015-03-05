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


void Scene::buildKdTree(){
    
    kdtree = new KdTree();
    KdNode* root = kdtree->root;
    root->objects = objects;
    
    typedef vector<Geometry*>::const_iterator iter;
    for(iter j = objects.begin(); j != objects.end(); ++j) {
        root->node_bounds.merge((*j)->getBoundingBox());
    }
    
    kdtree->splitTree(root, 0);
    return;
}



void KdTree::splitTree(KdNode *node, int depth){
    
    if(node->objects.size() <= TraceUI::m_nKdLeaves || depth == TraceUI::m_nKdDepth)
        return;
    
    typedef vector<Geometry*>::const_iterator iter;
    
    node->axis = node->node_bounds.longestAxis();
    
    double mid = 0;
    
    typedef vector<Geometry*>::const_iterator iter;
    
    for(iter j = node->objects.begin(); j != node->objects.end(); ++j) {
        
        mid += (((*j)->getBoundingBox().getMax()[node->axis] - (*j)->getBoundingBox().getMin()[node->axis])/2 + (*j)->getBoundingBox().getMin()[node->axis])*1.0 / node->objects.size();
        
    }
    

    node->mid = mid;
    
    node->left = new KdNode();
    node->right = new KdNode();
    
    for(iter j = node->objects.begin(); j != node->objects.end(); ++j) {
        if((*j)->getBoundingBox().getMax()[node->axis] >= node->mid){
            node->right->objects.push_back(*j);
            node->right->node_bounds.merge((*j)->getBoundingBox());
        }else if((*j)->getBoundingBox().getMin()[node->axis] <= node->mid ){
            node->left->objects.push_back(*j);
            node->left->node_bounds.merge((*j)->getBoundingBox());
        }
    }
    
    
    int match = 0;
    
    for (int i = 0; i < node->left->objects.size(); i++) {
        for (int j = 0; j < node->right->objects.size(); j++) {
            if (node->left->objects[i] == node->right->objects[j]) {
                match++;
            }
        }
    }
    
    
    if (((match*1.0 / node->left->objects.size()) < 0.5) || ((match * 1.0 / node->right->objects.size()) < 0.5)){
    
        splitTree(node->right,  depth + 1);
        splitTree(node->left, depth + 1);
        return;
    }
}


void KdTree::searchTree(KdNode *node, ray &r, std::vector<Geometry*> &result){
    
    
    double tMax;
    double tMin;
    
    if (node->node_bounds.intersect(r, tMin, tMax)) {
    
        typedef vector<Geometry*>::const_iterator iter;
        if(node->right == NULL && node->left == NULL){
            for(iter j = node->objects.begin(); j != node->objects.end(); ++j)
                result.push_back(*j);
            return;
        }
    
        if(node->right->objects.size() != 0 && node->right->node_bounds.intersect(r,tMin, tMax)){
            searchTree(node->right, r, result);
        }
    
        if(node->right->objects.size() != 0 && node->left->node_bounds.intersect(r, tMin, tMax)){
            searchTree(node->left, r, result);
        }
    }
    
}


