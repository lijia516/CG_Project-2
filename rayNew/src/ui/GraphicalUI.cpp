//
// GraphicalUI.cpp
//
// Handles FLTK integration and other user interface tasks
//
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <thread>

#ifndef COMMAND_LINE_ONLY

#include <FL/fl_ask.H>
#include "debuggingView.h"

#include "GraphicalUI.h"
#include "../RayTracer.h"

#define MAX_INTERVAL 500

#ifdef _WIN32
#define print sprintf_s
#else
#define print sprintf
#endif

bool GraphicalUI::stopTrace = false;
bool GraphicalUI::doneTrace = true;

GraphicalUI* GraphicalUI::pUI = NULL;

char* GraphicalUI::traceWindowLabel = "Raytraced Image";

bool TraceUI::m_debug = false;
bool TraceUI::m_cubeMap = false;
bool TraceUI::m_kd = false;
bool TraceUI::m_hasJitteredSupersample = false;
bool TraceUI::m_glossyRefection = false;
bool TraceUI::m_DOF = false;
bool TraceUI::m_adaptiveAntiliasing = false;

int TraceUI::m_nKdDepth = 10;
int TraceUI::m_nKdLeaves = 3;

double TraceUI::m_nFilter = 0;
double TraceUI::m_nDOF = 50;
double TraceUI::m_nAaThresh = 3;
double TraceUI::m_nThreshold = 0;

static int gobal_y;
static std::mutex _lock;

//------------------------------------- Help Functions --------------------------------------------
GraphicalUI* GraphicalUI::whoami(Fl_Menu_* o)	// from menu item back to UI itself
{
	return ((GraphicalUI*)(o->parent()->user_data()));
}

//--------------------------------- Callback Functions --------------------------------------------
void GraphicalUI::cb_load_scene(Fl_Menu_* o, void* v) 
{
	pUI = whoami(o);

	static char* lastFile = 0;
	char* newfile = fl_file_chooser("Open Scene?", "*.ray", NULL );

	if (newfile != NULL) {
		char buf[256];

		if (pUI->raytracer->loadScene(newfile)) {
			print(buf, "Ray <%s>", newfile);
			stopTracing();	// terminate the previous rendering
		} else print(buf, "Ray <Not Loaded>");

		pUI->m_mainWindow->label(buf);
		pUI->m_debuggingWindow->m_debuggingView->setDirty();

		if( lastFile != 0 && strcmp(newfile, lastFile) != 0 )
			pUI->m_debuggingWindow->m_debuggingView->resetCamera();

		pUI->m_debuggingWindow->redraw();
	}
}

void GraphicalUI::cb_save_image(Fl_Menu_* o, void* v) 
{
	pUI = whoami(o);

	char* savefile = fl_file_chooser("Save Image?", "*.bmp", "save.bmp" );
	if (savefile != NULL) {
		pUI->m_traceGlWindow->saveImage(savefile);
	}
}

void GraphicalUI::cb_exit(Fl_Menu_* o, void* v)
{
	pUI = whoami(o);

	// terminate the rendering
	stopTracing();

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
	pUI->m_debuggingWindow->hide();
    pUI->m_cubeMapChooser->hide();
	TraceUI::m_debug = false;
    TraceUI::m_cubeMap = false;
}

void GraphicalUI::cb_exit2(Fl_Widget* o, void* v) 
{
	pUI = (GraphicalUI *)(o->user_data());

	// terminate the rendering
	stopTracing();

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
	pUI->m_debuggingWindow->hide();
    pUI->m_cubeMapChooser->hide();
	TraceUI::m_debug = false;
    TraceUI::m_cubeMap = false;
}

void GraphicalUI::cb_about(Fl_Menu_* o, void* v) 
{
	fl_message("RayTracer Project for CS384g.");
}




void GraphicalUI::cb_multiThreadsLightButton(Fl_Widget* o, void* v)
{
    pUI=(GraphicalUI*)(o->user_data());
    
    if (pUI->m_hasMultiThreads==true) pUI->m_hasMultiThreads=false;
    
    else {
        
        pUI->m_hasMultiThreads=true;
        
    }
    
    std::cout << "multi threads: " << pUI->m_hasMultiThreads << "\n";
}




