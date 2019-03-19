#include "SignalGenerator.h"
#include "DynamicsCommon.h"

static float NORMINV (float probability, float mean, float standard_deviation);

USignalGenerator::USignalGenerator()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USignalGenerator::BeginPlay()
{
	Super::BeginPlay();
}

void USignalGenerator::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
    
    float V = 0.f;
    float T = Frequency * Time + Phase;

    switch (SignalType) {
        case SignalType::SIGNAL_SINE:
            V = FMath::Sin( 2.f * PI * T );
            break;
        case SignalType::SIGNAL_SQUARE:
            V = FMath::Sign( FMath::Sin( 2.f * PI * T ) );
            break;
        case SignalType::SIGNAL_TRIANGLE:
            V = 1.f - 4.f * FMath::Abs( FMath::RoundToFloat( T - 0.25f ) - ( T - 0.25f ) );
            break;
        case SignalType::SIGNAL_SAWTOOTH:
            V = 2.f * ( T - FMath::FloorToFloat( T + 0.5f ) );
            break;
        case SignalType::SIGNAL_PULSE:
            V = ( FMath::Abs( FMath::Sin( 2.f * PI * T ) ) < 1.0 - 10E-3 ) ? ( 0 ) : ( 1 );
            break;
        case SignalType::SIGNAL_WHITENOISE:
            V = RandomStream.FRandRange(-1.0f, 1.0f);
            break;
        case SignalType::SIGNAL_GAUSSNOISE:
            V = NORMINV(RandomStream.FRandRange(0.0f, 1.0f), 0.0f, 0.4f);
            break;
        case SignalType::SIGNAL_DIGITALNOISE:
            V = RandomStream.RandRange(0, 1);
            break;
    }
    
    Value = (Invert ? -1 : 1) * Amplitude * V + Offset;
	Time += DeltaTime;
}



//
// Lower tail quantile for standard normal distribution function.
//
// This function returns an approximation of the inverse cumulative
// standard normal distribution function.  I.e., given P, it returns
// an approximation to the X satisfying P = Pr{Z <= X} where Z is a
// random variable from the standard normal distribution.
//
// The algorithm uses a minimax approximation by rational functions
// and the result has a relative error whose absolute value is less
// than 1.15e-9.
//
// Author:      Peter J. Acklam
// Time-stamp:  2003-05-05 05:15:14
// E-mail:      pjacklam@online.no
// WWW URL:     http://home.online.no/~pjacklam

// An algorithm with a relative error less than 1.15*10-9 in the entire region.

static float NORMSINV (float p)
{
    
    // Coefficients in rational approximations
    float a[] = { -39.696830f, 220.946098f, -275.928510f, 138.357751f, -30.664798f, 2.506628f };
    
    float b[] = { -54.476098f, 161.585836f, -155.698979f, 66.801311f, -13.280681f };
    
    float c[] = { -0.007784894002f, -0.32239645f, -2.400758f, -2.549732f, 4.374664f, 2.938163f };
    
    float d[] = { 0.007784695709f, 0.32246712f, 2.445134f, 3.754408f };
    
    // Define break-points.
    float plow = 0.02425f;
    float phigh = 1 - plow;
    
    // Rational approximation for lower region:
    if ( p < plow ) {
        float q = FMath::Sqrt( -2 * FMath::Loge( p ) );
        return ( ( ( ( ( c[ 0 ] * q + c[ 1 ] ) * q + c[ 2 ] ) * q + c[ 3 ] ) * q + c[ 4 ] ) * q + c[ 5 ] ) /
        ( ( ( ( d[ 0 ] * q + d[ 1 ] ) * q + d[ 2 ] ) * q + d[ 3 ] ) * q + 1 );
    }
    
    // Rational approximation for upper region:
    if ( phigh < p ) {
        float q = FMath::Sqrt( -2 * FMath::Loge( 1 - p ) );
        return -( ( ( ( ( c[ 0 ] * q + c[ 1 ] ) * q + c[ 2 ] ) * q + c[ 3 ] ) * q + c[ 4 ] ) * q + c[ 5 ] ) /
        ( ( ( ( d[ 0 ] * q + d[ 1 ] ) * q + d[ 2 ] ) * q + d[ 3 ] ) * q + 1 );
    }
    
    // Rational approximation for central region:
    {
        float q = p - 0.5f;
        float r = q * q;
        return ( ( ( ( ( a[ 0 ] * r + a[ 1 ] ) * r + a[ 2 ] ) * r + a[ 3 ] ) * r + a[ 4 ] ) * r + a[ 5 ] ) * q /
        ( ( ( ( ( b[ 0 ] * r + b[ 1 ] ) * r + b[ 2 ] ) * r + b[ 3 ] ) * r + b[ 4 ] ) * r + 1 );
    }
}

static float NORMINV (float probability, float mean, float standard_deviation)
{
    return ( NORMSINV( probability ) * standard_deviation + mean );
}

static float NORMINV (float probability, const TArray<float>& Samples)
{
    return NORMINV( probability, UDynamicsCommon::MeanOfFloatArray( Samples ), UDynamicsCommon::StandardDeviationOfFloatArray( Samples ) );
}

