#include "material.h"
#include "ray.h"
#include "light.h"
#include "../ui/TraceUI.h"
#include "../RayTracer.h"
extern TraceUI* traceUI;

#include "../fileio/bitmap.h"
#include "../fileio/pngimage.h"

using namespace std;
extern bool debugMode;

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
Vec3d Material::shade(Scene *scene, const ray& r, const isect& i) const
{
  // YOUR CODE HERE

  // For now, this method just returns the diffuse color of the object.
  // This gives a single matte color for every distinct surface in the
  // scene, and that's it.  Simple, but enough to get you started.
  // (It's also inconsistent with the phong model...)

  // Your mission is to fill in this method with the rest of the phong
  // shading model, including the contributions of all the light sources.

  // When you're iterating through the lights,
  // you'll want to use code that looks something
  // like this:
  //
  // for ( vector<Light*>::const_iterator litr = scene->beginLights(); 
  // 		litr != scene->endLights(); 
  // 		++litr )
  // {
  // 		Light* pLight = *litr;
  // 		.
  // 		.
  // 		.
  // }

  // You will need to call both the distanceAttenuation() and
  // shadowAttenuation() methods for each light source in order to
  // compute shadows and light falloff.
    
    Vec3d q = r.at(i.t);
    Vec3d I = ke(i);
    
    
    if (Trans()) {
        
        I = I + ka(i) % (Vec3d(1.0, 1.0, 1.0) - kt(i)) % scene->ambient();
    } else {
        I = I + ka(i) % scene->ambient();
    }
    
    for ( vector<Light*>::const_iterator litr = scene->beginLights(); litr != scene->endLights(); ++litr ) {
        
        Light* pLight = *litr;
        Vec3d atten = pLight->shadowAttenuation(r, q) * pLight->distanceAttenuation(q);
        
        
        //diffuse
        
        
          Vec3d N = i.N;
        
        if (TraceUI::m_bbm) {
             N = getDisNormal(i.uvCoordinates);
            if (N[0] == 0 && N[1] == 0 && N[2] == 0) N = i.N;
        }
        
      
        
        
       // Vec3d N = getDisNormal(i.uvCoordinates);
        
        
       // if (N[0] == 0 && N[1] == 0 && N[2] == 0) N = i.N;
        
        
        
        
     //   N = i.N + i.N % N;
        
        Vec3d L = pLight->getDirection(q);
        
        double NLv = N * L;
        Vec3d diffuseTerm = kd(i) * fmax(0, NLv);
        
        //specular reflection
        Vec3d R = N * 2* NLv - L;
        Vec3d V = - r.getDirection();
 
        Vec3d specularTerm = ks(i) * pow (fmax(0, R * V), shininess(i));
        I = I + (diffuseTerm + specularTerm) % atten % pLight->getColor();
    }
    
    return I;
}

TextureMap::TextureMap( string filename ) {

	int start = (int) filename.find_last_of('.');
	int end = (int) filename.size() - 1;
	if (start >= 0 && start < end) {
		string ext = filename.substr(start, end);
		if (!ext.compare(".png")) {
			png_cleanup(1);
			if (!png_init(filename.c_str(), width, height)) {
				double gamma = 2.2;
				int channels, rowBytes;
				unsigned char* indata = png_get_image(gamma, channels, rowBytes);
				int bufsize = rowBytes * height;
				data = new unsigned char[bufsize];
				for (int j = 0; j < height; j++)
					for (int i = 0; i < rowBytes; i += channels)
						for (int k = 0; k < channels; k++)
							*(data + k + i + j * rowBytes) = *(indata + k + i + (height - j - 1) * rowBytes);
				png_cleanup(1);
			}
		}
		else
			if (!ext.compare(".bmp")) data = readBMP(filename.c_str(), width, height);
			else data = NULL;
	} else data = NULL;
	if (data == NULL) {
		width = 0;
		height = 0;
		string error("Unable to load texture map '");
		error.append(filename);
		error.append("'.");
		throw TextureMapException(error);
	}
}

