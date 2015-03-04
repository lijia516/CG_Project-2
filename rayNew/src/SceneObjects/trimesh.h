#ifndef TRIMESH_H__
#define TRIMESH_H__

#include <list>
#include <vector>

#include "../scene/ray.h"
#include "../scene/material.h"
#include "../scene/scene.h"

class TrimeshFace;
class KDNodeTM{
		public:
		KDNodeTM();
			BoundingBox node_bounds;
			KDNodeTM * left = NULL;
			KDNodeTM * right = NULL;
			std::vector<TrimeshFace*> faces;
            int axis;
            double mid;
};
class KdTreeTM{
		public:
        KdTreeTM() { root = new KDNodeTM();}
		KDNodeTM *root;
		static void buildTree(KDNodeTM *node, int depth);
		static void searchTree(KDNodeTM *node, ray &r, std::vector<TrimeshFace*> &result);
		static bool compare0(TrimeshFace *a, TrimeshFace* b);
		static bool compare1(TrimeshFace *a, TrimeshFace* b);
		static bool compare2(TrimeshFace *a, TrimeshFace* b);
        static double getCost(KDNodeTM *node, double pos);
};


class Trimesh : public MaterialSceneObject
{
		friend class TrimeshFace;
		typedef std::vector<Vec3d> Normals;
		typedef std::vector<Vec3d> Vertices;
		typedef std::vector<TrimeshFace*> Faces;
		typedef std::vector<Material*> Materials;

		Normals normals;
		Materials materials;

		public:
		//kd
		KdTreeTM *kdtree = new KdTreeTM();
		void buildKdTree();

		BoundingBox localBounds;
		Faces faces;
		Vertices vertices;

		Trimesh( Scene *scene, Material *mat, TransformNode *transform )
				: MaterialSceneObject(scene, mat), 
				displayListWithMaterials(0),
				displayListWithoutMaterials(0)
		{
				this->transform = transform;
				vertNorms = false;
		}

		bool vertNorms;

		bool intersectLocal(ray& r, isect& i) const;

		~Trimesh();

		// must add vertices, normals, and materials IN ORDER
		void addVertex( const Vec3d & );
		void addMaterial( Material *m );
		void addNormal( const Vec3d & );
		bool addFace( int a, int b, int c );

		char *doubleCheck();

		void generateNormals();

		bool hasBoundingBoxCapability() const { return true; }

		BoundingBox ComputeLocalBoundingBox()
		{
				BoundingBox localbounds;
				if (vertices.size() == 0) return localbounds;
				localbounds.setMax(vertices[0]);
				localbounds.setMin(vertices[0]);
				Vertices::const_iterator viter;
				for (viter = vertices.begin(); viter != vertices.end(); ++viter)
				{
						localbounds.setMax(maximum( localbounds.getMax(), *viter));
						localbounds.setMin(minimum( localbounds.getMin(), *viter));
				}
				localBounds = localbounds;
				return localbounds;
		}

		protected:
		void glDrawLocal(int quality, bool actualMaterials, bool actualTextures) const;
		mutable int displayListWithMaterials;
		mutable int displayListWithoutMaterials;
};

class TrimeshFace : public MaterialSceneObject
{
		Trimesh *parent;
		int ids[3];
		Vec3d normal;
		double dist;

		public:

		TrimeshFace( Scene *scene, Material *mat, Trimesh *parent, int a, int b, int c)
				: MaterialSceneObject( scene, mat )
		{
				this->parent = parent;
				ids[0] = a;
				ids[1] = b;
				ids[2] = c;

				// Compute the face normal here, not on the fly
				Vec3d a_coords = parent->vertices[a];
				Vec3d b_coords = parent->vertices[b];
				Vec3d c_coords = parent->vertices[c];

				Vec3d vab = (b_coords - a_coords);
				Vec3d vac = (c_coords - a_coords);
				Vec3d vcb = (b_coords - c_coords);

				if (vab.iszero() || vac.iszero() || vcb.iszero()) degen = true;
				else {
						degen = false;
						normal = ((b_coords - a_coords) ^ (c_coords - a_coords));
						normal.normalize();
						dist = normal * a_coords;
				}
				localbounds = ComputeLocalBoundingBox();
				bounds = localbounds;
		}

		BoundingBox localbounds;
		bool degen;

		int operator[]( int i ) const
		{
				return ids[i];
		}

		Vec3d getNormal()
		{
				return normal;
		}

		bool intersect(ray& r, isect& i ) const;
		bool intersectLocal(ray& r, isect& i ) const;

		bool hasBoundingBoxCapability() const { return true; }

		BoundingBox ComputeLocalBoundingBox()
		{
				BoundingBox localbounds;
				localbounds.setMax(maximum( parent->vertices[ids[0]], parent->vertices[ids[1]]));
				localbounds.setMin(minimum( parent->vertices[ids[0]], parent->vertices[ids[1]]));

				localbounds.setMax(maximum( parent->vertices[ids[2]], localbounds.getMax()));
				localbounds.setMin(minimum( parent->vertices[ids[2]], localbounds.getMin()));
				return localbounds;
		}

		const BoundingBox& getBoundingBox() const { return localbounds; }

};

#endif // TRIMESH_H__
