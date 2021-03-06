/**
 * The Core of BCModular's ScopeSync system. Handles configuration,
 * parameter management and pulling together the various aspects
 * of communication between different parts of the system.
 *
 * Operates in two modes (known as AppContexts): plugin and scopefx
 * The former is when ScopeSync is hosted within a DAW or plugin
 * host and the latter is when it is hosted within a Scope app,
 * either Scope or Scope SDK.
 * 
 * The ScopeSync object is instantiated differently in each of
 * these two different contexts and there are different DLL
 * builds for each, relying on a __DLL_EFFECT__ pre-processor
 * directive to identify the scopefx context.
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

#ifndef SCOPESYNC_H_INCLUDED
#define SCOPESYNC_H_INCLUDED

class PluginProcessor;
class ScopeFX;
class Configuration;
class ConfigurationManagerWindow;

#ifdef __DLL_EFFECT__
    #include "../Comms/ScopeSyncAsync.h"
#endif // __DLL_EFFECT__

#include <JuceHeader.h>
#include "../Components/BCMLookAndFeel.h"
#include "../Configuration/Configuration.h"

class ScopeSync : public ActionListener,
                  public Value::Listener
{
public:
    /* ========================== Initialisation ============================= */
#ifndef __DLL_EFFECT__
    ScopeSync(PluginProcessor* owner);
#else
    ScopeSync(ScopeFX* owner);
#endif // __DLL_EFFECT__
    ~ScopeSync();
    void unload();

    static const String scopeSyncVersionString;
	static const int numScopeCodes;       // Number of Scope codes
    
    /* ========================== Public Actions ============================= */
    static int  getNumScopeSyncInstances();
	static bool oscUIDInUse(int uid, ScopeSync* currentInstance);
	static void reloadAllGUIs();
    static void shutDownIfLastInstance();
    static const String& getScopeCode(int scopeSyncId);
    static const StringArray& getScopeCodes();;
	static BCMParameter::ParameterType getScopeCodeType(const String& scopeCode);
    static BCMParameter::ParameterType getScopeCodeType(const int scopeCodeId);;
    static int ScopeSync::getScopeCodeId(const String& scopeCode);
    static void snapshotAll();
    void setGUIEnabled(bool shouldBeEnabled) const;
    bool guiNeedsReloading() const;
    void setGUIReload(bool reloadGUIFlag);
    void reloadLayout();
    void unloadConfiguration();
    void addBCMLookAndFeel(BCMLookAndFeel* bcmLookAndFeel);
    BCMLookAndFeel* getBCMLookAndFeelById(String id) const;
    void clearBCMLookAndFeels();
    int  getNumBCMLookAndFeels() const;
    StringArray getBCMLookAndFeelIds(const Identifier& componentType) const;

    ConfigurationManagerWindow* getConfigurationManagerWindow() const;
    void showConfigurationManager(int posX, int posY);
    void hideConfigurationManager();
    ApplicationCommandManager* getCommandManager() const;

    BCMParameterController* getParameterController() const;

#ifdef __DLL_EFFECT__
    ScopeSyncAsync& getScopeSyncAsync();
#else
    PluginProcessor* getPluginProcessor();
#endif // __DLL_EFFECT__
    

    UndoManager& getUndoManager() { return undoManager; }

    void toggleEditToolbar()     { showEditToolbar = !showEditToolbar; }

    bool shouldShowEditToolbar() const;

    /* =================== Public Configuration Methods ====================== */
    void           applyConfiguration();
	bool           isInitialised() const;
    void           saveConfiguration() const;
    bool           saveConfigurationAs();
    void           addConfiguration(juce::Rectangle<int> windowPosition);
    void           reloadSavedConfiguration();
    bool           configurationHasUnsavedChanges() const;
    Configuration& getConfiguration() const;
    ValueTree      getConfigurationRoot() const;
    String         getConfigurationName(bool showUnsavedIndicator) const;
    bool           configurationIsReadOnly() const;
    const File&    getConfigurationFile() const;
    const File&    getLastFailedConfigurationFile() const;

    String         getConfigurationDirectory() const;

    int            getConfigurationUID() const;

    String         getLayoutDirectory() const;
    bool           hasConfigurationUpdate(String& fileName);
    void           changeConfiguration(const String& fileName);
    void           changeConfiguration(int uid);
    bool           processConfigurationChange();

    ValueTree      getMapping() const;
    XmlElement&    getLayout(String& errorText, String& errorDetails, bool forceReload) const;
    static XmlElement*    getSystemLookAndFeels();
    static XmlElement*    getStandardContent(const String& contentToShow);

    Value&         getSystemError();
    Value&         getSystemErrorDetails();
    void           setSystemError(const String& errorText, const String& errorDetailsText);
    bool           newConfigIsInLocation();
    static void    alertBoxLaunchLocationEditor(int result, juce::Rectangle<int> newConfigWindowPosition, ScopeSync* scopeSync);
    void           addConfiguration(File newFile, ValueTree newSettings);
    void           hideAddConfigurationWindow();
    void           actionListenerCallback(const String& message) override;

private:

    /* ========================== Initialisation ============================== */
    void initialise();
    
    void valueChanged(Value& valueThatChanged) override;

    /* ===================== Private member variables ========================= */
#ifndef __DLL_EFFECT__
    PluginProcessor* pluginProcessor;
#else
    ScopeFX*       scopeFX;
    ScopeSyncAsync scopeSyncAsync;
#endif // __DLL_EFFECT__

    Value configurationID;

    OwnedArray<BCMLookAndFeel> bcmLookAndFeels;
    ScopedPointer<ApplicationCommandManager> commandManager;
    ScopedPointer<BCMParameterController>    parameterController;
    static Array<ScopeSync*>   scopeSyncInstances;     // Tracks instances of this object, so Juce can be shutdown when no more remain
	
    ScopedPointer<ConfigurationManagerWindow> configurationManagerWindow;
    UndoManager                undoManager;
    ScopedPointer<NewConfigurationWindow>     addConfigurationWindow;
	
    juce::Rectangle<int> newConfigWindowPosition;
    
    CriticalSection flagLock;
   
    bool reloadGUI;                 // Flag to indicate whether the GUI needs to be reloaded
    bool retainParameterState;      // Flag to indicate whether parameter values should be restored after loading configuration
    bool configurationLoading;
	bool initialised;
    
    bool showEditToolbar; // Indicates whether the EditToolbar should be shown in the GUI's Main Component

	Array<String, CriticalSection> configurationChanges;
    ScopedPointer<Configuration>   configuration;

	// Global flag to disable Performance Mode (used by ScopeFX on project/preset load)
	static int performanceModeGlobalDisable;

	static const StringArray scopeCodes;  // Array of Scope codes for looking up during configuration
	static const BCMParameter::ParameterType scopeCodeTypes[]; // Types of each of the scope codes
    
    Value systemError;        // Latest system error text
    Value systemErrorDetails; // Latest system error details text
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopeSync)
};

#endif  // SCOPESYNC_H_INCLUDED
