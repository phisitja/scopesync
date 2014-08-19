/**
 * The BCModular version of Juce's Slider, which adds the ability
 * to be created from an XML definition, as well as being tied into
 * the ScopeSync parameter system
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

#ifndef BCMSLIDER_H_INCLUDED
#define BCMSLIDER_H_INCLUDED

class ScopeSyncGUI;
class SliderProperties;

#include <JuceHeader.h>
#include "../Core/BCMParameter.h"
#include "BCMComponentBounds.h"

class BCMSlider : public Slider
{
public:
    BCMSlider(const String& name);
    ~BCMSlider();

    void applyProperties(SliderProperties& properties, ScopeSyncGUI& gui);

    // Indicates whether a BCMSlider has a parameter mapping
    bool hasParameter() { return mapsToParameter; };
    
    // Returns the parameter a BCMSlider is mapped to
    BCMParameter* getParameter()  { return parameter; };

    // Font and justification variables (made public so they can be accessed by the LookAndFeel)
    float                fontHeight;
    Font::FontStyleFlags fontStyleFlags;
    Justification::Flags justificationFlags;
    
private:
    bool          mapsToParameter; // Flag for whether BCMComboBox maps to a parameter
    BCMParameter* parameter;       // Pointer to a mapped parameter

    StringArray settingsNames; // Names of discrete settings for the parameter mapped to

    BCMComponentBounds componentBounds; // Position/Size information
    
    // Returns a formatted string for a given value to be displayed in the Slider's Textbox
    String getTextFromValue(double v);

    // Interprets input typed into a Slider's Textbox and converts it into a value.
    // Includes support for discrete parameter settings
    double getValueFromText(const String& text);

    // Utility methods for applying User Settings that override aspects of Slider handling
    void overrideSliderStyle(PropertiesFile& appProperties, Slider::SliderStyle style);
    void overridePopupEnabled(PropertiesFile& appProperties, bool popupEnabledFlag);
    void overrideVelocityBasedMode(PropertiesFile& appProperties, bool velocityBasedMode);
    bool getEncoderSnap(PropertiesFile& appProperties, bool encoderSnap);

    // Enumerations for User Settings relating to BCMSliders
    enum RotaryMovement
    {
        _DUMMYROTARYMOVEMENT_,
        noOverride_RM,
        rotary,
        vertical,
        horizontal,
        horizontalVertical
    };

    enum EncoderSnap
    {
        _DUMMYENCODERSNAP_,
        noOverride_ES,
        dontSnap,
        snap
    };

    enum PopupEnabled
    {
        _DUMMYPOPUPENABLED_,
        noOverride_PE,
        popupEnabled,
        popupDisabled
    };

    enum VelocityBasedMode
    {
        _DUMMYVELOCITYBASEDMODE_,
        noOverride_VBM,
        velocityBasedModeOn,
        velocityBasedModeOff
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BCMSlider);
};

#endif  // BCMSLIDER_H_INCLUDED