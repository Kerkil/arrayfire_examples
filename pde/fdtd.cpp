#include <stdio.h>
#include <math.h>
#include <arrayfire.h>
#include <string.h> // memset
#include "../common/progress.h"

using namespace af;

struct pml_t {
    int npml_x, npml_z;
    int pml_vec[4];
};
struct medium_t {
    float Mstart, epsilon_r, chi_1, tau_0, sigma, dt;
    bool back_bound;
    array gbc;
};
typedef enum { P_GAUSS = 1, P_SINE = 2 } pchoice_t;
typedef enum { P_ADDITIVE = 1, P_HARD = 0 } ptype_t;
struct pulse_t {
    int Az, Aw, Ac; // antenna coordinates
    seq ind;
    pchoice_t choice;
    ptype_t   type;
    float stdev, cut_off, t0, dt, freq_in, amplitude;
};

// initialize and populate CPU-side matries
static float *cpu_constant(float val, int n)
{
    float *vals = (float *)malloc(n*sizeof(*vals));
    for (int i = 0; i < n; ++i)
        vals[i] = val;
    return vals;
}

// get electric pulse information
static void get_pulse(int Nx, float dt, struct pulse_t& pulse)
{
    if (pulse.choice == P_GAUSS) {
        pulse.t0 = 20;
        pulse.stdev = 6;
        pulse.cut_off = 2 * pulse.t0;
    } else if (pulse.choice == P_SINE) {
        float freq_in_MHz = 3000;            // frequency in megahertz.
        pulse.freq_in = 1e6f * freq_in_MHz;
        float cut_off = 3;  //  number of sine wave cycles
        pulse.cut_off = cut_off / (pulse.freq_in * dt);
        if (ceil(cut_off) != cut_off) { // equal if freq_in_MHZ*ddx divides 3e8.
            fprintf(stderr, __FILE__":%d: nonzero sine wave cuttoff\n", __LINE__);
        }
        pulse.dt = dt;
    } else {
        fprintf(stderr, __FILE__":%d: undefined wave type: %d\n", __LINE__, pulse.choice);
        exit(EXIT_FAILURE);
    }

    // Determine the type of pulse used
    pulse.amplitude = 1;
    pulse.type = P_HARD;

    int Az = pulse.Az, Aw = pulse.Aw, Ac = pulse.Ac;
    pulse.ind = Nx * Az + Ac - 1 + seq(-Aw / 2, Aw / 2);
}

// create parameters that characterize the medium
static void medium_params_debye(array& ga, array& gb, array& gbc, int Nx, int Nz, float dt, struct medium_t& m)
{
    // prepare indexing for dielectric region
    seq dielectric(m.Mstart - 1, Nz - m.Mstart + 1);

    // ------------------------------------------------------------------------
    //  Input dielectric constant vector and location of dielectric.
    // ------------------------------------------------------------------------
    array epsilon_r_vec = constant(1,Nx,Nz);
    epsilon_r_vec(span, dielectric) = m.epsilon_r;

    // ------------------------------------------------------------------------
    //  Input the conductivity. Ohmic conductivity: J = sigma*E.
    // ------------------------------------------------------------------------
    array sigma_vec = constant(0,Nx,Nz);
    sigma_vec(span, dielectric) = m.sigma;
    if (m.back_bound) {
        sigma_vec(span, end) = 1e6;    //  To simulate a metal back boundary
    }

    // ------------------------------------------------------------------------
    //  Determine necessary vectors and constants. See (Eq 2.23)
    // ------------------------------------------------------------------------
    float epsilon_0 = 8.8e-12f;  //  Permittivity of free space.
    // FIXME: after matmul
    gb = (dt * sigma_vec) / epsilon_0;
    gbc = constant(0,Nx,Nz);
    gbc(span, dielectric) = m.chi_1 * dt / m.tau_0;
    ga = 1 / (epsilon_r_vec + gb + gbc);
    ga(0,   span) = 0;
    ga(end, span) = 0;
    ga(span, 0) = 0;
    ga(span, end) = 0;

    // ------------------------------------------------------------------------
    //  Save the parameters necessary for later calculations
    // ------------------------------------------------------------------------
    m.gbc = gbc;
    m.dt  = dt;
}

