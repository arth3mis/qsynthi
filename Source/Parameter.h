/*
  ==============================================================================

    Parameter.h
    Created: 16 Dec 2022 10:50:27pm
    Author:  Arthur

  ==============================================================================
*/

#pragma once

struct Parameter
{
    float attackFactor;
    float decayFactor;
    float releaseFactor;
    float sustainLevel;

    Parameter(float att, float dec, float rel, float sus) 
        : attackFactor{att}, decayFactor{dec}, releaseFactor{rel}, sustainLevel{sus}
    {}
};