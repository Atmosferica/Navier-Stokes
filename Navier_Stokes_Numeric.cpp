#define H_D cudaMemcpyHostToDevice
#define D_H cudaMemcpyDeviceToHost

// Includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// CUDA standard includes
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>


#include <helper_functions.h>
#include <helper_cuda.h>


// CUDA FFT Libraries
#include <cufft.h>

#include "defines.h"
#include "fluidsGL_kernels.h"
#define MAX_EPSILON_ERROR 1.0f



//#define MAX(A,B)({A > B ? A : B;})

void cleanup(void);
void reshape(int x, int y);

// CUFFT plan handle
cufftHandle planr2c;
cufftHandle planc2r;
static cData *vxfield = NULL;
static cData *vyfield = NULL;

cData *hvfield = NULL;
cData *dvfield = NULL;

StopWatchInterface *timer = NULL;

// Particle data
static cData *particles = NULL; // particle positions in host memory
//static int lastx = 0, lasty = 0;

// Texture pitch
size_t tPitch = 0; // Now this is compatible with gcc in 64-bit

char *ref_file         = NULL;
bool g_bQAAddTestForce = true;
int  g_iFrameToCompare = 100;
int  g_TotalErrors     = 0;

bool g_bExitESC = false;

void autoTest();

extern "C" void addForces(cData *v, int dx, int dy, int spx, int spy, float fx, float fy, int r);
extern "C" void advectVelocity(cData *v, float *vx, float *vy, int dx, int pdx, int dy, float dt);
extern "C" void diffuseProject(cData *vx, cData *vy, int dx, int dy, float dt, float visc);
extern "C" void updateVelocity(cData *v, float *vx, float *vy, int dx, int pdx, int dy);
//extern "C" void advectParticles(GLuint vbo, cData *v, int dx, int dy, float dt);

void simulateFluids(void)
{
	advectVelocity(dvfield, (float *)vxfield, (float *)vyfield, DIM, RPADW, DIM, DT);
	diffuseProject(vxfield, vyfield, CPADW, DIM, DT, VIS);
	updateVelocity(dvfield, (float *)vxfield, (float *)vyfield, DIM, RPADW, DIM);
}

float myrand(void)
{
    static int seed = 72191;
    char sq[22];

    if (ref_file)
    {
        seed *= seed;
        sprintf(sq, "%010d", seed);
        // pull the middle 5 digits out of sq
        sq[8] = 0;
        seed = atoi(&sq[3]);

        return seed/99999.f;
    }
    else
    {
        return rand()/(float)RAND_MAX;
    }
}

void initParticles(cData *p, int dx, int dy)
{
    int i, j;

    for (i = 0; i < dy; i++)
    {
        for (j = 0; j < dx; j++)
        {
            p[i*dx+j].x = (j+0.5f+(myrand() - 0.5f))/dx;
            p[i*dx+j].y = (i+0.5f+(myrand() - 0.5f))/dy;
        }
    }
}

int main(int argc, char **argv)
{



	//static int wWidth  = MAX(512, DIM);
	//static int wHeight = MAX(512, DIM);

    hvfield = (cData *)malloc(sizeof(cData) * DS);
    memset(hvfield, 0, sizeof(cData) * DS);
    
    cudaMallocPitch((void **)&dvfield, &tPitch, sizeof(cData)*DIM, DIM);
    cudaMemcpy(dvfield, hvfield, sizeof(cData) * DS, H_D);

    cudaMalloc((void **)&vxfield, sizeof(cData) * PDS);
    cudaMalloc((void **)&vyfield, sizeof(cData) * PDS);

    setupTexture(DIM, DIM);
    bindTexture();

    particles = (cData *)malloc(sizeof(cData) * DS);
    memset(particles, 0, sizeof(cData) * DS);

    initParticles(particles, DIM, DIM);

    cufftPlan2d(&planr2c, DIM, DIM, CUFFT_R2C);
    cufftPlan2d(&planc2r, DIM, DIM, CUFFT_C2R);
    cufftSetCompatibilityMode(planr2c, CUFFT_COMPATIBILITY_FFTW_PADDING);
    cufftSetCompatibilityMode(planc2r, CUFFT_COMPATIBILITY_FFTW_PADDING);

	while(true){
		simulateFluids();
	}
	
	return 0;
}