// initialize PML vectors and parameters
static void get_pmlr(array& _gi1, array& _gi2, array& _gi3,
                     array& _gj1, array& _gj2, array& _gj3,
                     array& _fi2, array& _fi3,
                     array& _fj2, array& _fj3,
                     int Nz, int Nx, pml_t p)
{
    // allocate PML vectors
    float *gi1 = cpu_constant(0,Nx), *gi2 = cpu_constant(1,Nx), *gi3 = cpu_constant(1,Nx);
    float *gj1 = cpu_constant(0,Nz), *gj2 = cpu_constant(1,Nz), *gj3 = cpu_constant(1,Nz);
    float *fi2 = cpu_constant(1,Nx),  *fi3 = cpu_constant(1,Nx);
    float *fj2 = cpu_constant(1,Nz),  *fj3 = cpu_constant(1,Nz);

    //------------------------------------------------------------------------
    // Determine PML parameters.
    //------------------------------------------------------------------------
    if (p.npml_x != 0) {
        for (int i = 0; i <= p.npml_x; ++i) {
            float xnum = (float)p.npml_x - i;
            // D_z
            float xxn = xnum / p.npml_x;
            float xn = .333f * pow(xxn, 3);
            if (p.pml_vec[1] == 1) {
                gi1[i] = xn;
                gi2[i] = 1 / (1 + xn);
                gi3[i] = (1 - xn) / (1 + xn);
            }
            if (p.pml_vec[0] == 1) {
                gi1[Nx - i - 1] = xn;
                gi2[Nx - i - 1] = 1 / (1 + xn);
                gi3[Nx - i - 1] = (1 - xn) / (1 + xn);
            }

            // for H_x and H_y
            xxn = (xnum - 0.5f) / p.npml_x;
            xn = .333f * pow(xxn, 3);
            if (p.pml_vec[1] == 1) {
                fi2[i] = 1 / (1 + xn);
                fi3[i] = (1 - xn) / (1 + xn);
            }
            if (p.pml_vec[0] == 1) {
                fi2[Nx - i - 2] = 1 / (1 + xn);
                fi3[Nx - i - 2] = (1 - xn) / (1 + xn);
            }
        }
    }

    if (p.npml_z != 0) {
        for (int j = 0; j <= p.npml_z; ++j) {
            float xnum = (float)p.npml_z - j;

            // D_z
            float xxn = xnum / p.npml_z;
            float xn = .333f * pow(xxn, 3);
            if (p.pml_vec[3] == 1) {
                gj1[j] = xn;
                gj2[j] = 1 / (1 + xn);
                gj3[j] = (1 - xn) / (1 + xn);
            }
            if (p.pml_vec[2] == 1) {
                gj1[Nz - j - 1] = xn;
                gj2[Nz - j - 1] = 1 / (1 + xn);
                gj3[Nz - j - 1] = (1 - xn) / (1 + xn);
            }

            // for H_x and H_y
            xxn = (xnum - 0.5f) / p.npml_z;
            xn = .333f * pow(xxn, 3);
            if (p.pml_vec[3] == 1) {
                fj2[j] = 1 / (1 + xn);
                fj3[j] = (1 - xn) / (1 + xn);
            }
            if (p.pml_vec[2] == 1) {
                fj2[Nz - j - 2] = 1 / (1 + xn);
                fj3[Nz - j - 2] = (1 - xn) / (1 + xn);
            }
        }
    }

    // push to GPU and expand
    _gi1 = tile(array(Nx, gi1), 1, Nz); free(gi1);
    _gi2 = tile(array(Nx, gi2), 1, Nz); free(gi2);
    _gi3 = tile(array(Nx, gi3), 1, Nz); free(gi3);

    _gj1 = tile(array(Nz, gj1), 1, Nx).T(); free(gj1);
    _gj2 = tile(array(Nz, gj2), 1, Nx).T(); free(gj2);
    _gj3 = tile(array(Nz, gj3), 1, Nx).T(); free(gj3);

    _fi2 = tile(array(Nx, fi2), 1, Nz); free(fi2);
    _fi3 = tile(array(Nx, fi3), 1, Nz); free(fi3);
    _fj2 = tile(array(Nz, fj2), 1, Nx).T(); free(fj2);
    _fj3 = tile(array(Nz, fj3), 1, Nx).T(); free(fj3);
}


// compute electromagnetic pulse.
static void compute_pulse(array& d, float T, struct pulse_t& params)
{
    float cut_off = params.cut_off;
    bool is_hard = (params.type == P_HARD);
    seq ind = params.ind; // pulse positions

    if (params.choice == P_GAUSS && T < cut_off) {
        float pulse = params.amplitude * exp(-0.5f * pow((params.t0 - T) / params.stdev, 2));
        if (is_hard) { d(ind)  = pulse; }
        else { d(ind) += pulse; }
    } else if (params.choice == P_SINE && T < cut_off) {
        float pulse = params.amplitude * sin(2 * 3.14159f * params.freq_in * params.dt * T);
        if (is_hard) { d(ind)  = pulse; }
        else { d(ind) += pulse; }
    } else {
        if (is_hard) { d(ind)  = 0; }
    }
}


static void update_s_debye_TM(array& py, const array& ey, const struct medium_t& medium)
{
    // Update the parameter s for a debye medium
    // The auxiliary differential equations approach is taken
    float dt     = medium.dt;
    float tau_0  = medium.tau_0;
    array gbc   = medium.gbc;

    py = ((1 - 0.5f * dt / tau_0) * py + gbc * ey) / (1 + 0.5f * dt / tau_0);
}


