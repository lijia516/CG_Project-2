#include <cmath>

#include "light.h"

using namespace std;

double DirectionalLight::distanceAttenuation(const Vec3d& P) const
{
  // distance to light is infinite, so f(di) goes to 0.  Return 1.
  return 1.0;
}


Vec3d DirectionalLight::shadowAttenuation(const ray& r, const Vec3d& p) const
{
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.
    isect i;
    ray light_ray(p, getDirection(p), ray::VISIBILITY);
    
    if (scene->intersect(light_ray, i)) {
        
        if (i.material->Trans()) {
            return i.material->kt(i);
        }
        
        return Vec3d(0,0,0);
    }
    
    return Vec3d(1,1,1);
}

Vec3d DirectionalLight::getColor() const
{
  return color;
}

Vec3d DirectionalLight::getDirection(const Vec3d& P) const
{
  // for directional light, direction doesn't depend on P
  return -orientation;
}

double PointLight::distanceAttenuation(const Vec3d& P) const
{

  // YOUR CODE HERE

  // You'll need to modify this method to attenuate the intensity 
  // of the light based on the distance between the source and the 
  // point P.  For now, we assume no attenuation and just return 1.0
    double d = (position - P).length();
    d = 1.0 / (constantTerm + linearTerm * d + quadraticTerm * d * d);
    
    return min(d, 1.0);
}

Vec3d PointLight::getColor() const
{
  return color;
}

Vec3d PointLight::getDirection(const Vec3d& P) const
{
  Vec3d ret = position - P;
  ret.normalize();
  return ret;
}


Vec3d PointLight::shadowAttenuation(const ray& r, const Vec3d& p) const
{
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.
    
    
    double size = TraceUI::m_nFilter;
    
    Vec3d color(0,0,0);
    
    int count = 0;
    for (double i = -size + position[0]; i <= size + position[0]; i = i + 0.2) {
        for (double j = -size + position[1]; j <= size + position[1]; j = j + 0.2) {
            
            color += calclulate(r,  p, Vec3d(i, j, position[2]));
            count++;
        }
    }
    
    color /= count;
    return color;
}

Vec3d PointLight::calclulate(const ray& r, const Vec3d& p, const Vec3d& position) const {
    
    Vec3d dir = position - p;
    dir.normalize();
    
    
    isect i;
    ray light_ray(p, dir, ray::VISIBILITY);
    
    if (scene->intersect(light_ray, i)) {
        
        Vec3d q_m = light_ray.at(i.t);
        
        if ( (p[0] - position[0]) * (q_m[0] - position[0]) >= 0 && abs(p[0] - position[0]) > abs(q_m[0] - position[0])) {   //(p - position).length() > (q_m - position).length()) {
            
            
            if (i.material->Trans()) {
                return i.material->kt(i);
            }
            
            
            return Vec3d(0,0,0);
        }
    }
    return Vec3d(1,1,1);
    
}
