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
int RayTracer::aa_depth_total = 0;

// Trace a top-level ray through pixel(i,j), i.e. normalized window coordinates (x,y),
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.

Vec3d RayTracer::trace(double x, double y)
{
  // Clear out the ray cache in the scene for debugging purposes,
  if (TraceUI::m_debug) scene->intersectCache.clear();
  ray r(Vec3d(0,0,0), Vec3d(0,0,0), ray::VISIBILITY);
  
    
     Vec3d ret(0,0,0);
    
    if (TraceUI::m_glossyRefection) {
        
       
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
        
       
    } else if (TraceUI::m_DOF) {
        
         scene->getCamera().rayThrough(x,y,r);
        
         ret = traceRay_DOF(r, x, y);
        
        
    } else {
        
        scene->getCamera().rayThrough(x,y,r);
        ret = traceRay(r, traceUI->getDepth());
       
    }
    
    
  ret.clamp();
  
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


Vec3d RayTracer::traceRay_DOF(ray& r, double x, double y) {
    
    isect i;
    Vec3d ret(0,0,0);
    
   // cout << "in DOF\n";
    
    if (scene->intersect(r,i)) {
        
        Vec3d eye = scene->getCamera().getEye();
        Vec3d pos = r.at(i.t);
        
        double len = (eye - pos).length();
        
        if ( len > TraceUI::m_nDOF) {
            
            double diff = (len - TraceUI::m_nDOF)/len;
            
            for (int i = 0; i < 16; i++) {
                
                double dx = (static_cast<double>(rand() % 10) / 500) - 0.01f;
                double dy = (static_cast<double>(rand() % 10) / 500) - 0.01f;
                
                dx = dx * diff;
                dy = dy * diff;
                
                r.p += Vec3d(dx, dy, 0);
                r.d -= Vec3d(dx, dy, 0);
                
                Vec3d color = traceRay(r, traceUI->getDepth());
                
                color.clamp();
                
                ret += color ;
            }
            
            
            ret = ret / 16;
            
        } else {
          
              ret = traceRay(r, traceUI->getDepth());
            
        }

    } else {
        
       //   cout << "do not have intersect\n";
        
        if (!TraceUI::m_cubeMap) ret = traceRay(r, traceUI->getDepth());
        
        else {
            for (int i = 0; i < 16; i++) {
                
                double dx = (static_cast<double>(rand() % 10) / 500) - 0.01f;
                double dy = (static_cast<double>(rand() % 10) / 500) - 0.01f;
     
                
                r.p += Vec3d(dx, dy, 0);
                r.d -= Vec3d(dx, dy, 0);
                
                Vec3d color = traceRay(r, traceUI->getDepth());
                
                color.clamp();
                
                ret += color ;
            }
            
            ret = ret / 16;
        
        }
        
    }
    
    ret.clamp();
    return ret;
    
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
        
        
        
        
        if (TraceUI::m_glossyRefection) {
            
            
            double dx = (static_cast<double>(rand() % 10) / 500) - 0.001f;
            double dy = (static_cast<double>(rand() % 10) / 500) - 0.001f;
            double dz = (static_cast<double>(rand() % 10) / 500) - 0.001f;
            
            i.N += Vec3d(dx, dy, dz);
            
        }
        

      
        const Material& m = i.getMaterial();
        colorC = m.shade(scene, r, i);
        
        if (depth < 1) {
            
            return colorC;
        }
        
        
        
        if (colorC[0] < TraceUI::m_nThreshold && colorC[1] < TraceUI::m_nThreshold && colorC[2] < TraceUI::m_nThreshold) {
            
            return colorC;
        }
        
        //////reflection
        Vec3d N = i.N;
        Vec3d V = - r.getDirection();
        Vec3d NV = N % V;
        double NVv = NV[0] + NV[1] + NV[2];
        Vec3d R = i.N * 2* NVv - V;
        
        
       
        
        
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
        
        
        if (TraceUI::m_cubeMap) colorC = cubemap->getColor(r);
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
        
    } else if (TraceUI::m_adaptiveAntiliasing) {
        
        
        Vec3d temp_col_ld (0, 0, 0);
        Vec3d temp_col_rd (0, 0, 0);
        Vec3d temp_col_lu (0, 0, 0);
        Vec3d temp_col_ru (0, 0, 0);
        
        color += adaptive_antiAliased ( x, y , x_gap / 2, y_gap / 2, -1, temp_col_ld, temp_col_rd, temp_col_lu, temp_col_ru);
        color = color / (aa_depth_total+ 1);

        
        aa_depth_total = 0;
    
    
    } else {
        
        
        int size = sqrt(sampling);
        int center_r = size / 2;
        int center_c = size / 2;
        
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                
                color += trace(x + ((i + (-size / 2)) * 1.0 /size * x_gap), y + ((j + (-size / 2)) * 1.0 /size * y_gap));
            }
        }
        
        color /= (size * size);
        
    }
    
    pixel[0] = (int)( 255.0 * color[0]);
    pixel[1] = (int)( 255.0 * color[1]);
    pixel[2] = (int)( 255.0 * color[2]);
}


