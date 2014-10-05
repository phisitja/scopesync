/**
 * Base class for Component classes that can map to a BCMParameter
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

#ifndef BCMPARAMETERWIDGET_H_INCLUDED
#define BCMPARAMETERWIDGET_H_INCLUDED

#include <JuceHeader.h>
#include "../Core/BCMParameter.h"

class BCMParameterWidget : public ApplicationCommandTarget
{
public:
    BCMParameterWidget(ApplicationCommandManager* acm);
    ~BCMParameterWidget() {}

    // Indicates whether a BCMSlider has a parameter mapping
    bool hasParameter() { return mapsToParameter; };
    
    // Returns the parameter a BCMSlider is mapped to
    BCMParameter* getParameter() { return parameter.get(); };

protected:
    bool          mapsToParameter;         // Flag for whether the widget maps to a parameter
    WeakReference<BCMParameter> parameter; // Pointer to a mapped parameter

    ValueTree   mapping;

    ApplicationCommandManager* commandManager; // ScopeSync's ApplicationCommandManager

    /* ================= Application Command Target overrides ================= */
    virtual void getAllCommands(Array<CommandID>& commands) override;
    virtual void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override;
    virtual bool perform(const InvocationInfo& info) override;
    virtual ApplicationCommandTarget* getNextCommandTarget();

    virtual void deleteMapping() {};
    virtual void editMapping() {};
    virtual void editMappedParameter() {};
    virtual void showPopup();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BCMParameterWidget);
};



#endif  // BCMPARAMETERWIDGET_H_INCLUDED