void GraphicalUI::cb_jitteredSupersampleLightButton(Fl_Widget* o, void* v)
{
    pUI=(GraphicalUI*)(o->user_data());
    
    if (pUI->m_hasJitteredSupersample==true) pUI->m_hasJitteredSupersample=false;
    
    else {
        
        pUI->m_hasJitteredSupersample=true;
        
    }
    
    std::cout << "Jittered Supersample: " << pUI->m_hasJitteredSupersample << "\n";
}



void GraphicalUI::cb_sizeSlides(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());

	// terminate the rendering so we don't get crashes
	stopTracing();

	pUI->m_nSize=int(((Fl_Slider *)o)->value());
	int width = (int)(pUI->getSize());
	int height = (int)(width / pUI->raytracer->aspectRatio() + 0.5);
	pUI->m_traceGlWindow->resizeWindow(width, height);
}

void GraphicalUI::cb_depthSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->m_nDepth=int( ((Fl_Slider *)o)->value() ) ;
}


void GraphicalUI::cb_samplingSlides(Fl_Widget* o, void* v)
{
    ((GraphicalUI*)(o->user_data()))->m_nSampling=int( ((Fl_Slider *)o)->value() ) ;
}


void GraphicalUI::cb_dofSlides(Fl_Widget* o, void* v)
{
    ((GraphicalUI*)(o->user_data()))->m_nDOF=double( ((Fl_Slider *)o)->value() ) ;
    
    
}

void GraphicalUI::cb_aaThreshSlider(Fl_Widget* o, void* v)
{
    ((GraphicalUI*)(o->user_data()))->m_nAaThresh=double( ((Fl_Slider *)o)->value() ) ;
    
}


void GraphicalUI::cb_filterSlides(Fl_Widget* o, void* v)
{
    ((GraphicalUI*)(o->user_data()))->m_nFilter=double( ((Fl_Slider *)o)->value() ) ;
}

void GraphicalUI::cb_multiThreadsSlides(Fl_Widget* o, void* v)
{
    ((GraphicalUI*)(o->user_data()))->m_nMultiThreads=int( ((Fl_Slider *)o)->value() ) ;
}



void GraphicalUI::cb_kdDepthSlides(Fl_Widget* o, void* v)
{
    ((GraphicalUI*)(o->user_data()))->m_nKdDepth=int( ((Fl_Slider *)o)->value() ) ;
}

void GraphicalUI::cb_kdLeavesSlides(Fl_Widget* o, void* v)
{
    ((GraphicalUI*)(o->user_data()))->m_nKdLeaves=int( ((Fl_Slider *)o)->value() ) ;
}


void GraphicalUI::cb_refreshSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->refreshInterval=clock_t(((Fl_Slider *)o)->value()) ;
}


void GraphicalUI::cb_thresholdSlides(Fl_Widget* o, void* v)
{
    ((GraphicalUI*)(o->user_data()))->m_nThreshold=double(((Fl_Slider *)o)->value()) ;
   
}




void GraphicalUI::cb_kdCheckButton(Fl_Widget* o, void* v)
{
    pUI=(GraphicalUI*)(o->user_data());
    pUI->m_kdInfo = (((Fl_Check_Button*)o)->value() == 1);
    if (pUI->m_kdInfo)
    {
        pUI->m_treeDepthSlider->activate();
        pUI->m_leafSizeSlider->activate();
        pUI->m_kd = true;
    }
    else
    {
        pUI->m_treeDepthSlider->deactivate();
        pUI->m_leafSizeSlider->deactivate();
        pUI->m_kd = false;
    }
}

void GraphicalUI::cb_debuggingDisplayCheckButton(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_displayDebuggingInfo = (((Fl_Check_Button*)o)->value() == 1);
	if (pUI->m_displayDebuggingInfo)
	  {
	    pUI->m_debuggingWindow->show();
	    pUI->m_debug = true;
	  }
	else
	  {
	    pUI->m_debuggingWindow->hide();
	    pUI->m_debug = false;
	  }
}

void GraphicalUI::cb_cubeMapCheckButton(Fl_Widget* o, void* v)
{
    pUI=(GraphicalUI*)(o->user_data());
    pUI->m_cubeMapInfo = (((Fl_Check_Button*)o)->value() == 1);
    if (pUI->m_cubeMapInfo)
    {
        pUI->m_cubeMapChooser->show();
    }
    else
    {
        pUI->m_cubeMapChooser->hide();
        pUI->m_cubeMap = false;
    }
    
    
    std::cout << "m_cubeMap: " << pUI->m_cubeMap << "\n";
}