Vec3d RayTracer::adaptive_antiAliased ( double x, double y , double x_gap, double y_gap, int dir , Vec3d temp_col_ld, Vec3d temp_col_rd, Vec3d temp_col_lu, Vec3d temp_col_ru) {
    
    
 //   std::cout <<"in recursion fun: aa_depth_total"<<aa_depth_total<<"\n";
    
 //   std::cout <<"x,y"<<x<<","<<y<<"\n";
    
    double center_r = y;
    double center_c = x;
    double max_diff = -1;
    
    Vec3d col = trace(x, y);
    
    if (dir == -1) {
    
        temp_col_ld = trace(x - x_gap, y - y_gap);
        temp_col_rd = trace(x + x_gap, y - y_gap);
        temp_col_lu = trace(x - x_gap, y + y_gap);
        temp_col_ru = trace(x + x_gap, y + y_gap);
        
    }
    
    
    max_diff = (abs(col[0] - temp_col_ld[0]) + abs(col[1] - temp_col_ld[1]) + abs(col[2] - temp_col_ld[2]));
    
    double max_temp1 = (abs(col[0] - temp_col_rd[0]) + abs(col[1] - temp_col_rd[1]) + abs(col[2] - temp_col_rd[2]));
    double max_temp2 = (abs(col[0] - temp_col_lu[0]) + abs(col[1] - temp_col_lu[1]) + abs(col[2] - temp_col_lu[2]));
    double max_temp3 = (abs(col[0] - temp_col_ru[0]) + abs(col[1] - temp_col_ru[1]) + abs(col[2] - temp_col_ru[2]));
    
    double half_x_gap = x_gap / 2;
    double half_y_gap = x_gap / 2;
    
    double new_max_diff = fmax(max_diff, fmax(max_temp1, fmax(max_temp2, max_temp3)));
    
    if (new_max_diff > TraceUI::m_nAaThresh && half_x_gap > 0.0000001 && half_y_gap > 0.0000001) {
        
        
        Vec3d temp_col_ll = trace(x, y - y_gap);
        Vec3d temp_col_rr = trace(x, y + y_gap);
        Vec3d temp_col_uu = trace(x + x_gap, y);
        Vec3d temp_col_dd = trace(x - x_gap, y);

        
        if (max_diff > TraceUI::m_nAaThresh) {
            
                center_c = x - half_x_gap;
                center_r = y - half_y_gap;
                
                col += adaptive_antiAliased ( center_c, center_r , half_x_gap, half_y_gap, 0, temp_col_ld, temp_col_dd, temp_col_ll, col);
                
        }
        
        if (max_temp2 > TraceUI::m_nAaThresh) {
    
                
                center_c = x + half_x_gap;
                center_r = y - half_y_gap;
                
                col += adaptive_antiAliased ( center_c, center_r , half_x_gap, half_y_gap, 1, temp_col_dd, temp_col_rd, col, temp_col_rr);
         
        }
        
        if (max_temp2 > TraceUI::m_nAaThresh) {
            
                center_c = x - half_x_gap;
                center_r = y + half_y_gap;
                col += adaptive_antiAliased ( center_c, center_r , half_x_gap, half_y_gap, 2, temp_col_ll, col, temp_col_lu, temp_col_uu);
         
        }
        
        if (max_temp3 > TraceUI::m_nAaThresh) {
    
                
                center_c = x + half_x_gap;
                center_r = y + half_y_gap;
                col += adaptive_antiAliased ( center_c, center_r , half_x_gap, half_y_gap, 3, col,temp_col_rr, temp_col_uu, temp_col_ru);
        }
        
        
        if (dir == -1) {
            
            col += temp_col_ld;
            col += temp_col_rd;
            col += temp_col_lu;
            col += temp_col_ru;
            
            col += temp_col_ll;
            col += temp_col_rr;
            col += temp_col_uu;
            col += temp_col_dd;
            
            aa_depth_total += 9;
            
        } else {
            
            col += temp_col_ll;
            col += temp_col_rr;
            col += temp_col_uu;
            col += temp_col_dd;
            
            aa_depth_total += 5;
        }
    
    }
    
    return col;
}


