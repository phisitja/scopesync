/**
 * Models a Host or Scope parameter, including the handling
 * of its ValueTree and various values
 *
 *  (C) Copyright 2014 bcmodular (http://www.bcmodular.co.uk/)
 *
 * This file is part of ScopeSync.
 *
 * ScopeSync is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * ScopeSync is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ScopeSync.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contributors:
 *  Simon Russell
 *  Will Ellis
 *  Jessica Brandt
 */

#include "BCMParameter.h"
#include "../Utils/BCMMath.h"
#include "Global.h"
#include "ScopeSyncApplication.h"
#include "ScopeSync.h"
#include "../Windows/UserSettings.h"

const int BCMParameter::deadTimeTimerInterval = 100;
const int BCMParameter::maxAsyncDeadTime = 3;
const int BCMParameter::maxOSCDeadTime   = 3;

BCMParameter::BCMParameter(int index, ValueTree parameterDefinition, ParameterType parameterType, ScopeSync& ss)
    : type(parameterType),
      hostIdx(index),
      definition(parameterDefinition),
      affectedByUI(false),
	  scopeSync(ss)
{
	oscDeadTimeCounter   = 0;
	asyncDeadTimeCounter = 0;

	startTimer(deadTimeTimerInterval);
	
	putValuesInRange(true);
    setNumDecimalPlaces();
	uiValue.addListener(this);
}

BCMParameter::~BCMParameter()
{
	masterReference.clear();
	stopTimer();
};

void BCMParameter::setNumDecimalPlaces()
{
    float uiInterval = definition.getProperty(Ids::uiRangeInterval);
    
    // figure out the number of DPs needed to display all values at this
    // interval setting.
    numDecimalPlaces = 7;

    if (uiInterval != 0)
    {
        int v = std::abs (roundToInt(uiInterval * 10000000));

        while ((v % 10) == 0)
        {
            --numDecimalPlaces;
            v /= 10;
        }
    }
}

void BCMParameter::setParameterValues(ParameterUpdateSource updateSource, double newLinearNormalisedValue, double newUIValue)
{
	linearNormalisedValue = newLinearNormalisedValue;
	uiValue               = newUIValue;

	if (updateSource == oscUpdate)
		oscDeadTimeCounter = maxOSCDeadTime;

	if (updateSource != asyncUpdate)
		asyncDeadTimeCounter = maxAsyncDeadTime;
}

void BCMParameter::putValuesInRange(bool initialise)
{
    // DBG("BCMParameter::putValuesInRange - " + String(getName()));
    float uiMinValue = definition.getProperty(Ids::uiRangeMin);
    float uiMaxValue = definition.getProperty(Ids::uiRangeMax);
    
    // DBG("BCMParameter::putValuesInRange - uiMinValue: " + String(uiMinValue) + ", uiMaxValue: " + String(uiMaxValue));
    
	double newUIValue = 0.0f;

    if (initialise)
    {
        // DBG("BCMParameter::putValuesInRange - Initialise to: " + String(definition.getProperty(Ids::uiResetValue)));
        newUIValue = definition.getProperty(Ids::uiResetValue);
    }
    else
    {
        if (float(uiValue.getValue()) < uiMinValue)
        {
            // DBG("BCMParameter::putValuesInRange - Bumping up to: " + String(uiMinValue));
            newUIValue = uiMinValue;
        }
        else if (float(uiValue.getValue()) > uiMaxValue)
        {
            // DBG("BCMParameter::putValuesInRange - Dropping down to: " + String(uiMaxValue));
            newUIValue = uiMaxValue;
        }
    }

	double newLinearNormalisedValue = scaleDouble(uiMinValue, uiMaxValue, 0.0f, 1.0f, newUIValue);

	setParameterValues(internalUpdate, newLinearNormalisedValue, newUIValue);
}

void BCMParameter::mapToUIValue(Value& valueToMapTo)
{
    // DBG("BCMParameter::mapToUIValue - current uiValue: " + uiValue.getValue().toString());
    valueToMapTo.referTo(uiValue);
}

void BCMParameter::setAffectedByUI(bool isAffected)
{
    affectedByUI = isAffected;
}