Vec3d TextureMap::getMappedValue( const Vec2d& coord ) const
{
  // YOUR CODE HERE

  // In order to add texture mapping support to the 
  // raytracer, you need to implement this function.
  // What this function should do is convert from
  // parametric space which is the unit square
  // [0, 1] x [0, 1] in 2-space to bitmap coordinates,
  // and use these to perform bilinear interpolation
  // of the values.

 // return Vec3d(1,1,1);
    
  /*
    
    std::cout<<"text mapping\n";
    
    Vec3d ij = getPixelAt((int) (width * coord[0]), (int) (height * coord[1]));
    Vec3d i1j = getPixelAt((int) (width * (coord[0] + 1)), (int) (height * coord[1]));
    Vec3d ij1 = getPixelAt((int) (width * coord[0]), (int) (height * (coord[1] + 1)));
    Vec3d i1j1 = getPixelAt((int) (width * (coord[0] + 1)), (int) (height * (coord[1] + 1)));

    int i = floor(coord[0]);
    int j = floor(coord[1]);
    
    double delta_x = coord[0] - i;
    double delta_y = coord[1] - j;
    
    Vec3d value;

    value[0] =  (1 - delta_x) * (1 - delta_y) * ij[0] + delta_x * (1 - delta_y) * i1j[0] + (1-delta_x) * delta_y * ij1[0] + delta_x * delta_y * i1j1[0];
    value[1] = (1 - delta_x) * (1 - delta_y) * ij[1] + delta_x * (1 - delta_y) * i1j[1] + (1-delta_x) * delta_y * ij1[1] + delta_x * delta_y * i1j1[1];
    value[2] = (1 - delta_x) * (1 - delta_y) * ij[2] + delta_x * (1 - delta_y) * i1j[2] + (1-delta_x) * delta_y * ij1[2] + delta_x * delta_y * i1j1[2];
    
    
    value[0] = double(value[0]);
    value[1] = double(value[1]);
    value[2] = double(value[2]);

    
    return value; //getPixelAt((int) (width * coord[0]), (int) (height * coord[1])); */
    
    if (TraceUI::m_aat) {
    
        double coord_x = fmax(coord[0] * (getWidth() - 1), 0);
        double coord_y = fmax(coord[1] * (getHeight() - 1), 0);
        
        int i = floor(coord_x);
        int j = floor(coord_y);
        
        double delta_x = coord_x - i;
        double delta_y = coord_y - j;
        
        
        Vec3d value;
        
        unsigned char *pixel_ij = data + (i + j * getWidth()) *3;
        unsigned char *pixel_i1j = data + ((i < (getWidth() - 1) ? i + 1 : i) + j * getWidth()) *3;
        unsigned char *pixel_ij1 = data + (i + (j < (getHeight() - 1) ? j + 1 : j) * getWidth()) *3;
        unsigned char *pixel_i1j1 = data + ((i < (getWidth() - 1) ? i + 1 : i) + (j < (getHeight() - 1) ? j + 1 : j) * getWidth()) *3;
        
        value[0] =  (1 - delta_x) * (1 - delta_y) * pixel_ij[0] + delta_x * (1 - delta_y) * pixel_i1j[0] + (1-delta_x) * delta_y * pixel_ij1[0] + delta_x * delta_y * pixel_i1j1[0];
        value[1] = (1 - delta_x) * (1 - delta_y) * pixel_ij[1] + delta_x * (1 - delta_y) * pixel_i1j[1] + (1-delta_x) * delta_y * pixel_ij1[1] + delta_x * delta_y * pixel_i1j1[1];
        value[2] = (1 - delta_x) * (1 - delta_y) * pixel_ij[2] + delta_x * (1 - delta_y) * pixel_i1j[2] + (1-delta_x) * delta_y * pixel_ij1[2] + delta_x * delta_y * pixel_i1j1[2];
        
        
        value[0] = double(value[0]) / 255.0;
        value[1] = double(value[1]) / 255.0;
        value[2] = double(value[2]) / 255.0;
        
     //   std::cout<< "value: " << value[0]<< ", " << value[1]<< ", "<< value[2]<< ", "<<"\n";
        
        return value;
        
    } else {
        
        return getPixelAt((int) (width * coord[0]), (int) (height * coord[1]));
    }

}


Vec3d TextureMap::getPixelAt( int x, int y ) const
{
    // This keeps it from crashing if it can't load
    // the texture, but the person tries to render anyway.
    if (0 == data)
      return Vec3d(1.0, 1.0, 1.0);

    if( x >= width )
       x = width - 1;
    if( y >= height )
       y = height - 1;

    // Find the position in the big data array...
    int pos = (y * width + x) * 3;
    return Vec3d(double(data[pos]) / 255.0, 
       double(data[pos+1]) / 255.0,
       double(data[pos+2]) / 255.0);
}

Vec3d MaterialParameter::value( const isect& is ) const
{
    if( 0 != _textureMap )
        return _textureMap->getMappedValue( is.uvCoordinates );
    else
        return _value;
}

double MaterialParameter::intensityValue( const isect& is ) const
{
    if( 0 != _textureMap )
    {
        Vec3d value( _textureMap->getMappedValue( is.uvCoordinates ) );
        return (0.299 * value[0]) + (0.587 * value[1]) + (0.114 * value[2]);
    }
    else
        return (0.299 * _value[0]) + (0.587 * _value[1]) + (0.114 * _value[2]);
}


Vec3d Material::getDisNormal(const Vec2d& coord) const {
    
    
    
    int width = RayTracer::backgroundImage_width;
    int height = RayTracer::backgroundImage_height;
    unsigned char * data = RayTracer::backgroundImage;
    
    
    int x = width * coord[0];
    int y = height * coord[1];
    
    if( x >= (width - 1) ) return Vec3d(0,0,0);
 
    if( y >= (height - 1) ) return Vec3d(0,0,0);

    int pos = (y * width + x) * 3;
    
    double Hg = ( double(data[pos]) / 255.0 +  double(data[pos+1]) / 255.0 + double(data[pos+2]) / 255.0 ) / 3;
    
    pos = (y * width + fmin(x + 1, width - 1)) * 3;
    
    double Hr = (double(data[pos]) / 255.0 + double(data[pos+1]) / 255.0 + double(data[pos+2]) / 255.0) / 3;
    
    pos = (fmin(y + 1, height - 1) * width + x) * 3;
    
    double Ha = (double(data[pos]) / 255.0 + double(data[pos+1]) / 255.0 + double(data[pos+2]) / 255.0) / 3;
    
    Vec3d n1 (1, 0, Hr - Hg);
    Vec3d n2 (0, 1, Ha - Hg);
    
    
    Vec3d newN = n1 ^ n2;
    newN.normalize();
    
    return newN;
    
    
    
}
