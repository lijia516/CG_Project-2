// The main ray tracer.

#pragma warning (disable: 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"

#include "parser/Tokenizer.h"
#include "parser/Parser.h"

#include "ui/TraceUI.h"
#include <cmath>
#include <algorithm>

extern TraceUI* traceUI;

#include <iostream>
#include <fstream>


using namespace std;

// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = false;

// Trace a top-level ray through pixel(i,j), i.e. normalized window coordinates (x,y),
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.

Vec3d RayTracer::trace(double x, double y)
{
  // Clear out the ray cache in the scene for debugging purposes,
  if (TraceUI::m_debug) scene->intersectCache.clear();
  ray r(Vec3d(0,0,0), Vec3d(0,0,0), ray::VISIBILITY);
  
    
  //  scene->getCamera().rayThrough(x,y,r);
  //  Vec3d ret = traceRay(r, traceUI->getDepth());
  //  ret.clamp();
  //  return ret;
    
    
    Vec3d ret(0,0,0);

    
    for (int i = 0; i < 16; i++) {
        
        scene->getCamera().rayThrough(x,y,r);
        
        
        
        double dx = (static_cast<double>(rand() % 10) / 5000) - 0.001f;
        double dy = (static_cast<double>(rand() % 10) / 5000) - 0.001f;
       
        r.p += Vec3d(dx, dy, 0);
        r.d -= Vec3d(dx, dy, 0);
        
        
        Vec3d color = traceRay(r, traceUI->getDepth());
        
        color.clamp();
        
        ret += color ;
    }
    
    
    ret = ret / 16;
  
  return ret;
}

Vec3d RayTracer::tracePixel(int i, int j)
{
	Vec3d col(0,0,0);

	if( ! sceneLoaded() ) return col;
    

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);

	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	col = trace(x, y);

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
	return col;
}


// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
Vec3d RayTracer::traceRay(ray& r, int depth)
{
	isect i;
	Vec3d colorC;

	if(scene->intersect(r, i)) {
		// YOUR CODE HERE

		// An intersection occurred!  We've got work to do.  For now,
		// this code gets the material for the surface that was intersected,
		// and asks that material to provide a color for the ray.  

		// This is a great place to insert code for recursive ray tracing.
		// Instead of just returning the result of shade(), add some
		// more steps: add in the contributions from reflected and refracted
		// rays.
	  //const Material& m = i.getMaterial();
	  //colorC = m.shade(scene, r, i);
        
      
        const Material& m = i.getMaterial();
        colorC = m.shade(scene, r, i);
        
        if (depth < 1) {
            
            return colorC;
        }
        
      //  std::cout <<"colorC"<<colorC[0]<<","<<colorC[1]<<","<<colorC[2]<<"\n";
       // std::cout << TraceUI::m_nThreshold<<"\n";
        
        
        if (colorC[0] < TraceUI::m_nThreshold && colorC[1] < TraceUI::m_nThreshold && colorC[2] < TraceUI::m_nThreshold) {
            
         //   std::cout <<"return\n";
         //   std::cout <<"colorC"<<colorC[0]<<","<<colorC[1]<<","<<colorC[2]<<"\n";
         //   std::cout << TraceUI::m_nThreshold<<"\n";
            
            return colorC;
        }
        
        //////reflection
        Vec3d N = i.N;
        Vec3d V = - r.getDirection();
        Vec3d NV = N % V;
        double NVv = NV[0] + NV[1] + NV[2];
        Vec3d R = i.N * 2* NVv - V;
        
        
        
        double dx = (static_cast<double>(rand() % 10) / 500) - 0.001f;
        double dy = (static_cast<double>(rand() % 10) / 500) - 0.001f;
        double dz = (static_cast<double>(rand() % 10) / 500) - 0.001f;
        
        R += Vec3d(dx, dy, dz);

        
        
        ray new_r = ray(r.at(i.t), R, ray::VISIBILITY);
        Vec3d a = m.kr(i) % traceRay(new_r,depth - 1);
        
        colorC = colorC + a;
 
        double nr = 0;
        
        // ray is entering object
        if (NVv > 0)  nr = 1.0 / m.index(i);
        else if (NVv < 0) nr = m.index(i);
        
        if (m.Trans()) {
            
            if ((1 - nr * nr * (1 - NVv * NVv)) < 0) return colorC;
            
            double cosQ = sqrt(1 - nr * nr * (1 - NVv * NVv));
            Vec3d T;
            
            if (NVv > 0) T = N * (nr * NVv - cosQ) - V * nr;
            else T = N * (-(nr * (-NVv) - cosQ)) - V * nr;
            
            
            new_r = ray(r.at(i.t), T);
            colorC = colorC + m.kt(i) % traceRay(new_r,depth - 1);
        }
        
        
        
        
        
        
	} else {
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.
        
        
        if (m_useCubeMap) colorC = cubemap->getColor(r);
        else colorC = Vec3d(0, 0, 0);
        
	}
	return colorC;
}