bool BCMParameter::isAffectedByUI()
{
    return affectedByUI;
}

String BCMParameter::getName()
{
    return definition.getProperty(Ids::name).toString();
}

int BCMParameter::getScopeCode()
{
    int scopeCode;

    if (type == hostParameter)
    {
        scopeCode = definition.getProperty(Ids::scopeSync, -1);
    }
    else
    {
        scopeCode = definition.getProperty(Ids::scopeLocal, -1);
        
        if (scopeCode != -1)
        {
            // We found one, so shift the value by the number of
            // ScopeSyncIds to generate a valid ScopeCode
            scopeCode += ScopeSyncApplication::numScopeSyncParameters;
        }
    }

    return scopeCode;
}

String BCMParameter::getScopeCodeText()
{
    String scopeCodeText(String::empty);

    if (type == hostParameter)
    {
        int scopeCode = definition.getProperty(Ids::scopeSync, -1);

        if (scopeCode != -1)
            scopeCodeText = ScopeSync::getScopeSyncCode(scopeCode);
    }
    else
    {
        int scopeCode = definition.getProperty(Ids::scopeLocal, -1);

        if (scopeCode != -1)
            scopeCodeText = ScopeSync::getScopeLocalCode(scopeCode);
    }

    return scopeCodeText;
}

void BCMParameter::getSettings(ValueTree& settings)
{
    settings = definition.getChildWithName(Ids::settings);
}

void BCMParameter::getDescriptions(String& shortDesc, String& fullDesc)
{
    shortDesc = definition.getProperty(Ids::shortDescription).toString();
    fullDesc  = definition.getProperty(Ids::fullDescription).toString();
}

void BCMParameter::getUIRanges(double& rangeMin, double& rangeMax, double& rangeInt, String& uiSuffix)
{
    rangeMin = definition.getProperty(Ids::uiRangeMin);
    rangeMax = definition.getProperty(Ids::uiRangeMax);

    if (rangeMin == rangeMax)
        rangeMax = rangeMin + 1.0f;
    rangeInt = definition.getProperty(Ids::uiRangeInterval);
    uiSuffix = definition.getProperty(Ids::uiSuffix);
}

double BCMParameter::getUIResetValue()
{
    return definition.getProperty(Ids::uiResetValue);
}

double BCMParameter::getUISkewFactor()
{
    return definition.getProperty(Ids::uiSkewFactor);
}

void BCMParameter::getUITextValue(String& textValue)
{
    int parameterValueType = definition.getProperty(Ids::valueType);

    if (parameterValueType == discrete)
    {
        ValueTree paramSettings = definition.getChildWithName(Ids::settings);

        if (paramSettings.isValid())
        {
            int settingIdx = roundDoubleToInt(uiValue.getValue());
            String settingName = paramSettings.getChild(settingIdx).getProperty(Ids::name);
                    
            if (settingName.isNotEmpty())
                textValue = settingName;
        }
    }
    else
    {
        textValue = String(float(uiValue.getValue()), numDecimalPlaces) + definition.getProperty(Ids::uiSuffix);
    }
    //DBG("BCMParameter::getUITextValue - " + definition.getProperty(paramNameId).toString() + ": " + textValue);
}

float BCMParameter::getHostValue()
{
	float hostValue = linearNormalisedValue.getValue();

	if (!definition.getProperty(Ids::skewUIOnly))
		hostValue = skewHostValue(hostValue, true);

    // DBG("BCMParameter::getHostValue - " + definition.getProperty(Ids::name).toString() + ": " + String(hostValue));
    return hostValue;
}

void BCMParameter::getScopeRanges(int& min, int& max)
{
    min = definition.getProperty(Ids::scopeRangeMin);
    max = definition.getProperty(Ids::scopeRangeMax);
}