void GraphicalUI::cb_grCheckButton(Fl_Widget* o, void* v)
{
    pUI=(GraphicalUI*)(o->user_data());
    pUI->m_grInfo = (((Fl_Check_Button*)o)->value() == 1);
    if (pUI->m_grInfo)
    {
      
        pUI->m_glossyRefection = true;
    }
    else
    {
     
        pUI->m_glossyRefection = false;
    }
    
    
    std::cout << "m_glossyRefection: " << pUI->m_glossyRefection << "\n";
}


void GraphicalUI::cb_dofCheckButton(Fl_Widget* o, void* v)
{
    pUI=(GraphicalUI*)(o->user_data());
    pUI->m_dofInfo = (((Fl_Check_Button*)o)->value() == 1);
    if (pUI->m_dofInfo)
    {
        
        pUI->m_DOF = true;
    }
    else
    {
        
        pUI->m_DOF = false;
    }
    
    
    std::cout << "m_DOF: " << pUI->m_DOF << "\n";
}




void GraphicalUI::cb_aaCheckButton(Fl_Widget* o, void* v)
{
    pUI=(GraphicalUI*)(o->user_data());
    pUI->m_aaInfo = (((Fl_Check_Button*)o)->value() == 1);
    if (pUI->m_aaInfo)
    {
        
        pUI->m_adaptiveAntiliasing = true;
    }
    else
    {
        
        pUI->m_adaptiveAntiliasing = false;
    }
    
    
    std::cout << "m_adaptiveAntiliasing: " << pUI->m_adaptiveAntiliasing << "\n";
}


void GraphicalUI::cb_render(Fl_Widget* o, void* v) {
    
    char buffer[256];
    
    pUI = (GraphicalUI*)(o->user_data());
    doneTrace = stopTrace = false;
    if (pUI->raytracer->sceneLoaded())
    {
        int width = pUI->getSize();
        int height = (int)(width / pUI->raytracer->aspectRatio() + 0.5);
        int origPixels = width * height;
        pUI->m_traceGlWindow->resizeWindow(width, height);
        pUI->m_traceGlWindow->show();
        pUI->raytracer->traceSetup(width, height);
        
        // Save the window label
        const char *old_label = pUI->m_traceGlWindow->label();
        
        if (pUI->m_hasMultiThreads) {
            gobal_y = 0;
            int num = pUI->getMultiThreads();
            std::vector<std::thread> threads;
            for (int i = 0; i < num; i++) {
                
                threads.push_back(std::thread(call_from_thread, pUI, stopTrace, buffer, width, height, old_label));
            }
            for(auto& t : threads) t.join();
            
        } else {
            
            clock_t now, prev;
            now = prev = clock();
            clock_t intervalMS = pUI->refreshInterval * 100;
            
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    if (stopTrace) break;
                    // check for input and refresh view every so often while tracing
                    now = clock();
                    if ((now - prev)/CLOCKS_PER_SEC * 1000 >= intervalMS)
                    {
                        prev = now;
                        sprintf(buffer, "(%d%%) %s", (int)((double)y / (double)height * 100.0), old_label);
                        pUI->m_traceGlWindow->label(buffer);
                        pUI->m_traceGlWindow->refresh();
                        Fl::check();
                        if (Fl::damage()) { Fl::flush(); }
                    }
                    // look for input and refresh window
                    
                    
                    //  std::cout<< "before tracePixel: " <<"\n";
                    
                    pUI->raytracer->tracePixel(x, y);
                    pUI->m_debuggingWindow->m_debuggingView->setDirty();
                }
                if (stopTrace) break;
            }
        }
        doneTrace = true;
        stopTrace = false;
        // Restore the window label
        pUI->m_traceGlWindow->label(old_label);
        pUI->m_traceGlWindow->refresh();
    }
}