RayTracer::RayTracer()
: scene(0), buffer(0), buffer_width(256), buffer_height(256), m_bBufferReady(false), cubemap(0), m_useCubeMap(false)
{}

RayTracer::~RayTracer()
{
	delete scene;
	delete [] buffer;
    delete cubemap;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene( char* fn ) {
	ifstream ifs( fn );
	if( !ifs ) {
		string msg( "Error: couldn't read scene file " );
		msg.append( fn );
		traceUI->alert( msg );
		return false;
	}
	
	// Strip off filename, leaving only the path:
	string path( fn );
	if( path.find_last_of( "\\/" ) == string::npos ) path = ".";
	else path = path.substr(0, path.find_last_of( "\\/" ));

	// Call this with 'true' for debug output from the tokenizer
	Tokenizer tokenizer( ifs, false );
    Parser parser( tokenizer, path );
	try {
		delete scene;
		scene = 0;
		scene = parser.parseScene();
	} 
	catch( SyntaxErrorException& pe ) {
		traceUI->alert( pe.formattedMessage() );
		return false;
	}
	catch( ParserException& pe ) {
		string msg( "Parser: fatal exception " );
		msg.append( pe.message() );
		traceUI->alert( msg );
		return false;
	}
	catch( TextureMapException e ) {
		string msg( "Texture mapping exception: " );
		msg.append( e.message() );
		traceUI->alert( msg );
		return false;
	}

	if( !sceneLoaded() ) return false;
    
    
    scene->buildKdTree();

	return true;
}

void RayTracer::traceSetup(int w, int h)
{
	if (buffer_width != w || buffer_height != h)
	{
		buffer_width = w;
		buffer_height = h;
		bufferSize = buffer_width * buffer_height * 3;
		delete[] buffer;
		buffer = new unsigned char[bufferSize];
	}
	memset(buffer, 0, w*h*3);
	m_bBufferReady = true;
}

void RayTracer::antiAliased(int sampling, int i, int j) {
    
    double x_gap = 1 / double(buffer_width);
    double y_gap = 1 / double(buffer_height);
    
    double x = double(i)/double(buffer_width);
    double y = double(j)/double(buffer_height);
    
    unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;
    
    
    Vec3d color(0,0,0);
    
    
    if (TraceUI::m_hasJitteredSupersample) {
        
        color = trace( x,y );
        
        for (int i = 1; i < sampling; i++) {
            color += trace( x + (rand() % (sampling / 2 + 1) * 1.0 / (sampling * 1.0 / 2) * pow(-1, rand() % 2) ) * x_gap, y + (rand() % (sampling / 2 + 1) * 1.0 / (sampling * 1.0 / 2) * pow(-1, rand() % 2)) * y_gap);
        }
        
        color /= sampling;
        
    } else {
        
        
        int size = sqrt(sampling);
        int center_r = size / 2;
        int center_c = size / 2;
        
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                
                color += trace(x + ((i + (-size / 2)) * 1.0 /size * x_gap), y + ((j + (-size / 2)) * 1.0 /size * y_gap));
               // std::cout <<"color"<<color[0]<<","<<color[1]<<","<<color[2]<<"\n";
            }
        }
        
        color /= (size * size);
        
    }
    

    
    
    pixel[0] = (int)( 255.0 * color[0]);
    pixel[1] = (int)( 255.0 * color[1]);
    pixel[2] = (int)( 255.0 * color[2]);
}