void fdtd(bool console)
{
    double time_total = 30; // run for N seconds

    //------------------------------------------------------------------------
    // Initialize necessary parameters.
    //------------------------------------------------------------------------
    // Problem geometry and grid spacing.
    float Lz   = .4f;            // Length of comp. region in z-dir.
    int Nz     = 160;            // Num. grid pts z-dir. (Need Lz/Nz int.)
    int Nx     = 400;            // Num. grid pts x-dir. (Lx=Nx*(Lz/Nz))
    float ddx  = Lz / Nz;        // Grid spacing.
    float dt   = ddx / 6e8f;     // Time step size, Eqn. (1.7).

    // PML parameters.
    struct pml_t pml;
    pml.npml_x = 8;    // Num. pml grid pts. x-dir.
    pml.npml_z = 20;   // Num. pml grid pts. z-dir.
    // Determine bounderies on which you want pml's.
    pml.pml_vec[0] = 1;
    pml.pml_vec[1] = 1;
    pml.pml_vec[2] = 0;
    pml.pml_vec[3] = 1;
    // Increase Nx and Nz to account for PMLs.
    Nx += (pml.pml_vec[0] + pml.pml_vec[1]) * pml.npml_x;
    Nz += (pml.pml_vec[2] + pml.pml_vec[3]) * pml.npml_z;

    // Medium parameters.
    struct medium_t medium;
    float Ml = .3f; // Length of dispersive medium.
    medium.Mstart     = ceil((Lz - Ml) / ddx) + pml.npml_z; // z interface of medium.
    medium.epsilon_r  = 5;                                // Relative permittivity.
    medium.chi_1      = 30;                               // Debye ODE parameter.
    medium.tau_0      = 1e-11f;                           // Debye ODE parameter.
    medium.sigma      = .01f;                             // Conductivity.
    medium.back_bound = false;                            // true -> sigma = 1e6; false -> E=0.

    // Antenna and pulse parameters.
    struct pulse_t pulse;
    int Az = 10;                             // z-coord of antenna is Az*ddx.
    pulse.Az      = pml.npml_z + (Az - 1);   // Account for pml.
    pulse.Aw      = 8;                       // Antenna width. (divisible by 2)
    pulse.Ac      = Nx / 2;                  // Center of antenna.
    pulse.choice  = P_SINE;                  // Gaussian or sine wave.

    // initialize PML parameters and vectors
    array gi1, gi2, gi3;
    array gj1, gj2, gj3;
    array fi2, fi3, fj2, fj3;
    get_pmlr(gi1, gi2, gi3, gj1, gj2, gj3, fi2, fi3, fj2, fj3, Nz, Nx, pml);

    // initialize medium parameters and vectors
    array ga, gb, gbc;
    medium_params_debye(ga, gb, gbc, Nx, Nz, dt, medium);

    // get electric pulse information
    get_pulse(Nx, dt, pulse);

    // Initialization vectors and parameters for the FDTD algorithm.
    array ZERO = constant(0,Nx,Nz);
    array hx = ZERO, hz = ZERO; // Magnetic field
    array ey = ZERO, sy = ZERO, iy = ZERO; // Electric field and medium
    // temp storage vectors
    array ihx   = ZERO, ihz    = ZERO;
    array dy    = ZERO, dy_hat = ZERO;
    array dyhz  = ZERO, dxhx   = ZERO;
    array cey   = ZERO, cez    = ZERO;

    //------------------------------------------------------------------------
    // MAIN FDTD LOOP.
    //------------------------------------------------------------------------

    printf("Finite-difference time-domain simulation of electromagnetic field\n");

    timer t = timer::start();
    unsigned iter = 0;

    while (progress(iter, t, time_total)) {

        dyhz(seq(1, end), span) = diff1(hz);
        dxhx(span, seq(1, end)) = diff1(hx, 1);

        array dy_hat_temp = gi3 * dy_hat + 0.5f * gi2 * (dyhz - dxhx);
        dy = gj3 * dy + gj2 * (dy_hat_temp - dy_hat);
        dy_hat = dy_hat_temp;

        compute_pulse(dy, (float)iter++, pulse);

        //----------------------------------------------------------------------
        // Determine the Ey field from Dy. Update Iy and Sy.
        //--------------------d--------------------------------------------------
        array ey = ga * (dy - iy - sy);
        iy = iy + gb * ey;
        update_s_debye_TM(sy, ey, medium);

        cez(seq(0, end - 1), span) = diff1(ey);
        cey(span, seq(0, end - 1)) = diff1(-ey, 1);

        ihz += gj1 * cez;
        hz = fi3 * hz + fi2 * (0.5f * cez + ihz);

        ihx += gi1 * cey;
        hx = fj3 * hx + fj2 * (0.5f * cey + ihx);

        ey = abs(ey);

        if (!console) {
            // continually draw the current field
            fig("color","heat");
            fig("sub",2,2,2);   image(ey);
            fig("sub",2,2,4);   plot2(ey);
            fig("sub",2,1,1);   surface(ey);
        } else {
            eval(hx, hz, ey);
        }
    }
}

int main(int argc, char* argv[])
{
    int device = argc > 1 ? atoi(argv[1]) : 0;
    bool console = argc > 2 ? argv[2][0] == '-' : false;

    try {
        af::deviceset(device);
        af::info();

        fdtd(console);
    } catch (af::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        throw;
    }
    return 0;
}