void GraphicalUI::call_from_thread(GraphicalUI* pUI, bool stopTrace, char* buffer, int width, int height, const char *old_label) {
    
    int y;
    
    clock_t now, prev;
    now = prev = clock();
    clock_t intervalMS = pUI->refreshInterval * 100;
    
    while (gobal_y <= height - 1) {
        
        
        _lock.lock();
        
        y = gobal_y;
        gobal_y++;
        
        // std::cout<< "thead: " << std::this_thread::get_id() << "," << gobal_y<< "," << y <<"\n";
        
        _lock.unlock();
        
        
        if (gobal_y > height) return;
        
        for (int x = 0; x < width; x++)
        {
            if (stopTrace) break;
            // check for input and refresh view every so often while tracing
            // now = clock();
            // if ((now - prev)/CLOCKS_PER_SEC * 1000 >= intervalMS)
            // {
            //      prev = now;
            //      sprintf(buffer, "(%d%%) %s", (int)((double)y / (double)height * 100.0), old_label);
            //      pUI->m_traceGlWindow->label(buffer);
            //      pUI->m_traceGlWindow->refresh();
            //      Fl::check();
            //     if (Fl::damage()) { Fl::flush(); }
            // }
            // look for input and refresh window
            
            //std::cout<< "x, y " << x << "," << y <<"\n";
            pUI->raytracer->tracePixel(x, y);
            //  pUI->m_debuggingWindow->m_debuggingView->setDirty();
        }
        if (stopTrace) break;
    }
}


void GraphicalUI::cb_stop(Fl_Widget* o, void* v)
{
	pUI = (GraphicalUI*)(o->user_data());
	stopTracing();
}

void GraphicalUI::cb_antiAliased(Fl_Widget* o, void* v)
{
    // pUI = (GraphicalUI*)(o->user_data());
    // pUI->raytracer->antiAliased(pUI->getSampling());
    // pUI->m_traceGlWindow->refresh();
    
    char buffer[256];
    
    pUI = (GraphicalUI*)(o->user_data());
    doneTrace = stopTrace = false;
    if (pUI->raytracer->sceneLoaded())
    {
        int width = pUI->getSize();
        int height = (int)(width / pUI->raytracer->aspectRatio() + 0.5);
        int origPixels = width * height;
        pUI->m_traceGlWindow->resizeWindow(width, height);
        pUI->m_traceGlWindow->show();
        pUI->raytracer->traceSetup(width, height);
        
        // Save the window label
        const char *old_label = pUI->m_traceGlWindow->label();
        
        clock_t now, prev;
        now = prev = clock();
        clock_t intervalMS = pUI->refreshInterval * 100;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                if (stopTrace) break;
                // check for input and refresh view every so often while tracing
                now = clock();
                if ((now - prev)/CLOCKS_PER_SEC * 1000 >= intervalMS)
                {
                    prev = now;
                    sprintf(buffer, "(%d%%) %s", (int)((double)y / (double)height * 100.0), old_label);
                    pUI->m_traceGlWindow->label(buffer);
                    pUI->m_traceGlWindow->refresh();
                    Fl::check();
                    if (Fl::damage()) { Fl::flush(); }
                }
                // look for input and refresh window
                pUI->raytracer->antiAliased(pUI->getSampling(), x, y);
                pUI->m_debuggingWindow->m_debuggingView->setDirty();
            }
            if (stopTrace) break;
        }
        doneTrace = true;
        stopTrace = false;
        // Restore the window label
        pUI->m_traceGlWindow->label(old_label);
        pUI->m_traceGlWindow->refresh();
    }
    
    
    
}


int GraphicalUI::run()
{
	Fl::visual(FL_DOUBLE|FL_INDEX);

	m_mainWindow->show();

	return Fl::run();
}

void GraphicalUI::alert( const string& msg )
{
	fl_alert( "%s", msg.c_str() );
}

void GraphicalUI::setRayTracer(RayTracer *tracer)
{
	TraceUI::setRayTracer(tracer);
	m_traceGlWindow->setRayTracer(tracer);
	m_debuggingWindow->m_debuggingView->setRayTracer(tracer);
}

// menu definition
Fl_Menu_Item GraphicalUI::menuitems[] = {
	{ "&File", 0, 0, 0, FL_SUBMENU },
	{ "&Load Scene...",	FL_ALT + 'l', (Fl_Callback *)GraphicalUI::cb_load_scene },
	{ "&Save Image...", FL_ALT + 's', (Fl_Callback *)GraphicalUI::cb_save_image },
	{ "&Exit", FL_ALT + 'e', (Fl_Callback *)GraphicalUI::cb_exit },
	{ 0 },

	{ "&Help",		0, 0, 0, FL_SUBMENU },
	{ "&About",	FL_ALT + 'a', (Fl_Callback *)GraphicalUI::cb_about },
	{ 0 },

	{ 0 }
};

