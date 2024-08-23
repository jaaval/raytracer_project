#pragma once


constexpr int ABSMAXITER = 5000;
constexpr double DISTLIMIT = 5000;
constexpr double HITEPS = 1e-6;

constexpr int MAXRECURSION = 16;

constexpr double MINT = 0.1;
constexpr double MAXT = 3.00;

// an arbitrary multiplier in soft shadow computation
constexpr double SOFTSHADOWCONSTANT = 350.0;

// factors in lighting computation
constexpr double K_SPEC = 1.0;
constexpr double K_DIFF = 1.0;
constexpr double K_AMBI = 0.1;

// this is the eps for normal estimation
constexpr double NORMALCOMPCONST = 1e-7;


// sdf type ids 
constexpr int AIR = 0;
constexpr int GROUP = 1;
constexpr int OBJECT = 2;
constexpr int LIGHT = 3;


// material defaults
constexpr double REFRACTINDEX = 1.0;
constexpr double REFLECTANCE = 0.01;
constexpr double TRANSPARITY = 0.0;
constexpr double ROUGHNESS = 0.0;
constexpr double DEFR = 0.7;
constexpr double DEFG = 0.5;
constexpr double DEFB = 0.4;

//
constexpr double PI = 3.1415926536;