int BCMParameter::getScopeIntValue()
{
    int minScopeIntValue;
    int maxScopeIntValue;

    getScopeRanges(minScopeIntValue, maxScopeIntValue);

    int parameterValueType = definition.getProperty(Ids::valueType);

    if (parameterValueType == discrete)
    {
        int intValue = 0;
        
        ValueTree paramSettings = definition.getChildWithName(Ids::settings);

        if (paramSettings.isValid())
        {
            int settingIdx = roundDoubleToInt(uiValue.getValue());
        
            if (settingIdx < paramSettings.getNumChildren())
                intValue = paramSettings.getChild(settingIdx).getProperty(Ids::intValue);
        }
        
        return intValue;
    }
    else
    {
        double valueToScale = linearNormalisedValue.getValue();

        double ref        = definition.getProperty(Ids::scopeDBRef);
        
        if (ref != 0.0f)
        {
            double uiMinValue = definition.getProperty(Ids::uiRangeMin);
            double uiMaxValue = definition.getProperty(Ids::uiRangeMax);

            valueToScale = dbSkew(linearNormalisedValue.getValue(), ref, uiMinValue, uiMaxValue, true);
        }

        int scopeValue = roundDoubleToInt(scaleDouble(0.0f, 1.0f, minScopeIntValue, maxScopeIntValue, valueToScale));

        return scopeValue;
    }
}

bool BCMParameter::isDiscrete()
{
    int parameterValueType = definition.getProperty(Ids::valueType);

    if (parameterValueType == discrete)
    {
        ValueTree parameterSettings = definition.getChildWithName(Ids::settings);

        if (parameterSettings.isValid() && parameterSettings.getNumChildren() > 0)
            return true;
    }

    return false;
}

double BCMParameter::convertLinearNormalisedToUIValue(double linearNormalisedValue)
{
    double minUIValue;
    double maxUIValue;
    double uiInterval;
    String uiSuffix;
    
    getUIRanges(minUIValue, maxUIValue, uiInterval, uiSuffix);
    
    return scaleDouble(0.0f, 1.0f, minUIValue, maxUIValue, linearNormalisedValue);
}

double BCMParameter::convertUIToLinearNormalisedValue(double newValue)
{
    double minUIValue;
    double maxUIValue;
    double uiInterval;
    String uiSuffix;
    
    getUIRanges(minUIValue, maxUIValue, uiInterval, uiSuffix);
    
    return scaleDouble(minUIValue, maxUIValue, 0.0f, 1.0f, newValue);
}

void BCMParameter::setHostValue(float newValue)
{
	if (!definition.getProperty(Ids::skewUIOnly))
		newValue = skewHostValue(newValue, false);

	double newUIValue               = convertLinearNormalisedToUIValue(newValue);
    
	setParameterValues(hostUpdate, newValue, newUIValue);
    
	DBG("BCMParameter::setHostValue - " + definition.getProperty(Ids::name).toString() + " linearNormalisedValue: " + linearNormalisedValue.toString() + ", uiValue: " + uiValue.toString());
}

void BCMParameter::setScopeIntValue(int newValue)
{
	if (!affectedByUI && asyncDeadTimeCounter == 0)
    {
		double newUIValue = 0.0f;
		double newLinearNormalisedValue = 0.0f;

        int parameterValueType = definition.getProperty(Ids::valueType);

        if (parameterValueType == discrete)
        {
            int newSetting = findNearestParameterSetting(newValue);
            
			newUIValue = newSetting;
			newLinearNormalisedValue = convertUIToLinearNormalisedValue(newUIValue);
        }
        else
        {
            int minScopeValue;
            int maxScopeValue;

            getScopeRanges(minScopeValue, maxScopeValue);
            newLinearNormalisedValue = scaleDouble(minScopeValue, maxScopeValue, 0.0f, 1.0f, newValue);

            double ref        = definition.getProperty(Ids::scopeDBRef);
        
            if (ref != 0.0f)
            {
                double uiMinValue = definition.getProperty(Ids::uiRangeMin);
                double uiMaxValue = definition.getProperty(Ids::uiRangeMax);
            
                newLinearNormalisedValue = dbSkew(newLinearNormalisedValue, ref, uiMinValue, uiMaxValue, false);
            }
			//else
			//{
			//	if (definition.getProperty(Ids::skewUIOnly))
			//		newLinearNormalisedValue = skewHostValue(newLinearNormalisedValue, false);
			//}

            newUIValue = convertLinearNormalisedToUIValue(newLinearNormalisedValue);
        }
        
		setParameterValues(asyncUpdate, newLinearNormalisedValue, newUIValue);

        DBG("BCMParameter::setScopeIntValue - " + definition.getProperty(Ids::name).toString() + " linearNormalisedValue: " + linearNormalisedValue.toString() + ", uiValue: " + uiValue.toString());
    }
    else
    {
        DBG("ScopeSync::receiveUpdatesFromScopeAsync: Parameter affected by UI since last update, or still in async dead time: " + definition.getProperty(Ids::name).toString());
    }
}