void GraphicalUI::stopTracing()
{
	stopTrace = true;
}

GraphicalUI::GraphicalUI() : refreshInterval(10) {
	// init.
	m_mainWindow = new Fl_Window(100, 40, 450, 459, "Ray <Not Loaded>");
	m_mainWindow->user_data((void*)(this));	// record self to be used by static callback functions
	// install menu bar
	m_menubar = new Fl_Menu_Bar(0, 0, 440, 25);
	m_menubar->menu(menuitems);

	// set up "render" button
	m_renderButton = new Fl_Button(360, 37, 70, 25, "&Render");
	m_renderButton->user_data((void*)(this));
	m_renderButton->callback(cb_render);

	// set up "stop" button
	m_stopButton = new Fl_Button(360, 65, 70, 25, "&Stop");
	m_stopButton->user_data((void*)(this));
	m_stopButton->callback(cb_stop);
    
    
    // set up "antiAliased" button
    m_antiAliasedButton = new Fl_Button(330, 220, 110, 25, "&Anti-aliased");
    m_antiAliasedButton->user_data((void*)(this));
    m_antiAliasedButton->callback(cb_antiAliased);
    
    
    
    
    // Add multi Threads button
    m_multiThreadsLightButton = new Fl_Light_Button(330,190,110,25,"&Multi Threads");
    m_multiThreadsLightButton->user_data((void*)(this));   // record self to be used by static callback functions
    m_multiThreadsLightButton->callback(cb_multiThreadsLightButton);
    
    
    // Add multi Threads button
    m_jitteredSupersampleLightButton = new Fl_Light_Button(290,280,150,25,"&jittered Supersample");
    m_jitteredSupersampleLightButton->user_data((void*)(this));   // record self to be used by static callback functions
    m_jitteredSupersampleLightButton->callback(cb_jitteredSupersampleLightButton);
    

	// install depth slider
	m_depthSlider = new Fl_Value_Slider(10, 40, 180, 20, "Recursion Depth");
	m_depthSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_depthSlider->type(FL_HOR_NICE_SLIDER);
	m_depthSlider->labelfont(FL_COURIER);
	m_depthSlider->labelsize(12);
	m_depthSlider->minimum(0);
	m_depthSlider->maximum(10);
	m_depthSlider->step(1);
	m_depthSlider->value(m_nDepth);
	m_depthSlider->align(FL_ALIGN_RIGHT);
	m_depthSlider->callback(cb_depthSlides);

	// install size slider
	m_sizeSlider = new Fl_Value_Slider(10, 65, 180, 20, "Screen Size");
	m_sizeSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_sizeSlider->type(FL_HOR_NICE_SLIDER);
	m_sizeSlider->labelfont(FL_COURIER);
	m_sizeSlider->labelsize(12);
	m_sizeSlider->minimum(64);
	m_sizeSlider->maximum(1024);
	m_sizeSlider->step(2);
	m_sizeSlider->value(m_nSize);
	m_sizeSlider->align(FL_ALIGN_RIGHT);
	m_sizeSlider->callback(cb_sizeSlides);

	// install refresh interval slider
	m_refreshSlider = new Fl_Value_Slider(10, 90, 180, 20, "Screen Refresh Interval (0.1 sec)");
	m_refreshSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_refreshSlider->type(FL_HOR_NICE_SLIDER);
	m_refreshSlider->labelfont(FL_COURIER);
	m_refreshSlider->labelsize(12);
	m_refreshSlider->minimum(1);
	m_refreshSlider->maximum(300);
	m_refreshSlider->step(1);
	m_refreshSlider->value(refreshInterval);
	m_refreshSlider->align(FL_ALIGN_RIGHT);
	m_refreshSlider->callback(cb_refreshSlides);
    
    
    
    
    // install refresh interval slider
    m_thresholdSlider = new Fl_Value_Slider(10, 160, 180, 20, "daptive termination threshold");
    m_thresholdSlider->user_data((void*)(this));	// record self to be used by static callback functions
    m_thresholdSlider->type(FL_HOR_NICE_SLIDER);
    m_thresholdSlider->labelfont(FL_COURIER);
    m_thresholdSlider->labelsize(12);
    m_thresholdSlider->minimum(0);
    m_thresholdSlider->maximum(0.2);
    m_thresholdSlider->step(0.01);
    m_thresholdSlider->value(m_nThreshold);
    m_thresholdSlider->align(FL_ALIGN_RIGHT);
    m_thresholdSlider->callback(cb_thresholdSlides);
    
    
    
    // install sampling slider
    m_samplingSlider = new Fl_Value_Slider(10, 220, 180, 20, "Samples number");
    m_samplingSlider->user_data((void*)(this));	// record self to be used by static callback functions
    m_samplingSlider->type(FL_HOR_NICE_SLIDER);
    m_samplingSlider->labelfont(FL_COURIER);
    m_samplingSlider->labelsize(12);
    m_samplingSlider->minimum(1);
    m_samplingSlider->maximum(16);
    m_samplingSlider->step(1);
    m_samplingSlider->value(m_nSampling);
    m_samplingSlider->align(FL_ALIGN_RIGHT);
    m_samplingSlider->callback(cb_samplingSlides);
    
    // install sampling slider
    m_dofSlider = new Fl_Value_Slider(130, 310, 180, 20, "DOF");
    m_dofSlider->user_data((void*)(this));	// record self to be used by static callback functions
    m_dofSlider->type(FL_HOR_NICE_SLIDER);
    m_dofSlider->labelfont(FL_COURIER);
    m_dofSlider->labelsize(12);
    m_dofSlider->minimum(1);
    m_dofSlider->maximum(50);
    m_dofSlider->step(0.05);
    m_dofSlider->value(m_nDOF);
    m_dofSlider->align(FL_ALIGN_RIGHT);
    m_dofSlider->callback(cb_dofSlides);
    
    
    
    
    // install sampling slider
    m_aaThreshSlider = new Fl_Value_Slider(10, 250, 180, 20, "adaptive anti-aliasing threshold");
    m_aaThreshSlider->user_data((void*)(this));	// record self to be used by static callback functions
    m_aaThreshSlider->type(FL_HOR_NICE_SLIDER);
    m_aaThreshSlider->labelfont(FL_COURIER);
    m_aaThreshSlider->labelsize(12);
    m_aaThreshSlider->minimum(0);
    m_aaThreshSlider->maximum(3);
    m_aaThreshSlider->step(0.01);
    m_aaThreshSlider->value(m_nAaThresh);
    m_aaThreshSlider->align(FL_ALIGN_RIGHT);
    m_aaThreshSlider->callback(cb_aaThreshSlider);
    
    
    
    
    // install sampling slider
    m_filterSlider = new Fl_Value_Slider(10, 125, 180, 20, "soft shadow size");
    m_filterSlider->user_data((void*)(this));	// record self to be used by static callback functions
    m_filterSlider->type(FL_HOR_NICE_SLIDER);
    m_filterSlider->labelfont(FL_COURIER);
    m_filterSlider->labelsize(12);
    m_filterSlider->minimum(0);
    m_filterSlider->maximum(2);
    m_filterSlider->step(0.1);
    m_filterSlider->value(m_nFilter);
    m_filterSlider->align(FL_ALIGN_RIGHT);
    m_filterSlider->callback(cb_filterSlides);
    
    
    
    // install multiThreads slider
    m_multiThreadsSlider = new Fl_Value_Slider(10, 190, 180, 20, "threads number");
    m_multiThreadsSlider->user_data((void*)(this));	// record self to be used by static callback functions
    m_multiThreadsSlider->type(FL_HOR_NICE_SLIDER);
    m_multiThreadsSlider->labelfont(FL_COURIER);
    m_multiThreadsSlider->labelsize(12);
    m_multiThreadsSlider->minimum(1);
    m_multiThreadsSlider->maximum(16);
    m_multiThreadsSlider->step(1);
    m_multiThreadsSlider->value(m_nMultiThreads);
    m_multiThreadsSlider->align(FL_ALIGN_RIGHT);
    m_multiThreadsSlider->callback(cb_multiThreadsSlides);
    
    
    // install multiThreads slider
    m_treeDepthSlider = new Fl_Value_Slider(130, 340, 180, 20, "kd tree depth");
    m_treeDepthSlider->user_data((void*)(this));	// record self to be used by static callback functions
    m_treeDepthSlider->type(FL_HOR_NICE_SLIDER);
    m_treeDepthSlider->labelfont(FL_COURIER);
    m_treeDepthSlider->labelsize(12);
    m_treeDepthSlider->minimum(1);
    m_treeDepthSlider->maximum(30);
    m_treeDepthSlider->step(1);
    m_treeDepthSlider->value(m_nKdDepth);
    m_treeDepthSlider->align(FL_ALIGN_RIGHT);
    m_treeDepthSlider->callback(cb_kdDepthSlides);
    m_treeDepthSlider->deactivate();
    
    // install multiThreads slider
    m_leafSizeSlider = new Fl_Value_Slider(130, 360, 180, 20, "kd tree leave size");
    m_leafSizeSlider->user_data((void*)(this));	// record self to be used by static callback functions
    m_leafSizeSlider->type(FL_HOR_NICE_SLIDER);
    m_leafSizeSlider->labelfont(FL_COURIER);
    m_leafSizeSlider->labelsize(12);
    m_leafSizeSlider->minimum(1);
    m_leafSizeSlider->maximum(10);
    m_leafSizeSlider->step(1);
    m_leafSizeSlider->value(m_nKdLeaves);
    m_leafSizeSlider->align(FL_ALIGN_RIGHT);
    m_leafSizeSlider->callback(cb_kdLeavesSlides);
    m_leafSizeSlider->deactivate();
    
    
    // kd tree check box
    m_kdCheckButton = new Fl_Check_Button(10, 345, 80, 20, "Kd Tree");
    m_kdCheckButton->user_data((void*)(this));
    m_kdCheckButton->callback(cb_kdCheckButton);
    m_kdCheckButton->value(m_kdInfo);
    
    
	// set up debugging display checkbox
	m_debuggingDisplayCheckButton = new Fl_Check_Button(10, 429, 140, 20, "Debugging display");
	m_debuggingDisplayCheckButton->user_data((void*)(this));
	m_debuggingDisplayCheckButton->callback(cb_debuggingDisplayCheckButton);
	m_debuggingDisplayCheckButton->value(m_displayDebuggingInfo);
    
    // set up debugging display checkbox
    m_cubeMapCheckButton = new Fl_Check_Button(10, 400, 140, 20, "Cube Map Chooser");
    m_cubeMapCheckButton->user_data((void*)(this));
    m_cubeMapCheckButton->callback(cb_cubeMapCheckButton);
    m_cubeMapCheckButton->value(m_cubeMapInfo);
    
    
    // set up debugging display checkbox
    m_grCheckButton = new Fl_Check_Button(200, 400, 140, 20, "glossy reflection");
    m_grCheckButton->user_data((void*)(this));
    m_grCheckButton->callback(cb_grCheckButton);
    m_grCheckButton->value(m_grInfo);
    
    
    // set up debugging display checkbox
    m_grCheckButton = new Fl_Check_Button(10, 310, 140, 20, "depth of field");
    m_grCheckButton->user_data((void*)(this));
    m_grCheckButton->callback(cb_dofCheckButton);
    m_grCheckButton->value(m_dofInfo);
    
    
    // set up debugging display checkbox
    m_aaCheckButton = new Fl_Check_Button(10, 280, 140, 20, "adaptive antialiasing");
    m_aaCheckButton->user_data((void*)(this));
    m_aaCheckButton->callback(cb_aaCheckButton);
    m_aaCheckButton->value(m_aaInfo);
    

	m_mainWindow->callback(cb_exit2);
	m_mainWindow->when(FL_HIDE);
	m_mainWindow->end();

	// image view
	m_traceGlWindow = new TraceGLWindow(100, 150, m_nSize, m_nSize, traceWindowLabel);
	m_traceGlWindow->end();
	m_traceGlWindow->resizable(m_traceGlWindow);

	// debugging view
	m_debuggingWindow = new DebuggingWindow();
    
    m_cubeMapChooser = new CubeMapChooser();
    m_cubeMapChooser->setCaller(this);
}

#endif
