#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__

// The main ray tracer.

#include "scene/ray.h"
#include "scene/cubeMap.h"
#include <time.h>
#include <queue>

class Scene;

class RayTracer
{
public:
	RayTracer();
        ~RayTracer();

	Vec3d tracePixel(int i, int j);
	Vec3d trace(double x, double y);
	Vec3d traceRay(ray& r, int depth);

	void getBuffer(unsigned char *&buf, int &w, int &h);
	double aspectRatio();

	void traceSetup( int w, int h );
    void antiAliased(int sampling, int i, int j);

	bool loadScene(char* fn);
	bool sceneLoaded() { return scene != 0; }
    
    bool loadBackgroundImage(char* fn);

	void setReady(bool ready) { m_bBufferReady = ready; }
	bool isReady() const { return m_bBufferReady; }

	const Scene& getScene() { return *scene; }

        void setCubeMap(CubeMap* m) {
            if (cubemap) delete cubemap;
            cubemap = m;
        }
        CubeMap *getCubeMap() {return cubemap;}
        bool haveCubeMap() { return cubemap != 0; }
    
    
    Vec3d adaptive_antiAliased ( double x, double  y , double  x_gap, double  y_gap, int dir, Vec3d col_ld, Vec3d temp_col_rd, Vec3d temp_col_lu, Vec3d temp_col_ru);
    
    static int aa_depth_total;
    Vec3d traceRay_DOF(ray& r, double x, double y);
    

public:
        unsigned char *buffer;
        int buffer_width, buffer_height;
        int bufferSize;
        Scene* scene;
        CubeMap* cubemap;

        bool m_bBufferReady;
        bool m_useCubeMap;
    
    static unsigned char *backgroundImage;
    static int backgroundImage_width, backgroundImage_height;
    static bool hasBackgroundImage;
    
};

#endif // __RAYTRACER_H__