void BCMParameter::setUIValue(float newValue)
{
	double newUIValue = newValue;
    double newLinearNormalisedValue = convertUIToLinearNormalisedValue(newUIValue);

	//if (ScopeSyncApplication::inScopeFXContext() && definition.getProperty(Ids::skewUIOnly))
	//	newLinearNormalisedValue = skewHostValue(newLinearNormalisedValue, true);

	setParameterValues(guiUpdate, newLinearNormalisedValue, newUIValue);
    DBG("BCMParameter::setUIValue - " + definition.getProperty(Ids::name).toString() + " linearNormalisedValue: " + linearNormalisedValue.toString() + ", uiValue: " + uiValue.toString());
}

void BCMParameter::setOSCValue(float newValue)
{
	double newUIValue = newValue;
    double newLinearNormalisedValue = convertUIToLinearNormalisedValue(newUIValue);

	setParameterValues(oscUpdate, newLinearNormalisedValue, newUIValue);
}

void BCMParameter::timerCallback()
{
	decDeadTimes();
}

void BCMParameter::decDeadTimes()
{
	if (oscDeadTimeCounter > 0)
        oscDeadTimeCounter--;

	if (asyncDeadTimeCounter > 0)
        asyncDeadTimeCounter--;
}

float BCMParameter::skewHostValue(float hostValue, bool invert)
{
    double skewedValue = hostValue;
    double skewFactor = definition.getProperty(Ids::uiSkewFactor);

    if ((skewFactor != 1.0f))
    {
        skewValue(skewedValue, skewFactor, 0.0f, 1.0f, invert);
    }
    
    return (float)skewedValue;
}


double BCMParameter::dbSkew(double valueToSkew, double ref, double uiMinValue, double uiMaxValue, bool invert)
{
    if (!invert)
    {
        double minNormalised = ref * pow(10, (uiMinValue/20));

        if (valueToSkew > minNormalised)
            return scaleDouble(uiMinValue, uiMaxValue, 0.0f, 1.0f, (20 * log10(valueToSkew / ref)));
        else
            return minNormalised;
    }
    else
    {
        valueToSkew = scaleDouble(0.0f, 1.0f, uiMinValue, uiMaxValue, valueToSkew);

        if (valueToSkew > uiMinValue && valueToSkew < uiMaxValue)
            return ref * pow(10, (valueToSkew / 20));
        else if (valueToSkew >= 0)
            return 1;
        else
            return 0;
    }
}

int BCMParameter::findNearestParameterSetting(int value)
{
    ValueTree settings = definition.getChildWithName(Ids::settings);

    int nearestItem = 0;
    int smallestGap = INT_MAX;

    for (int i = 0; i < settings.getNumChildren(); i++)
    {
        int settingValue = settings.getChild(i).getProperty(Ids::intValue);
        int gap = abs(value - settingValue);

        if (gap == 0)
        {
            DBG("BCMParameter::findNearestParameterSetting - Found 'exact' match for setting: " + String(settings.getChild(i).getProperty(Ids::name)));

            nearestItem = i;
            break;
        }
        else if (gap < smallestGap)
        {
            smallestGap = gap;
            nearestItem = i;
        }
    }

    return nearestItem;
}

void BCMParameter::valueChanged(Value& valueThatChanged)
{
	if (valueThatChanged.refersToSameSourceAs(uiValue))
	{
		if (oscDeadTimeCounter == 0)
			sendOSCParameterUpdate();
	}
}

void BCMParameter::sendOSCParameterUpdate()
{
	scopeSync.sendOSCParameterUpdate(hostIdx, uiValue.getValue());
}